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

        void paint(juce::Graphics &g) override;

        void resized() override;

        zlInterface::Dragger &getDragger() { return dragger; }

        zlInterface::Dragger &getTargetDragger() { return targetDragger; }

        zlInterface::Dragger &getSideDragger() { return sideDragger; }

        bool getSelected() { return dragger.getButton().getToggleState(); }

        void setSelected(bool f);

        void setMaximumDB(float db);

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlInterface::Dragger dragger, targetDragger, sideDragger;
        ButtonPopUp buttonPopUp;
        std::unique_ptr<zlInterface::DraggerParameterAttach> attachment, targetAttach, sideAttach;
        std::atomic<float> maximumDB{zlState::maximumDB::dBs[static_cast<size_t>(zlState::maximumDB::defaultI)]};
        std::atomic<zlIIR::FilterType> fType;
        std::atomic<zlDSP::lrType::lrTypes> lrType;
        std::atomic<size_t> band;
        std::atomic<bool> isFilterTypeHasTarget{false}, isDynamicHasTarget{false},
                isSelectedTarget{false}, isActiveTarget{false};
        juce::CriticalSection targetLock;

        static constexpr std::array IDs{zlDSP::fType::ID, zlDSP::lrType::ID, zlDSP::dynamicON::ID};
        static constexpr std::array NAIDs{zlState::active::ID};
        static constexpr auto scale = 1.5f;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void updateAttachment();

        void updateBounds();

        void updateTargetAttachment();

        const juce::NormalisableRange<float> freqRange{
            10.f, 20000.f,
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return std::exp(valueToRemap * std::log(
                                    rangeEnd / rangeStart)) * rangeStart;
            },
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return std::log(valueToRemap / rangeStart) / std::log(
                           rangeEnd / rangeStart);
            },
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return juce::jlimit(
                    rangeStart, rangeEnd, valueToRemap);
            }
        };
    };
} // zlPanel

#endif //ZLEqualizer_FILTER_BUTTON_PANEL_HPP
