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
        eq_max_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PEQMaxDB::kID)) {
        for (auto& receiver : receivers_) {
            receiver.setON(true);
        }
    }

    MatchFFTPanel::~MatchFFTPanel() = default;

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

    void MatchFFTPanel::run(juce::Thread& thread) {
        runFFT(thread);
    }

    void MatchFFTPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        if (bound.getHeight() < 1.f) {
            return;
        }
        width_.store(bound.getWidth());
        height_.store(bound.getHeight());
        font_size_.store(base_.getFontSize());

        to_update_xs_para_.store(true, std::memory_order::release);
        to_update_ys_para_.store(true, std::memory_order::release);
        to_update_curve_para_.store(true, std::memory_order::release);
    }

    void MatchFFTPanel::runFFT(juce::Thread& thread) {
        auto& sender{p_ref_.getController().getAnalyzerSender()};
        if (!sender.getLock().try_lock()) {
            return;
        }
        const auto sample_rate = sender.getSampleRate();
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
            smoother_.setSmooth(0.1);
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
            to_update_xs_para_.store(true, std::memory_order::relaxed);
            to_update_ys_para_.store(true, std::memory_order::relaxed);
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
            to_update_ys_para_.store(true, std::memory_order::relaxed);
        }
        // update xs para
        if (to_update_xs_para_.exchange(false, std::memory_order::acquire)) {
            const auto fft_max = freq_helper::getFFTMax(sample_rate);
            auto c_width = width_.load(std::memory_order::relaxed) * kFFTSizeOverWidth;
            c_width *= static_cast<float>(std::log((sample_rate * .5 - 0.1) * 0.1) / std::log(fft_max * 0.1));
            const auto temp_scale = static_cast<float>(1.0 / std::log(sample_rate * 0.5 / 10.0)) * c_width;
            const auto temp_bias = std::log(static_cast<float>(10.0)) * temp_scale;
            for (size_t i = 1; i < xs_.size(); ++i) {
                xs_[i] = std::log(freqs_[i]) * temp_scale - temp_bias;
            }
            xs_[0] = std::min(0.f, xs_[2] - 2.f * xs_[1]);
        }
        // update ys para
        if (to_update_ys_para_.exchange(false, std::memory_order::acquire)) {
            const auto height = height_.load(std::memory_order::relaxed);
            const auto font_size = font_size_.load(std::memory_order::relaxed);
            const auto bottom_area_height = getBottomAreaHeight(font_size);
            const auto h0 = font_size * kDraggerScale;
            const auto h1 = height - static_cast<float>(bottom_area_height) - h0;
            y_k_ = (h1 - h0) / c_fft_min_db_;
            y_b_ = h0;
        }
        const auto eq_max_db = zlstate::PEQMaxDB::kDBs[static_cast<size_t>(std::round(
            eq_max_db_idx_ref_.load(std::memory_order::relaxed)))];
        if (std::abs(eq_max_db - c_eq_max_db_) > .1f) {
            c_eq_max_db_ = eq_max_db;
            to_update_curve_para_.store(true, std::memory_order::relaxed);
        }
        if (to_update_curve_para_.exchange(false, std::memory_order::acquire)) {
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
        if (to_update_preset_.exchange(false, std::memory_order::acquire)) {
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
        zldsp::vector::sub(dbs_[0].data(), dbs_[1].data(), dbs_[0].size());
        zldsp::vector::fma(ys_.data(), dbs_[0].data(), c_k_, c_b_, ys_.size());
        createPath(paths_[2].getWriter());
    }

    void MatchFFTPanel::createPath(juce::Path& path) const {
        path.clear();
        path.startNewSubPath(xs_[0], ys_[0]);
        for (size_t i = 1; i < xs_.size(); ++i) {
            path.lineTo(xs_[i], ys_[i]);
        }
    }
}
