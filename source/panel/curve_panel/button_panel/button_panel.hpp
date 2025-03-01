// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLEqualizer_BUTTON_PANEL_HPP
#define ZLEqualizer_BUTTON_PANEL_HPP

#include "filter_button_panel.hpp"
#include "link_button_panel.hpp"
#include "../../../dsp/dsp.hpp"
#include "../../../state/state.hpp"
#include "../../../PluginProcessor.hpp"

namespace zlPanel {
    class ButtonPanel final : public juce::Component,
                              private juce::LassoSource<size_t>,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater,
                              private juce::ChangeListener {
    public:
        explicit ButtonPanel(PluginProcessor &processor,
                             zlInterface::UIBase &base);

        ~ButtonPanel() override;

        void paint(juce::Graphics &g) override;

        zlInterface::Dragger &getDragger(const size_t idx) const {
            return panels[idx]->getDragger();
        }

        zlInterface::Dragger &getSideDragger(const size_t idx) const {
            return panels[idx]->getSideDragger();
        }

        void updateDraggers() const {
            for (const auto &p: panels) {
                p->getDragger().updateButton();
                p->getTargetDragger().updateButton();
                p->getSideDragger().updateButton();
                p->updateDraggers();
            }
            for (const auto &p: linkButtons) {
                p->updateBound();
            }
        }

        void resized() override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        std::array<std::unique_ptr<FilterButtonPanel>, zlState::bandNUM> panels;
        std::array<std::unique_ptr<LinkButtonPanel>, zlState::bandNUM> linkButtons;

        PluginProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parametersRef, &parametersNARef;
        zlInterface::UIBase &uiBase;
        zlDSP::Controller<double> &controllerRef;

        std::array<zlInterface::SnappingSlider, 3> wheelSlider;
        std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 3> wheelAttachment;

        std::atomic<float> maximumDB;
        std::atomic<size_t> selectBandIdx{0};

        static constexpr std::array IDs{zlDSP::freq::ID, zlDSP::gain::ID, zlDSP::Q::ID};

        static constexpr std::array NAIDs{zlState::maximumDB::ID, zlState::selectedBandIdx::ID};

        std::array<std::unique_ptr<zlChore::ParaUpdater>, zlState::bandNUM> freqUpdaters, gainUpdaters, QUpdaters;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        inline static float xtoFreq(const float x, const juce::Rectangle<float> bound) {
            const auto portion = (x - bound.getX()) / bound.getWidth();
            return std::exp(portion *
                            static_cast<float>(std::log(zlFilter::frequencies.back() / zlFilter::frequencies.front())))
                   *
                   static_cast<float>(zlFilter::frequencies.front());
        }

        inline static float yToDB(const float y, const float maxDB, const juce::Rectangle<float> bound) {
            return -(y - bound.getCentreY()) / bound.getHeight() * 2.f * maxDB;
        }

        size_t findAvailableBand() const;

        juce::LassoComponent<size_t> lassoComponent;
        juce::SelectedItemSet<size_t> itemsSet;
        int previousLassoNum{0};
        std::atomic<bool> isLeftClick{true};
        std::array<std::atomic<float>, zlState::bandNUM> previousFreqs{}, previousGains{}, previousQs{};

        std::atomic<bool> toAttachGroup{false};

        void findLassoItemsInArea(juce::Array<size_t> &itemsFound, const juce::Rectangle<int> &area) override;

        juce::SelectedItemSet<size_t> &getLassoSelection() override;

        void changeListenerCallback(juce::ChangeBroadcaster *source) override;

        void attachGroup(size_t idx);

        inline void drawFilterParas(juce::Graphics &g, zlFilter::FilterType fType,
                                    double freq, double gain, const juce::Rectangle<float> &bound);

        inline void drawFreq(juce::Graphics &g, float freq, const juce::Rectangle<float> &bound, bool isTop);

        inline void drawGain(juce::Graphics &g, float gain, const juce::Rectangle<float> &bound, bool isLeft);

        inline void loadPreviousParameters();
    };
} // zlPanel

#endif //ZLEqualizer_BUTTON_PANEL_HPP
