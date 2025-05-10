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
#include "../../../state/state_definitions.hpp"
#include "side_panel.hpp"
#include "reset_attach.hpp"

namespace zlpanel {
    class SinglePanel final : public juce::Component,
                              private juce::AudioProcessorValueTreeState::Listener,
                              private juce::AsyncUpdater {
    public:
        explicit SinglePanel(size_t band_idx,
                             juce::AudioProcessorValueTreeState &parameters,
                             juce::AudioProcessorValueTreeState &parameters_NA,
                             zlgui::UIBase &base,
                             zlp::Controller<double> &controller,
                             zldsp::filter::Ideal<double, 16> &base_filter,
                             zldsp::filter::Ideal<double, 16> &target_filter,
                             zldsp::filter::Ideal<double, 16> &main_filter);

        ~SinglePanel() override;

        void paint(juce::Graphics &g) override;

        void resized() override;

        void setMaximumDB(const float x) {
            maximum_db_.store(x);
            to_repaint_.store(true);
        }

        void updateVisible() {
            if (active_.load() != isVisible()) {
                setVisible(active_.load());
                if (!isVisible()) {
                    avoid_repaint_.store(true);
                }
            }
        }

        bool checkRepaint();

        void setScale(const double x) {
            scale_.store(x);
            base_f_.setGain(static_cast<double>(zlp::gain::range.snapToLegalValue(
                static_cast<float>(current_base_gain_.load() * x))));
            target_f_.setGain(static_cast<double>(zlp::targetGain::range.snapToLegalValue(
                static_cast<float>(current_target_gain_.load() * x))));
        }

        void run(float physicalPixelScaleFactor);

        juce::Point<float> getButtonPos() const { return button_pos_.load(); }

        juce::Point<float> getTargetButtonPos() const {return target_button_pos_.load();}

        void lookAndFeelChanged() override;

    private:
        juce::Path curve_path_, stroke_path_, shadow_path_, dyn_path_;
        juce::Path recent_curve_path_, recent_shadow_path_, recent_dyn_path_;
        juce::SpinLock curve_lock_, shadow_lock_, dyn_lock_;

        size_t band_idx_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zlp::Controller<double> &controller_ref_;
        zlpanel::ResetAttach reset_attach_;
        zldsp::filter::Ideal<double, 16> &base_f_, &target_f_, &main_f_;

        std::atomic<bool> dyn_on_, selected_, active_;
        std::atomic<float> maximum_db_;
        AtomicBound<float> atomic_bound_;
        AtomicPoint<float> atomic_bottom_left_, atomic_bottom_right_;
        std::atomic<float> curve_thickness_{0.f};

        std::atomic<bool> to_repaint_{false};
        std::atomic<bool> avoid_repaint_{true};
        AtomicPoint<float> button_pos_, button_curve_pos_, target_button_pos_;
        std::atomic<double> current_base_gain_{0.0}, current_target_gain_{0.0};
        std::atomic<double> scale_{1.0};

        static constexpr std::array kChangeIDs{
            zlp::bypass::ID, zlp::lrType::ID, zlp::dynamicON::ID
        };

        static constexpr std::array kParaIDs{
            zlp::fType::ID, zlp::slope::ID,
            zlp::freq::ID, zlp::gain::ID, zlp::Q::ID,
            zlp::targetGain::ID, zlp::targetQ::ID
        };

        juce::Colour colour_;

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void handleAsyncUpdate() override;
    };
} // zlpanel
