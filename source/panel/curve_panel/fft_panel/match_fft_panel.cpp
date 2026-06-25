// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_fft_panel.hpp"

namespace zlpanel {
    MatchFFTPanel::MatchFFTPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p),
        base_(base),
        fft_min_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTMinDB::kID)),
        eq_max_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PEQMaxDB::kID)),
        fft_smooth_oct_value_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSmoothOCTValue::kID)),
        fft_smooth_erb_value_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSmoothERBValue::kID)),
        fft_smooth_type_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSmoothType::kID)) {
        for (auto& receiver : receivers_) {
            receiver.setON(true);
        }
        setInterceptsMouseClicks(true, false);
        for (auto& db : drawing_dbs_) {
            db.store(-1000.f, std::memory_order::relaxed);
        }
        std::ranges::fill(c_drawing_dbs_, -1000.f);
    }

    MatchFFTPanel::~MatchFFTPanel() {
        p_ref_.getController().setMatchBypassON(false);
        stopTimer();
    }

    void MatchFFTPanel::paint(juce::Graphics& g) {
        for (size_t i = 0; i < 3; i++) {
            paths_[i].pull();
        }
        const auto thickness = base_.getFontSize() * .2f;
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour).withAlpha(.75f));
        g.strokePath(paths_[0].getReader(),
                     {thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
        g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kSideColour).withAlpha(.75f));
        g.strokePath(paths_[1].getReader(),
                     {thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
        g.setColour(base_.getTextColour());
        g.strokePath(paths_[2].getReader(),
                     {thickness * 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
    }

    void MatchFFTPanel::run(const juce::Thread& thread) {
        if (to_reset_analyzer_.check()) {
            for (auto& accu : accumulators_) {
                accu.reset();
            }
            for (auto& db : drawing_dbs_) {
                db.store(-1000.f, std::memory_order::relaxed);
            }
            to_update_drawing_.signal();
        }
        if (match_phase_.load(std::memory_order::relaxed) == MatchPhase::kAnalyze) {
            runAnalyze(thread);
        } else {
            runMatch(thread);
            match_phase_.store(MatchPhase::kAnalyze, std::memory_order::release);
        }
    }

    void MatchFFTPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        if (bound.getHeight() < 1.f) {
            return;
        }
        width_.store(bound.getWidth());
        height_.store(bound.getHeight());
        font_size_.store(base_.getFontSize());

        to_update_xs_para_.signal();
        to_update_ys_para_.signal();
        to_update_curve_para_.signal();
    }

    void MatchFFTPanel::loadFromPreset(const std::vector<float>& freqs, const std::vector<float>& dbs) {
        {
            std::lock_guard lock{load_freq_mutex_};
            preset_freqs_ = freqs;
        }
        {
            std::lock_guard lock{load_db_mutex_};
            preset_dbs_ = dbs;
        }
        to_update_preset_.signal();
    }

    void MatchFFTPanel::saveToPreset(std::vector<float>& freqs, std::vector<float>& dbs) {
        {
            std::lock_guard lock{save_freq_mutex_};
            freqs.resize(freqs_.size());
            std::ranges::copy(freqs_, freqs.begin());
        }
        {
            std::lock_guard lock{save_db_mutex_};
            dbs.resize(dbs_[1].size());
            std::ranges::copy(dbs_[1], dbs.begin());
        }
    }

    void MatchFFTPanel::setSideMode(const SideMode mode) {
        side_mode_.store(mode, std::memory_order::release);
    }

    void MatchFFTPanel::setDiffDrawOn(const bool is_on) {
        c_diff_draw_off_ = !is_on;
        diff_draw_off_.store(!is_on, std::memory_order::relaxed);
    }

    void MatchFFTPanel::setDiffScale(const float diff_scale) {
        diff_scale_.store(diff_scale, std::memory_order::relaxed);
    }

    void MatchFFTPanel::setDiffShift(const float diff_shift) {
        diff_shift_.store(diff_shift, std::memory_order::relaxed);
    }

    void MatchFFTPanel::setDiffSlope(const float diff_slope) {
        diff_slope_.store(diff_slope, std::memory_order::relaxed);
        to_update_diff_slope_.signal();
    }

    void MatchFFTPanel::setMatchPhase(const MatchPhase match_phase) {
        match_phase_.store(match_phase, std::memory_order::relaxed);
    }

    MatchFFTPanel::MatchPhase MatchFFTPanel::getMatchPhase() const {
        return match_phase_.load(std::memory_order::relaxed);
    }

    void MatchFFTPanel::setMatchLimit(const float match_limit) {
        match_limit_.store(match_limit, std::memory_order::relaxed);
    }

    void MatchFFTPanel::runAnalyze(const juce::Thread& thread) {
        juce::ScopedNoDenormals noDenormals;
        auto& sender{p_ref_.getController().getAnalyzerSender()};
        if (!sender.getLock().try_lock()) {
            return;
        }
        const auto sample_rate = sender.getSampleRate();
        bool update_smooth{false};
        if (std::abs(c_sample_rate_ - sample_rate) > 0.1) {
            c_sample_rate_ = sample_rate;
            int fft_order;
            if (sample_rate <= 50000) {
                fft_order = 12;
            } else if (sample_rate <= 100000) {
                fft_order = 13;
            } else if (sample_rate <= 200000) {
                fft_order = 14;
            } else {
                fft_order = 15;
            }
            fft_size_ = 1 << fft_order;
            processor_.prepare(fft_order);
            for (auto& receiver : receivers_) {
                receiver.prepare(2);
            }
            for (auto& accu : accumulators_) {
                accu.prepare(static_cast<size_t>(fft_size_));
            }
            smoother_.prepare(static_cast<size_t>(fft_size_));
            update_smooth = true;
            tilter_.prepare(static_cast<size_t>(fft_size_));
            tilter_.setTiltSlope(sample_rate, 4.5);

            raw_freqs_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
            raw_dbs_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
            interpolator_ = std::make_unique<zldsp::interpolation::SeqMakima<float>>(
                raw_freqs_.data(), raw_dbs_.data(), raw_freqs_.size(), 0.0, 0.0);
            {
                std::lock_guard lock{save_freq_mutex_};
                freqs_.resize(kNumPoints);
            }
            xs_.resize(kNumPoints);
            ys_.resize(kNumPoints);
            dbs_[0].resize(kNumPoints);
            {
                std::lock_guard lock{save_db_mutex_};
                dbs_[1].resize(kNumPoints);
            }
            // fill raw_freqs
            {
                const double multiplier = c_sample_rate_ * 0.5 / static_cast<double>(raw_freqs_.size() - 1);
                for (size_t i = 0; i < raw_freqs_.size(); ++i) {
                    raw_freqs_[i] = static_cast<float>(static_cast<double>(i) * multiplier);
                }
            }
            const auto fft_max = freq_helper::getFFTMax(sample_rate);
            // fill freqs
            {
                constexpr double start_freq = 10.0;
                const double multiplier = std::pow(fft_max / start_freq, 1.0 / (kNumPoints - 1));
                double current_freq = start_freq;
                std::lock_guard lock{save_freq_mutex_};
                for (size_t i = 0; i < kNumPoints - 1; ++i) {
                    freqs_[i] = static_cast<float>(current_freq);
                    current_freq *= multiplier;
                }
                freqs_.back() = static_cast<float>(fft_max - 0.1);
            }
            to_update_xs_para_.signal();
            to_update_ys_para_.signal();
        }
        // receiver pull data
        auto& fifo{sender.getAbstractFIFO()};
        auto num_read = fifo.getNumReady() / 4 * 3;
        if (num_read > fft_size_) {
            (void)fifo.prepareToRead(num_read - fft_size_);
            fifo.finishRead(num_read - fft_size_);
            num_read = fft_size_;
        }
        const auto range = fifo.prepareToRead(num_read);
        receivers_[0].pull(range, sender.getSampleFIFOs()[0]);
        receivers_[1].pull(range, sender.getSampleFIFOs()[2]);
        fifo.finishRead(num_read);
        sender.getLock().unlock();
        if (thread.threadShouldExit()) {
            return;
        }
        if (fft_size_ <= 0) {
            return;
        }
        // update min db
        const auto min_db = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(
            fft_min_db_idx_ref_.load(std::memory_order::relaxed)))];
        if (std::abs(min_db - c_fft_min_db_) > .1f) {
            c_fft_min_db_ = min_db;
            to_update_ys_para_.signal();
        }
        // update smooth
        const auto fft_smooth_oct_value_idx = static_cast<int>(std::round(
            fft_smooth_oct_value_idx_ref_.load(std::memory_order::relaxed)));
        const auto fft_smooth_erb_value_idx = static_cast<int>(std::round(
            fft_smooth_erb_value_idx_ref_.load(std::memory_order::relaxed)));
        const auto fft_smooth_type_idx = static_cast<int>(std::round(
            fft_smooth_type_idx_ref_.load(std::memory_order::relaxed)));
        if (fft_smooth_oct_value_idx != fft_smooth_oct_value_idx_ ||
            fft_smooth_erb_value_idx != fft_smooth_erb_value_idx_ ||
            fft_smooth_type_idx != fft_smooth_type_idx_) {
            fft_smooth_oct_value_idx_ = fft_smooth_oct_value_idx;
            fft_smooth_erb_value_idx_ = fft_smooth_erb_value_idx;
            fft_smooth_type_idx_ = fft_smooth_type_idx;
            update_smooth = true;
        }
        if (update_smooth) {
            if (fft_smooth_type_idx == 0) {
                smoother_.setSmoothOCT(
                    zlstate::PFFTSmoothOCTValue::kValues[static_cast<size_t>(fft_smooth_oct_value_idx)]);
            } else {
                smoother_.setSmoothERB(
                    sample_rate, zlstate::PFFTSmoothERBValue::kValues[static_cast<size_t>(fft_smooth_erb_value_idx)]);
            }
        }
        // update xs para
        if (to_update_xs_para_.check()) {
            const auto fft_max = freq_helper::getFFTMax(sample_rate);
            auto c_width = width_.load(std::memory_order::relaxed) * kFFTSizeOverWidth;
            c_width *= static_cast<float>(std::log((sample_rate * .5 - 0.1) * 0.1) / std::log(fft_max * 0.1));
            const auto temp_scale = static_cast<float>(1.0 / std::log(sample_rate * 0.5 / 10.0)) * c_width;
            const auto temp_bias = std::log(static_cast<float>(10.0)) * temp_scale;
            xs_[0] = std::log(freqs_[1] * .5f) * temp_scale - temp_bias;
            for (size_t i = 1; i < xs_.size(); ++i) {
                xs_[i] = std::log(freqs_[i]) * temp_scale - temp_bias;
            }
        }
        // update ys para
        if (to_update_ys_para_.check()) {
            const auto height = height_.load(std::memory_order::relaxed);
            const auto font_size = font_size_.load(std::memory_order::relaxed);
            const auto bottom_area_height = getBottomAreaHeight(font_size);
            const auto h0 = font_size * kDraggerScale;
            const auto h1 = height - static_cast<float>(bottom_area_height) - h0;
            y_k_ = (h1 - h0) / c_fft_min_db_;
            y_b_ = h0;
        }
        // update curve para
        const auto eq_max_db = base_.getCurveDBScale(static_cast<size_t>(std::round(
            eq_max_db_idx_ref_.load(std::memory_order::relaxed))));
        if (std::abs(eq_max_db - c_eq_max_db_) > .1f) {
            c_eq_max_db_ = eq_max_db;
            to_update_curve_para_.signal();
        }
        if (to_update_curve_para_.check()) {
            const auto height = height_.load(std::memory_order::relaxed);
            const auto font_size = font_size_.load(std::memory_order::relaxed);
            const auto h = height - static_cast<float>(getBottomAreaHeight(font_size));
            const auto padding = font_size * kDraggerScale;
            const auto h0 = h * .5f;
            const auto h1 = h - padding;
            c_k_ = (h1 - h0) / eq_max_db;
            c_b_ = h0;
        }
        // process main FFT & path
        processMainFFT();
        // process side FFT & path
        switch (side_mode_.load(std::memory_order::relaxed)) {
        case SideMode::kSide: {
            processSideFFT();
            break;
        }
        case SideMode::kPreset: {
            processTargetPreset();
            break;
        }
        case SideMode::kFlat: {
            processTargetFlat();
            break;
        }
        }
        processDiff();
        if (thread.threadShouldExit()) {
            return;
        }
        for (size_t i = 0; i < 3; i++) {
            paths_[i].publish();
        }
    }

    void MatchFFTPanel::runMatch(const juce::Thread& thread) {
        zldsp::vector::aligned_vector<float> diffs;
        diffs.resize(dbs_[0].size());
        zldsp::vector::multiply(diffs.data(), dbs_[0].data(), -1.f, dbs_[0].size());
        const auto match_limit = match_limit_.load(std::memory_order::relaxed);
        zldsp::vector::clamp(diffs.data(), -match_limit, match_limit, diffs.size());
        zlchore::eq_match::EqMatchOptimizer optimizer{
            p_ref_.getAtomicSampleRate(),
            freqs_,
            diffs
        };
        auto& match_result{match_result_.getWriter()};
        match_result.num_band_ = optimizer.fit(
            match_result.filter_paras_, zlp::kBandNum, [&thread]() {
                return thread.threadShouldExit();
            });
        match_result_.publish();
        triggerAsyncUpdate();
    }

    void MatchFFTPanel::processMainFFT() {
        constexpr size_t i = 0;
        receivers_[i].forward(zldsp::analyzer::StereoType::kStereo);
        auto& spectrum{receivers_[i].getAbsSqrFFTBuffer()};
        accumulators_[i].process(spectrum);
        smoother_.smooth(spectrum);
        zldsp::vector::sqr_mag_to_db(raw_dbs_.data(), spectrum.data(), spectrum.size());
        tilter_.tilt(std::span{raw_dbs_.data(), raw_dbs_.size()});
        interpolator_->prepare();
        interpolator_->eval(freqs_.data(), dbs_[i].data(), kNumPoints);
        zldsp::vector::fma(ys_.data(), dbs_[i].data(), y_k_, y_b_, ys_.size());
        createPath(paths_[i].getWriter());
    }

    void MatchFFTPanel::processSideFFT() {
        constexpr size_t i = 1;
        receivers_[i].forward(zldsp::analyzer::StereoType::kStereo);
        auto& spectrum{receivers_[i].getAbsSqrFFTBuffer()};
        accumulators_[i].process(spectrum);
        smoother_.smooth(spectrum);
        zldsp::vector::sqr_mag_to_db(raw_dbs_.data(), spectrum.data(), spectrum.size());
        tilter_.tilt(std::span{raw_dbs_.data(), raw_dbs_.size()});
        interpolator_->prepare();
        {
            std::lock_guard lock{save_db_mutex_};
            interpolator_->eval(freqs_.data(), dbs_[i].data(), kNumPoints);
        }
        zldsp::vector::fma(ys_.data(), dbs_[i].data(), y_k_, y_b_, ys_.size());
        createPath(paths_[i].getWriter());
    }

    void MatchFFTPanel::processTargetPreset() {
        constexpr size_t i = 1;
        if (to_update_preset_.check()) {
            std::vector<float> preset_freqs, preset_dbs;
            {
                std::lock_guard lock{load_freq_mutex_};
                preset_freqs = preset_freqs_;
            }
            {
                std::lock_guard lock{load_db_mutex_};
                preset_dbs = preset_dbs_;
            }
            zldsp::interpolation::SeqMakima<float> interpolator{
                preset_freqs.data(), preset_dbs.data(), preset_freqs.size(), 0.f, 0.f};
            {
                std::lock_guard lock{save_db_mutex_};
                interpolator.eval(freqs_.data(), dbs_[i].data(), kNumPoints);
            }
        }
        zldsp::vector::fma(ys_.data(), dbs_[i].data(), y_k_, y_b_, ys_.size());
        createPath(paths_[i].getWriter());
    }

    void MatchFFTPanel::processTargetFlat() {
        constexpr size_t i = 1;
        {
            std::lock_guard lock{save_db_mutex_};
            std::ranges::fill(dbs_[i], 0.f);
        }
        std::ranges::fill(ys_, y_b_);
        createPath(paths_[i].getWriter());
    }

    void MatchFFTPanel::processDiff() {
        namespace hn = hwy::HWY_NAMESPACE;
        static constexpr hn::ScalableTag<float> d;
        static constexpr size_t lanes = hn::MaxLanes(d);

        if (to_update_drawing_.check()) {
            for (size_t i = 0; i < kNumPoints; ++i) {
                c_drawing_dbs_[i] = drawing_dbs_[i].load(std::memory_order::relaxed);
            }
        }
        if (to_update_diff_slope_.check()) {
            const float center_freq = std::sqrt(freqs_.front() * freqs_.back());
            const float log2_center = std::log2(center_freq);
            const auto v_log2_center = hn::Set(d, log2_center);
            const auto v_slope = hn::Set(d, diff_slope_.load(std::memory_order::relaxed));
            for (size_t i = 0; i < kNumPoints; i += lanes) {
                const auto v_freq = hn::Load(d, freqs_.data() + i);
                const auto v_log2_freq = hn::Log2(d, v_freq);
                const auto v_octaves = hn::Sub(v_log2_center, v_log2_freq);
                const auto v_tilt = hn::Mul(v_octaves, v_slope);
                hn::Store(v_tilt, d, c_diff_tilt_.data() + i);
            }
        }
        auto& dbs{dbs_[0]};
        zldsp::vector::sub(dbs.data(), dbs_[1].data(), dbs.size());
        const auto diff_avg = zldsp::vector::sum(dbs.data(), dbs.size()) / static_cast<float>(dbs.size());
        const auto diff_scale = diff_scale_.load(std::memory_order::relaxed);
        const auto diff_shift = diff_shift_.load(std::memory_order::relaxed);
        zldsp::vector::fma(dbs.data(), diff_scale, diff_shift - diff_avg * diff_scale, dbs.size());
        zldsp::vector::add(dbs.data(), c_diff_tilt_.data(), dbs.size());
        if (!diff_draw_off_.load(std::memory_order::relaxed)) {
            for (size_t i = 0; i < kNumPoints; ++i) {
                if (c_drawing_dbs_[i] > -100.f) {
                    dbs[i] = c_drawing_dbs_[i];
                }
            }
        }
        zldsp::vector::fma(ys_.data(), dbs.data(), c_k_, c_b_, ys_.size());
        createPath(paths_[2].getWriter());
    }

    void MatchFFTPanel::createPath(juce::Path& path) const {
        path.clear();
        path.startNewSubPath(xs_[0], ys_[0]);
        for (size_t i = 1; i < xs_.size(); ++i) {
            path.lineTo(xs_[i], ys_[i]);
        }
    }

    void MatchFFTPanel::mouseDown(const juce::MouseEvent& event) {
        if (c_diff_draw_off_) {
            return;
        }
        // update drawing y scaling (to index)
        {
            const auto sample_rate = p_ref_.getAtomicSampleRate();
            const auto fft_max = freq_helper::getFFTMax(sample_rate);
            auto fft_width = getLocalBounds().toFloat().getWidth();
            fft_width *= kFFTSizeOverWidth;
            fft_width *= static_cast<float>(std::log((sample_rate * .5 - 0.1) * 0.1) / std::log(fft_max * 0.1));
            drawing_p_scale_ = static_cast<float>(kNumPoints - 1) / fft_width;
        }
        // update drawing x scaling (to dB)
        {
            const auto height = static_cast<float>(getHeight());
            const auto font_size = base_.getFontSize();
            const auto h = height - static_cast<float>(getBottomAreaHeight(font_size));
            const auto padding = font_size * kDraggerScale;
            const auto h0 = h * .5f;
            const auto h1 = h - padding;
            const auto eq_max_db = base_.getCurveDBScale(static_cast<size_t>(std::round(
                eq_max_db_idx_ref_.load(std::memory_order::relaxed))));
            drawing_k_ = eq_max_db / (h1 - h0);
            drawing_b_ = -h0;
        }
        // update drawing pre idx/db
        {
            const int raw_pre_idx = static_cast<int>(std::round(event.position.x * drawing_p_scale_));
            drawing_pre_idx_ = static_cast<size_t>(std::clamp(raw_pre_idx, 0, static_cast<int>(kNumPoints - 1)));
            if (event.mods.isRightButtonDown()) {
                drawing_pre_db_ = 0.f;
            } else {
                drawing_pre_db_ = drawing_k_ * (event.position.y + drawing_b_);
            }
        }
    }

    void MatchFFTPanel::mouseDrag(const juce::MouseEvent& event) {
        if (c_diff_draw_off_) {
            return;
        }
        const int raw_c_idx = static_cast<int>(std::round(event.position.x * drawing_p_scale_));
        const auto c_drawing_idx = static_cast<size_t>(std::clamp(raw_c_idx, 0, static_cast<int>(kNumPoints - 1)));
        const auto c_drawing_db = drawing_k_ * (event.position.y + drawing_b_);

        const size_t start_idx = std::min(drawing_pre_idx_, c_drawing_idx);
        const size_t end_idx = std::max(drawing_pre_idx_, c_drawing_idx);

        if (event.mods.isCommandDown()) {
            // reset drawing dbs to none
            for (size_t i = start_idx; i <= end_idx; ++i) {
                drawing_dbs_[i].store(-1000.f, std::memory_order::relaxed);
            }
        } else if (event.mods.isShiftDown()) {
            // set drawing dbs to zero
            for (size_t i = start_idx; i <= end_idx; ++i) {
                drawing_dbs_[i].store(0.f, std::memory_order::relaxed);
            }
        } else {
            // interpolate drawing dbs
            if (start_idx == end_idx) {
                drawing_dbs_[c_drawing_idx].store(c_drawing_db, std::memory_order::relaxed);
            } else {
                const float y_start = (start_idx == drawing_pre_idx_) ? drawing_pre_db_ : c_drawing_db;
                const float y_end = (start_idx == drawing_pre_idx_) ? c_drawing_db : drawing_pre_db_;
                for (size_t i = start_idx; i <= end_idx; ++i) {
                    const float t = static_cast<float>(i - start_idx) / static_cast<float>(end_idx - start_idx);
                    const float interp_db = y_start + t * (y_end - y_start);
                    drawing_dbs_[i].store(interp_db, std::memory_order::relaxed);
                }
            }
        }
        drawing_pre_idx_ = c_drawing_idx;
        drawing_pre_db_ = c_drawing_db;
        to_update_drawing_.signal();
    }

    void MatchFFTPanel::mouseDoubleClick(const juce::MouseEvent&) {
        if (c_diff_draw_off_) {
            return;
        }
        for (auto& db : drawing_dbs_) {
            db.store(-1000.f, std::memory_order::relaxed);
        }
        to_update_drawing_.signal();
    }

    void MatchFFTPanel::handleAsyncUpdate() {
        match_result_.pull();
        const auto match_result = match_result_.getReader();
        base_.setPanelProperty(zlgui::PanelSettingIdx::kMatchPanel, 3.0);
        base_.setPanelProperty(zlgui::PanelSettingIdx::kMaximumNumBand,
                               static_cast<float>(match_result.filter_paras_.size()));
        base_.setPanelProperty(zlgui::PanelSettingIdx::kSuggestedNumBand,
                               static_cast<float>(match_result.num_band_));
    }

    void MatchFFTPanel::updateMatchNumBand(const size_t num_band) {
        p_ref_.getController().setMatchBypassON(true);
        match_result_.pull();
        auto match_result = match_result_.getReader();
        match_result.num_band_ = num_band;
        updateMatchFilters(match_result);
        stopTimer();
        startTimer(1000);
    }

    void MatchFFTPanel::resetAnalyzer() {
        to_reset_analyzer_.signal();
    }

    void MatchFFTPanel::updateMatchFilters(const MatchResult& match_result) {
        const auto num_band = std::min(match_result.num_band_, match_result.filter_paras_.size());
        for (size_t band = num_band; band < zlp::kBandNum; ++band) {
            const auto band_s = std::to_string(band);
            auto* status_para = p_ref_.parameters_.getParameter(zlp::PFilterStatus::kID + band_s);
            updateValue(status_para, 0.f);
        }
        for (size_t band = 0; band < num_band; ++band) {
            const auto band_s = std::to_string(band);
            const auto& paras{match_result.filter_paras_[band]};
            std::array IDs{
                zlp::PFilterType::kID, zlp::POrder::kID,
                zlp::PFreq::kID, zlp::PGain::kID, zlp::PQ::kID,
                zlp::PFilterStatus::kID
            };
            std::array values{
                static_cast<float>(paras.filter_type), static_cast<float>(zlp::POrder::convertToIdx(paras.order)),
                static_cast<float>(paras.freq), static_cast<float>(paras.gain), static_cast<float>(paras.q),
                2.f
            };
            for (size_t i = 0; i < IDs.size(); ++i) {
                auto* para = p_ref_.parameters_.getParameter(IDs[i] + band_s);
                updateValue(para, para->convertTo0to1(values[i]));
            }
        }
    }

    void MatchFFTPanel::timerCallback() {
        p_ref_.getController().setMatchBypassON(false);
        stopTimer();
    }
}
