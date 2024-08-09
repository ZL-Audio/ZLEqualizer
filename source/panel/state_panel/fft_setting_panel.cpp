// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_setting_panel.hpp"

#include "../../state/state.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {
    class FFTCallOutBox final : public juce::Component {
    public:
        explicit FFTCallOutBox(juce::AudioProcessorValueTreeState &parametersNA,
                               zlInterface::UIBase &base)
            : parametersNARef(parametersNA),
              uiBase(base),
              fftPreON("Pre:", zlState::fftPreON::choices, uiBase),
              fftPostON("Post:", zlState::fftPostON::choices, uiBase),
              fftSideON("Side:", zlState::fftSideON::choices, uiBase),
              ffTSpeed("", zlState::ffTSpeed::choices, uiBase),
              fftTilt("", zlState::ffTTilt::choices, uiBase) {
            for (auto &c: {&fftPreON, &fftPostON, &fftSideON}) {
                c->getLabelLAF().setFontScale(1.5f);
                c->setLabelScale(.5f);
                c->setLabelPos(zlInterface::ClickCombobox::left);
                addAndMakeVisible(c);
            }
            for (auto &c: {&ffTSpeed, &fftTilt}) {
                addAndMakeVisible(c);
            }
            attach({
                       &fftPreON.getCompactBox().getBox(),
                       &fftPostON.getCompactBox().getBox(),
                       &fftSideON.getCompactBox().getBox(),
                       &ffTSpeed.getBox(), &fftTilt.getBox()
                   },
                   {
                       zlState::fftPreON::ID, zlState::fftPostON::ID, zlState::fftSideON::ID,
                       zlState::ffTSpeed::ID, zlState::ffTTilt::ID
                   },
                   parametersNARef, boxAttachments);
        }

        ~FFTCallOutBox() override = default;

        void resized() override {
            juce::Grid grid;
            using Track = juce::Grid::TrackInfo;
            using Fr = juce::Grid::Fr;

            grid.templateRows = {Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60)), Track(Fr(60))};
            grid.templateColumns = {Track(Fr(50))};

            grid.items = {
                juce::GridItem(fftPreON).withArea(1, 1),
                juce::GridItem(fftPostON).withArea(2, 1),
                juce::GridItem(fftSideON).withArea(3, 1),
                juce::GridItem(ffTSpeed).withArea(4, 1),
                juce::GridItem(fftTilt).withArea(5, 1)
            };
            grid.setGap(juce::Grid::Px(uiBase.getFontSize() * .4125f));
            auto bound = getLocalBounds().toFloat();
            bound.removeFromTop(uiBase.getFontSize() * .2f);
            grid.performLayout(bound.toNearestInt());
        }

    private:
        juce::AudioProcessorValueTreeState &parametersNARef;
        zlInterface::UIBase &uiBase;

        zlInterface::ClickCombobox fftPreON, fftPostON, fftSideON;
        zlInterface::CompactCombobox ffTSpeed, fftTilt;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> boxAttachments{};
    };

    FFTSettingPanel::FFTSettingPanel(PluginProcessor &p,
                                     zlInterface::UIBase &base)
        : parametersRef(p.parameters),
          parametersNARef(p.parametersNA),
          uiBase(base),
          nameLAF(uiBase),
          callOutBoxLAF(uiBase) {
        juce::ignoreUnused(parametersRef, parametersNARef);
        name.setText("Analyzer", juce::sendNotification);
        nameLAF.setFontScale(1.375f);
        name.setLookAndFeel(&nameLAF);
        name.setEditable(false);
        name.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(name);
    }

    FFTSettingPanel::~FFTSettingPanel() {
        name.setLookAndFeel(nullptr);
        if (boxPointer.getComponent() != nullptr) {
            boxPointer->setLookAndFeel(nullptr);
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

    void FFTSettingPanel::mouseDown(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        openCallOutBox();
    }

    void FFTSettingPanel::resized() {
        name.setBounds(getLocalBounds());
    }

    void FFTSettingPanel::openCallOutBox() {
        if (getTopLevelComponent() == nullptr) {
            return;
        }
        auto content = std::make_unique<FFTCallOutBox>(parametersNARef, uiBase);
        content->setSize(juce::roundToInt(uiBase.getFontSize() * 7.f),
                         juce::roundToInt(uiBase.getFontSize() * 11.2f));

        auto &box = juce::CallOutBox::launchAsynchronously(std::move(content),
                                                           getBounds(),
                                                           getParentComponent()->getParentComponent());
        box.setLookAndFeel(&callOutBoxLAF);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
        boxPointer = &box;
    }
} // zlPanel
