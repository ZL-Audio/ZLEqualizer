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
#include "../../../dsp/analyzer/fft_analyzer/spectrum_accumulator.hpp"
#include "../../../dsp/interpolation/interpolation.hpp"
#include "../../../chore/eq_match/eq_match_optimizer.hpp"
#include "../../../chore/thread/notifier.hpp"

namespace zlpanel {
    class MatchFFTPanel final : public juce::Component,
                                private juce::AsyncUpdater,
                                private juce::Timer {
    public:
        static constexpr size_t kNumPoints = 128;

        enum class SideMode {
            kSide, kPreset, kFlat, kBalance, kNatural,
        };

        enum class MatchPhase {
            kAnalyze, kMatch
        };

        explicit MatchFFTPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~MatchFFTPanel() override;

        void paint(juce::Graphics& g) override;

        void run(const juce::Thread& thread);

        void resized() override;

        void loadFromPreset(const std::vector<float>& freqs, const std::vector<float>& dbs);

        void saveToPreset(std::vector<float>& freqs, std::vector<float>& dbs);

        void setSideMode(SideMode mode);

        void setDiffDrawOn(bool is_on);

        void setDiffScale(float diff_scale);

        void setDiffShift(float diff_shift);

        void setDiffSlope(float diff_slope);

        void setMatchPhase(MatchPhase match_phase);

        MatchPhase getMatchPhase() const;

        void setMatchLimit(float match_limit);

        void updateMatchNumBand(size_t num_band);

        void resetAnalyzer();

    private:
        PluginProcessor& p_ref_;
        zlgui::UIBase& base_;

        std::atomic<SideMode> side_mode_{SideMode::kSide};

        std::atomic<float>& fft_min_db_idx_ref_;
        float c_fft_min_db_{0.f};

        std::atomic<float>& eq_max_db_idx_ref_;
        float c_eq_max_db_{0.f};

        std::atomic<float> &fft_smooth_oct_value_idx_ref_;
        std::atomic<float> &fft_smooth_erb_value_idx_ref_;
        std::atomic<float> &fft_smooth_type_idx_ref_;
        int fft_smooth_oct_value_idx_{-1};
        int fft_smooth_erb_value_idx_{-1};
        int fft_smooth_type_idx_{-1};

        zldsp::vector::aligned_vector<float> raw_freqs_{}, raw_dbs_{};
        zldsp::vector::aligned_vector<float> freqs_{};
        std::array<zldsp::vector::aligned_vector<float>, 2> dbs_{};
        zldsp::vector::aligned_vector<float> xs_{};
        zldsp::vector::aligned_vector<float> ys_{};
        std::array<TriBuffer<juce::Path>, 3> paths_;

        double c_sample_rate_{0.0};
        int fft_size_{0};

        std::atomic<float> width_{0.f}, height_{0.f};
        std::atomic<float> font_size_{0.1f};
        float y_k_{0.f}, y_b_{0.f};
        float c_k_{0.f}, c_b_{0.f};
        zlchore::thread::Notifier to_update_xs_para_{true};
        zlchore::thread::Notifier to_update_ys_para_{true};
        zlchore::thread::Notifier to_update_curve_para_{true};
        zlchore::thread::Notifier to_update_target_curve_{true};

        zldsp::analyzer::FFTAnalyzerProcessor processor_;
        std::array<zldsp::analyzer::FFTAnalyzerReceiver, 2> receivers_{
            zldsp::analyzer::FFTAnalyzerReceiver{processor_},
            zldsp::analyzer::FFTAnalyzerReceiver{processor_}
        };
        std::array<zldsp::analyzer::SpectrumAccumulator, 2> accumulators_;
        zldsp::analyzer::SpectrumSmoother smoother_;
        zldsp::analyzer::SpectrumTilter tilter_;
        std::unique_ptr<zldsp::interpolation::SeqMakima<float>> interpolator_;

        std::mutex save_freq_mutex_;
        std::mutex save_db_mutex_;

        zlchore::thread::Notifier to_update_preset_{};
        std::mutex load_freq_mutex_;
        std::mutex load_db_mutex_;
        std::vector<float> preset_freqs_;
        std::vector<float> preset_dbs_;

        bool c_diff_draw_off_{false};
        std::atomic<bool> diff_draw_off_{false};
        std::array<std::atomic<float>, kNumPoints> drawing_dbs_{};
        alignas(64) std::array<float, kNumPoints> c_drawing_dbs_{};
        zlchore::thread::Notifier to_update_drawing_{};
        float drawing_k_{1.f}, drawing_b_{0.f}, drawing_p_scale_{0.f};
        size_t drawing_pre_idx_{0};
        float drawing_pre_db_{0.f};

        std::atomic<float> diff_scale_{1.f}, diff_shift_{0.f};
        alignas(64) std::array<float, kNumPoints> c_diff_tilt_{};
        std::atomic<float> diff_slope_{0.f};
        zlchore::thread::Notifier to_update_diff_slope_{};

        struct MatchResult {
            std::vector<zldsp::filter::FilterParameters> filter_paras_;
            size_t num_band_{0};
        };

        std::atomic<MatchPhase> match_phase_{MatchPhase::kAnalyze};
        TriBuffer<MatchResult> match_result_;

        std::atomic<float> match_limit_{12.f};

        zlchore::thread::Notifier to_reset_analyzer_{};

        void runAnalyze(const juce::Thread& thread);

        void runMatch(const juce::Thread& thread);

        void processMainFFT();

        void processSideFFT();

        void processTargetPreset();

        void processTargetFlat();

        void processTargetBalance();

        void processTargetNatural();

        void processDiff();

        void createPath(juce::Path& path) const;

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseDrag(const juce::MouseEvent& event) override;

        void mouseDoubleClick(const juce::MouseEvent& event) override;

        void updateMatchFilters(const MatchResult& match_result);

        void handleAsyncUpdate() override;

        void timerCallback() override;
    };
}
