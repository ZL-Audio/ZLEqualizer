// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_setting_panel.hpp"

namespace zlPanel {
    class FFTCallOutBox final : public juce::Component {
    public:
        explicit FFTCallOutBox(juce::AudioProcessorValueTreeState &parametersNA,
                               zlInterface::UIBase &base)
            : parametersNARef(parametersNA),
              uiBase(base.getFontSize(), base.getStyle()),
              ffTStyle("", zlState::ffTStyle::choices, uiBase),
              ffTSpeed("", zlState::ffTSpeed::choices, uiBase),
              fftTilt("", zlState::ffTTilt::choices, uiBase) {
            for (auto &c: {&ffTStyle, &ffTSpeed, &fftTilt}) {
                addAndMakeVisible(c);
            }
            attach({&ffTStyle.getBox(), &ffTSpeed.getBox(), &fftTilt.getBox()},
                   {zlState::ffTStyle::ID, zlState::ffTSpeed::ID, zlState::ffTTilt::ID},
                   parametersNARef, boxAttachments);
        }

        ~FFTCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(60))};

            grid.items = {
                juce::GridItem(ffTStyle).withArea(1, 1),
                juce::GridItem(ffTSpeed).withArea(2, 1),
                juce::GridItem(fftTilt).withArea(3, 1)
            };

            const auto bound = getLocalBounds().toFloat();
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase uiBase;

        zlInterface::CompactCombobox ffTStyle, ffTSpeed, fftTilt;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments;
    };

    FFTSettingPanel::FFTSettingPanel(juce::AudioProcessorValueTreeState &parameters,
                                     juce::AudioProcessorValueTreeState &parametersNA,
                                     zlInterface::UIBase &base)
        : parametersRef(parameters),
          parametersNARef(parametersNA),
          uiBase(base),
          drawable(juce::Drawable::createFromImageData(BinaryData::fadwaveform_svg, BinaryData::fadwaveform_svgSize)),
          button(drawable.get(), base),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        button.getButton().onClick = [this]() { openCallOutBox(); };
        addAndMakeVisible(button);
        setBufferedToImage(true);
    }

    FFTSettingPanel::~FFTSettingPanel() {
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->dismiss();
        }
    }

    void FFTSettingPanel::paint(juce::Graphics &g) {
        g.setColour(uiBase.getBackgroundColor().withMultipliedAlpha(.25f));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), uiBase.getFontSize() * .5f);
        g.setColour(uiBase.getTextColor().withMultipliedAlpha(.25f));
        juce::Path path;
        const auto bound = getLocalBounds().toFloat();
        path.addRoundedRectangle(bound.getX(), bound.getY(), bound.getWidth(), bound.getHeight(),
                                 uiBase.getFontSize() * .5f, uiBase.getFontSize() * .5f,
                                 false, false, true, true);
        g.fillPath(path);
    }


    void FFTSettingPanel::resized() {
        button.getLookAndFeel().setPadding(uiBase.getFontSize() * 1.f);
        button.setBounds(getLocalBounds());
    }

    void FFTSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<FFTCallOutBox>(parametersNARef, uiBase);
        content->setSize(static_cast<int>(uiBase.getFontSize() * 7),
                         static_cast<int>(uiBase.getFontSize() * 5.5));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getScreenBounds(),
                                                           nullptr);

        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();

        boxPointer = &box;
    }
} // zlPanel
