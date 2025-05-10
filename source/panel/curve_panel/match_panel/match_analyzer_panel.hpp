// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../helpers.hpp"
#include "match_label.hpp"

namespace zlpanel {
    class MatchAnalyzerPanel final : public juce::Component,
                                     private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::ValueTree::Listener,
                                     private zlgui::Dragger::Listener {
    public:
        explicit MatchAnalyzerPanel(zldsp::eq_match::EqMatchAnalyzer<double> &analyzer,
                                    juce::AudioProcessorValueTreeState &parameters_NA,
                                    zlgui::UIBase &base);

        ~MatchAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void visibilityChanged() override;

        void updatePaths();

        void updateDraggers();

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

    private:
        zldsp::eq_match::EqMatchAnalyzer<double> &analyzer_ref_;
        juce::AudioProcessorValueTreeState &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        juce::Path path1_{}, path2_{}, path3_{};
        juce::Path recent_path1_{}, recent_path2_{}, recent_path3_{};
        juce::SpinLock path_lock_;
        AtomicPoint<float> left_corner_, right_corner_;
        AtomicBound<float> atomic_bound_;
        float background_alpha_ = .5f;
        bool show_average_{true};
        std::atomic<float> db_scale_{1.f};
        float c_maximum_db_{zlstate::maximumDB::dBs[static_cast<size_t>(zlstate::maximumDB::defaultI)]};
        std::atomic<float> maximum_db_{zlstate::maximumDB::dBs[static_cast<size_t>(zlstate::maximumDB::defaultI)]};
        zlgui::Dragger low_dragger_, high_dragger_, shift_dragger_;
        MatchLabel match_label_;
        static constexpr auto kScale = 1.5f;
        size_t pre_draw_idx_{0};
        float pre_draw_db_{0.f};

        void valueTreePropertyChanged(juce::ValueTree &tree_whose_property_has_changed,
                                      const juce::Identifier &property) override;

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void lookAndFeelChanged() override;

        void dragStarted(zlgui::Dragger *dragger) override {
            juce::ignoreUnused(dragger);
        }

        void dragEnded(zlgui::Dragger *dragger) override {
            juce::ignoreUnused(dragger);
        }

        void draggerValueChanged(zlgui::Dragger *dragger) override;

        void getIdxDBromPoint(const juce::Point<int> &p, size_t &idx, float &dB) const {
            const auto bound = getLocalBounds().toFloat();
            const auto idx_int = juce::roundToInt(
                250.f * (static_cast<float>(p.getX()) - bound.getX()) / bound.getWidth());
            idx = static_cast<size_t>(std::clamp(idx_int, 0, 250));
            const auto y_p = (static_cast<float>(p.getY()) - bound.getY()) / bound.getHeight() - .5f;
            dB = -maximum_db_.load() * db_scale_.load() * y_p;
        }
    };
} // zlpanel
