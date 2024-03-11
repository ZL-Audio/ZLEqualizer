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
#include "../../../state/state.hpp"

namespace zlPanel {
    class ButtonPanel final : public juce::Component,
                              private juce::LassoSource<zlInterface::Dragger*>,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
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
        juce::CriticalSection wheelLock;

        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        std::atomic<float> maximumDB;
        std::atomic<size_t> selectBandIdx{0};

        static constexpr std::array IDs{zlState::maximumDB::ID, zlState::selectedBandIdx::ID};

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

        juce::LassoComponent<zlInterface::Dragger*> lassoComponent;
        juce::SelectedItemSet<zlInterface::Dragger*> itemsSet;

        void findLassoItemsInArea(juce::Array<zlInterface::Dragger*> &itemsFound, const juce::Rectangle<int> &area) override {
            juce::ignoreUnused(itemsFound, area);
            for (const auto &p:panels) {
                if (area.contains(p->getDragger().getButton().getBounds())) {
                    itemsFound.add(&p->getDragger());
                }
            }
        }

        juce::SelectedItemSet<zlInterface::Dragger*> &getLassoSelection() override {
            return itemsSet;
        }


        // juce::FileLogger logger{juce::File("/Volumes/Ramdisk/log.txt"), "button"};
    };
} // zlPanel

#endif //ZLEqualizer_BUTTON_PANEL_HPP
