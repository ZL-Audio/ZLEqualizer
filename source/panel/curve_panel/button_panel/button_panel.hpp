// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "filter_button_panel.hpp"
#include "link_button_panel.hpp"
#include "../../../dsp/dsp.hpp"
#include "../../../state/state.hpp"
#include "../../../PluginProcessor.hpp"

namespace zlpanel {
    class ButtonPanel final : public juce::Component,
                              private juce::LassoSource<size_t>,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::ChangeListener,
                              private juce::Timer {
    public:
        explicit ButtonPanel(PluginProcessor &processor,
                             zlgui::UIBase &base);

        ~ButtonPanel() override;

        void paint(juce::Graphics &g) override;

        zlgui::Dragger &getDragger(const size_t idx) const {
            return panels[idx]->getDragger();
        }

        zlgui::Dragger &getSideDragger(const size_t idx) const {
            return panels[idx]->getSideDragger();
        }

        void updateAttach();

        bool updateDragger(const size_t idx, const juce::Point<float> pos) {
            const auto &p = panels[idx];
            const auto f = p->isVisible()
                               ? p->getDragger().updateButton(pos)
                               : false;
            p->updateDraggers();
            return f;
        }

        void updateOtherDraggers(const size_t idx, const juce::Point<float> targetPos) {
            const auto &p = panels[idx];
            p->getPopUp().updateBounds(p->getDragger().getButton());
            p->getTargetDragger().updateButton(targetPos);
            p->getSideDragger().updateButton();
        }

        void updateLinkButton(const size_t idx) {
            linkButtons[idx]->updateBound();
        }

        void updatePopup(const size_t idx, const bool isDraggerMoved = false) {
            const auto &p = panels[idx];
            if (isDraggerMoved || p->getPopUp().getBounds().getX() < 0) {
                p->getPopUp().updateBounds(p->getDragger().getButton());
            }
        }

        void resized() override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        std::array<std::unique_ptr<FilterButtonPanel>, zlstate::bandNUM> panels;
        std::array<std::unique_ptr<LinkButtonPanel>, zlstate::bandNUM> linkButtons;

        PluginProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zlp::Controller<double> &controller_ref_;

        std::array<zlgui::SnappingSlider, 3> wheelSlider;
        std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 3> wheelAttachment;

        std::atomic<float> maximumDB;
        std::atomic<size_t> selectBandIdx{0};

        static constexpr std::array IDs{
            zlp::freq::ID, zlp::gain::ID, zlp::Q::ID, zlp::targetGain::ID, zlp::targetQ::ID
        };

        static constexpr std::array NAIDs{zlstate::maximumDB::ID, zlstate::selectedBandIdx::ID};

        std::array<std::unique_ptr<zldsp::chore::ParaUpdater>, zlstate::bandNUM> freqUpdaters, gainUpdaters, QUpdaters;
        std::array<std::unique_ptr<zldsp::chore::ParaUpdater>, zlstate::bandNUM> targetGainUpdaters, targetQUpdaters;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        inline static float xtoFreq(const float x, const juce::Rectangle<float> bound) {
            const auto portion = (x - bound.getX()) / bound.getWidth();
            return std::exp(portion *
                            static_cast<float>(std::log(zldsp::filter::kFrequencies.back() / zldsp::filter::kFrequencies.front())))
                   * static_cast<float>(zldsp::filter::kFrequencies.front());
        }

        inline static float yToDB(const float y, const float maxDB, const juce::Rectangle<float> bound) {
            return -(y - bound.getCentreY()) / bound.getHeight() * 2.f * maxDB;
        }

        size_t findAvailableBand() const;

        juce::LassoComponent<size_t> lassoComponent;
        std::atomic<bool> isDuringLasso{false};
        juce::SelectedItemSet<size_t> itemsSet;
        int previousLassoNum{0};
        std::atomic<bool> isLeftClick{true};
        std::array<std::atomic<float>, zlstate::bandNUM> previousFreqs{}, previousGains{}, previousQs{};
        std::array<std::atomic<float>, zlstate::bandNUM> previousTargetGains{}, previousTargetQs{};

        std::atomic<bool> toAttachGroup{false};

        void findLassoItemsInArea(juce::Array<size_t> &itemsFound, const juce::Rectangle<int> &area) override;

        juce::SelectedItemSet<size_t> &getLassoSelection() override;

        void changeListenerCallback(juce::ChangeBroadcaster *source) override;

        void timerCallback() override;

        void attachGroup(size_t idx);

        inline void drawFilterParas(juce::Graphics &g, zldsp::filter::FilterType fType,
                                    float freqP, float gainP, const juce::Rectangle<float> &bound);

        inline void drawFreq(juce::Graphics &g, float freqP, const juce::Rectangle<float> &bound, bool isTop);

        inline void drawGain(juce::Graphics &g, float gainP, const juce::Rectangle<float> &bound, bool isLeft);

        inline void loadPreviousParameters();
    };
} // zlpanel
