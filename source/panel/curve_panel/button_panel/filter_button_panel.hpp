// Copyright (C) 2023 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_FILTER_BUTTON_PANEL_HPP
#define ZLEqualizer_FILTER_BUTTON_PANEL_HPP

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"

#include "button_pop_up.hpp"

namespace zlPanel {
    class FilterButtonPanel final : public juce::Component,
                                    private juce::AudioProcessorValueTreeState::Listener,
                                    private juce::AsyncUpdater {
    public:
        explicit FilterButtonPanel(size_t bandIdx,
                                   juce::AudioProcessorValueTreeState &parameters,
                                   juce::AudioProcessorValueTreeState &parametersNA,
                                   zlInterface::UIBase &base);

        ~FilterButtonPanel() override;

        void resized() override;

        zlInterface::Dragger &getDragger() { return dragger; }

        bool getSelected() { return dragger.getButton().getToggleState(); }

        void setSelected(const bool f) {
            dragger.getButton().setToggleState(f, juce::NotificationType::dontSendNotification);
            if (!f) {
                removeChildComponent(&buttonPopUp);
            } else {
                // dragger.addAndMakeVisible(buttonPopUp);
            }
        }

        void setMaximumDB(float db);

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;



    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlInterface::Dragger dragger;
        ButtonPopUp buttonPopUp;
        std::unique_ptr<zlInterface::DraggerParameterAttach> attachment;
        std::atomic<float> maximumDB;
        std::atomic<zlIIR::FilterType> fType;
        std::atomic<size_t> band;

        static constexpr std::array IDs{zlDSP::fType::ID};
        static constexpr std::array NAIDs{zlState::active::ID};
        static constexpr auto scale = 1.5f;


        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void updateAttachment();

        void updateBounds();
    };
} // zlPanel

#endif //ZLEqualizer_FILTER_BUTTON_PANEL_HPP
