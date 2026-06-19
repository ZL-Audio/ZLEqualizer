// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"
#include "../../../dsp/analyzer/fft_analyzer/fft_analyzer_receiver.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_smoother.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_tilter.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_decayer.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_collision.hpp"
#include "../../../chore/thread/notifier.hpp"
#include "../../../dsp/interpolation/interpolation.hpp"

namespace zlpanel {
    class FFTPanel final : public juce::Component,
                           private juce::ValueTree::Listener {
    public:
        explicit FFTPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~FFTPanel() override;

        void paint(juce::Graphics& g) override;

        void run(const juce::Thread& thread);

        void resized() override;

        void setRefreshRate(double refresh_rate);

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;
        std::atomic<float>& pre_ref_;
        std::atomic<float>& post_ref_;
        std::atomic<float>& side_ref_;
        std::atomic<float>& stereo_ref_;
        std::atomic<float>& coll_ref_;
        std::atomic<float>& coll_strength_ref_;

        std::atomic<float> &fft_min_db_idx_ref_;
        float c_fft_min_db_{0.f};

        std::atomic<float> &fft_speed_idx_ref_;
        int fft_speed_idx_{zlstate::PFFTSpeed::kDefaultI};

        std::atomic<float> &fft_tilt_idx_ref_;
        int fft_tilt_idx_{zlstate::PFFTTilt::kDefaultI};

        std::atomic<float> &fft_smooth_oct_value_idx_ref_;
        std::atomic<float> &fft_smooth_erb_value_idx_ref_;
        std::atomic<float> &fft_smooth_type_idx_ref_;
        int fft_smooth_oct_value_idx_{-1};
        int fft_smooth_erb_value_idx_{-1};
        int fft_smooth_type_idx_{-1};

        bool skip_next_repaint_{false};

        static constexpr size_t kInterSize = 64;
        std::vector<float> xs_{}, ys_{};
        std::vector<float> inter_xs_{}, inter_ys_{};
        std::unique_ptr<zldsp::interpolation::SeqMakima<float>> inter_;
        std::array<TriBuffer<juce::Path>, 3> paths_;

        double c_sample_rate_{0.0};
        int fft_size_{0};
        size_t num_point_{0};

        std::atomic<float> width_{0.f}, height_{0.f};
        std::atomic<float> font_size_{0.1f};
        float c_width_{0.f};
        float c_height_{0.f};
        float y_k_{0.f}, y_b_{0.f};
        zlchore::thread::Notifier to_update_xs_para_{true};
        zlchore::thread::Notifier to_update_ys_para_{true};

        std::atomic<float> refresh_rate_{30.0};
        std::atomic<float> spectrum_extra_decay_speed_{1.f};
        zlchore::thread::Notifier to_update_decay_{true};

        std::atomic<float> spectrum_extra_tilt_slope_{0.f};
        zlchore::thread::Notifier to_update_tilt_{true};

        std::atomic<bool> is_fft_frozen_{false};

        zldsp::analyzer::FFTAnalyzerProcessor processor_;
        std::array<zldsp::analyzer::FFTAnalyzerReceiver, 3> receivers_{
            zldsp::analyzer::FFTAnalyzerReceiver{processor_},
            zldsp::analyzer::FFTAnalyzerReceiver{processor_},
            zldsp::analyzer::FFTAnalyzerReceiver{processor_}
        };
        zldsp::analyzer::SpectrumSmoother smoother_;
        zldsp::analyzer::SpectrumTilter tilter_;
        std::array<zldsp::analyzer::SpectrumDecayer, 3> decayers_;

        zldsp::vector::aligned_vector<float> current_ps_{}, coll_ps_{};
        TriBuffer<juce::ColourGradient> gradient_;

        void runFFT(const juce::Thread& thread);

        void lookAndFeelChanged() override;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) override;
    };
}
