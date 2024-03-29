// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_BUTTON_PANEL_HPP
#define ZLEqualizer_BUTTON_PANEL_HPP

#include "filter_button_panel.hpp"
#include "../../../dsp/dsp.hpp"
#include "../../../state/state.hpp"

namespace zlPanel {
    class ButtonPanel final : public juce::Component,
                              private juce::LassoSource<size_t>,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater,
                              private juce::ChangeListener {
    public:
        explicit ButtonPanel(juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parametersNA,
                             zlInterface::UIBase &base);

        ~ButtonPanel() override;

        void resized() override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        std::array<std::unique_ptr<FilterButtonPanel>, zlState::bandNUM> panels;

        juce::Slider wheelSlider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wheelAttachment;

        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<float> maximumDB;
        std::atomic<size_t> selectBandIdx{0};

        static constexpr std::array IDs{zlDSP::freq::ID, zlDSP::gain::ID, zlDSP::Q::ID};

        static constexpr std::array NAIDs{zlState::maximumDB::ID, zlState::selectedBandIdx::ID};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        inline static float xtoFreq(const float x, const juce::Rectangle<float> bound) {
            const auto portion = (x - bound.getX()) / bound.getWidth();
            return std::exp(portion *
                            static_cast<float>(std::log(zlIIR::frequencies.back() / zlIIR::frequencies.front()))) *
                   static_cast<float>(zlIIR::frequencies.front());
        }

        inline static float yToDB(const float y, const float maxDB, const juce::Rectangle<float> bound) {
            return -(y - bound.getCentreY()) / bound.getHeight() * 2.f * maxDB;
        }

        size_t findAvailableBand() const;

        juce::LassoComponent<size_t> lassoComponent;
        juce::SelectedItemSet<size_t> itemsSet;
        std::atomic<double> currentFreq, currentGain, currentQ;
        std::array<std::atomic<bool>, zlState::bandNUM> isSelected{};
        std::atomic<bool> toAttachGroup{false};

        void findLassoItemsInArea(juce::Array<size_t> &itemsFound, const juce::Rectangle<int> &area) override;

        juce::SelectedItemSet<size_t> &getLassoSelection() override;

        void changeListenerCallback(juce::ChangeBroadcaster *source) override;

        void attachGroup(size_t idx);
    };
} // zlPanel

#endif //ZLEqualizer_BUTTON_PANEL_HPP
