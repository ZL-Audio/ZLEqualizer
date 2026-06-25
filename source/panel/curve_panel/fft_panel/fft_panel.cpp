// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"

namespace zlpanel {
    FFTPanel::FFTPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p),
        base_(base),
        pre_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTPreON::kID)),
        post_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTPostON::kID)),
        side_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSideON::kID)),
        stereo_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTStereo::kID)),
        coll_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PCollisionON::kID)),
        coll_strength_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PCollisionStrength::kID)),
        fft_min_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTMinDB::kID)),
        fft_speed_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSpeed::kID)),
        fft_tilt_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTTilt::kID)),
        fft_smooth_oct_value_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSmoothOCTValue::kID)),
        fft_smooth_erb_value_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSmoothERBValue::kID)),
        fft_smooth_type_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTSmoothType::kID)) {
        constexpr auto preallocate_space = 100 * 3 + 1;
        for (auto& buffered_path : paths_) {
            for (auto& path : buffered_path.getBuffer()) {
                path.preallocateSpace(preallocate_space);
            }
        }
        for (auto& receiver : receivers_) {
            receiver.setON(true);
        }
        setInterceptsMouseClicks(false, false);
        base_.getPanelValueTree().addListener(this);
    }

    FFTPanel::~FFTPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    void FFTPanel::paint(juce::Graphics& g) {
        if (skip_next_repaint_) {
            skip_next_repaint_ = false;
            return;
        }
        const auto pre_on = pre_ref_.load(std::memory_order::relaxed) > .5f;
        const auto post_on = post_ref_.load(std::memory_order::relaxed) > .5f;
        const auto side_on = side_ref_.load(std::memory_order::relaxed) > .5f;
        const auto coll_on = coll_ref_.load(std::memory_order::relaxed) > .5f;
        const std::array<bool, 3> is_on{pre_on, post_on, side_on};
        for (size_t i = 0; i < 3; ++i) {
            if (is_on[i]) {
                paths_[i].pull();
            }
        }

        if (is_on[0]) {
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPreColour));
            g.fillPath(paths_[0].getReader());
        }
        if (is_on[1]) {
            const auto thickness = base_.getFontSize() * .2f;
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour).withAlpha(1.f));
            g.strokePath(paths_[1].getReader(),
                         {thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded});
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kPostColour));
            g.fillPath(paths_[1].getReader());
        }
        if (is_on[2]) {
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kSideColour));
            g.fillPath(paths_[2].getReader());
        }
        if (coll_on && ((pre_on && post_on) || (side_on && post_on))) {
            gradient_.pull();
            const auto& gradient = gradient_.getReader();
            if (gradient.getNumColours() >= 2) {
                g.setGradientFill(gradient);
                g.fillRect(getLocalBounds());
            }
        }
    }

    void FFTPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        if (bound.getHeight() < 1.f) {
            return;
        }
        width_.store(bound.getWidth());
        height_.store(bound.getHeight());
        font_size_.store(base_.getFontSize());

        skip_next_repaint_ = true;
        to_update_xs_para_.signal();
        to_update_ys_para_.signal();
    }

    void FFTPanel::run(const juce::Thread& thread) {
        runFFT(thread);
    }

    void FFTPanel::runFFT(const juce::Thread& thread) {
        juce::ScopedNoDenormals noDenormals;
        const auto pre_on = pre_ref_.load(std::memory_order::relaxed) > .5f;
        const auto post_on = post_ref_.load(std::memory_order::relaxed) > .5f;
        const auto side_on = side_ref_.load(std::memory_order::relaxed) > .5f;
        const auto coll_on = coll_ref_.load(std::memory_order::relaxed) > .5f;
        const std::array<bool, 3> is_on{pre_on, post_on, side_on};
        auto& sender{p_ref_.getController().getAnalyzerSender()};
        if (!sender.getLock().try_lock()) {
            return;
        }
        // update sample rate
        const auto sample_rate = sender.getSampleRate();
        bool update_smooth{false};
        if (std::abs(c_sample_rate_ - sample_rate) > 0.1) {
            c_sample_rate_ = sample_rate;
            to_update_tilt_.signal();
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
            smoother_.prepare(static_cast<size_t>(fft_size_));
            update_smooth = true;
            tilter_.prepare(static_cast<size_t>(fft_size_));
            for (auto& decayer : decayers_) {
                decayer.prepare(static_cast<size_t>(fft_size_));
            }

            xs_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
            ys_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
            inter_xs_.resize(kInterSize + 2);
            inter_ys_.resize(kInterSize + 2);
            inter_ = std::make_unique<zldsp::interpolation::SeqMakima<float>>(
                xs_.data(), ys_.data(), (kInterSize / 2) + 2, 0.f, 0.f);

            current_ps_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
            coll_ps_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
            std::ranges::fill(current_ps_, 0.f);
            std::ranges::fill(coll_ps_, 0.f);

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
        for (size_t i = 0; i < 3; i++) {
            if (is_on[i]) {
                receivers_[i].pull(range, sender.getSampleFIFOs()[i]);
            }
        }
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
            to_update_decay_.signal();
            to_update_ys_para_.signal();
        }
        // update tilt
        const auto fft_tilt_idx = static_cast<int>(std::round(
            fft_tilt_idx_ref_.load(std::memory_order::relaxed)));
        if (fft_tilt_idx != fft_speed_idx_) {
            fft_tilt_idx_ = fft_tilt_idx;
            to_update_tilt_.signal();
        }
        if (to_update_tilt_.check()) {
            tilter_.setTiltSlope(sample_rate,
                                 zlstate::PFFTTilt::kSlopes[static_cast<size_t>(fft_tilt_idx)] +
                                 spectrum_extra_tilt_slope_.load(std::memory_order::relaxed));
        }
        // update speed
        const auto fft_speed_idx = static_cast<int>(std::round(
            fft_speed_idx_ref_.load(std::memory_order::relaxed)));
        if (fft_speed_idx != fft_speed_idx_) {
            fft_speed_idx_ = fft_speed_idx;
            to_update_decay_.signal();
        }
        if (to_update_decay_.check()) {
            const auto refresh_rate = refresh_rate_.load(std::memory_order::relaxed);
            const auto decay_speed = zlstate::PFFTSpeed::kSpeeds[
                static_cast<size_t>(fft_speed_idx_)] * spectrum_extra_decay_speed_.load(std::memory_order::relaxed);
            for (auto& decayer : decayers_) {
                decayer.setDecaySpeed(refresh_rate, c_fft_min_db_, static_cast<float>(0.15 / decay_speed));
            }
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
            c_width_ = width_.load(std::memory_order::relaxed) * kFFTSizeOverWidth;
            c_width_ *= static_cast<float>(std::log((sample_rate * .5 - 0.1) * 0.1) / std::log(fft_max * 0.1));
            const auto delta_freq = static_cast<float>(sample_rate / static_cast<double>(fft_size_));
            const auto temp_scale = static_cast<float>(1.0 / std::log(sample_rate * 0.5 / 10.0)) * c_width_;
            const auto temp_bias = std::log(static_cast<float>(10.0)) * temp_scale;
            num_point_ = xs_.size();
            xs_[0] = std::log(delta_freq * 0.5f) * temp_scale - temp_bias;
            for (size_t i = 1; i < xs_.size(); ++i) {
                const auto freq = delta_freq * static_cast<float>(i);
                xs_[i] = std::log(freq) * temp_scale - temp_bias;
                if (xs_[i] > c_width_) {
                    num_point_ = i + 1;
                    break;
                }
            }
            inter_xs_[0] = xs_[0];
            inter_xs_[kInterSize] = xs_[kInterSize / 2];
            inter_xs_[kInterSize + 1] = xs_[kInterSize / 2 + 1];
            const auto delta_inter_x = (xs_[kInterSize / 2 - 1] - xs_[0]) / static_cast<float>(kInterSize - 1);
            for (size_t i = 1; i < kInterSize; ++i) {
                inter_xs_[i] = inter_xs_[i - 1] + delta_inter_x;
            }
        }
        // update ys para
        if (to_update_ys_para_.check()) {
            c_height_ = height_.load(std::memory_order::relaxed);
            const auto font_size = font_size_.load(std::memory_order::relaxed);
            const auto bottom_area_height = getBottomAreaHeight(font_size);
            const auto height0 = font_size * kDraggerScale;
            const auto height1 = c_height_ - static_cast<float>(bottom_area_height) - height0;
            y_b_ = height0;
            y_k_ = (height1 - height0) / c_fft_min_db_;
        }
        if (num_point_ < 3) {
            return;
        }
        // update each path
        const auto fft_stereo = static_cast<zldsp::analyzer::StereoType>(std::round(
            stereo_ref_.load(std::memory_order::relaxed)));
        const auto fft_frozen = is_fft_frozen_.load(std::memory_order::relaxed);
        for (size_t i = 0; i < 3; i++) {
            if (!is_on[i]) {
                continue;
            }
            receivers_[i].forward(fft_stereo);
            auto& spectrum{receivers_[i].getAbsSqrFFTBuffer()};
            smoother_.smooth(spectrum);
            zldsp::vector::sqr_mag_to_db(spectrum.data(), spectrum.size());
            tilter_.tilt(std::span{spectrum.data(), spectrum.size()});
            decayers_[i].decay(std::span{spectrum.data(), spectrum.size()}, fft_frozen);
            zldsp::vector::fma(ys_.data(), spectrum.data(), y_k_, y_b_, num_point_);
            inter_->prepare();
            inter_->eval(inter_xs_.data(), inter_ys_.data(), kInterSize + 2);

            auto& path{paths_[i].getWriter()};
            path.clear();
            PathMinimizer<5> minimizer{path};
            path.startNewSubPath(inter_xs_.front() - .1f, c_height_ * 1.5f);
            path.lineTo(inter_xs_[0], inter_ys_[0]);
            for (size_t j = 1; j < kInterSize; j += 2) {
                path.quadraticTo(inter_xs_[j], inter_ys_[j], inter_xs_[j + 1], inter_ys_[j + 1]);
            }
            path.lineTo(inter_xs_[kInterSize], inter_ys_[kInterSize]);
            minimizer.startNewSubPath<false>(xs_[kInterSize / 2], ys_[kInterSize / 2]);
            for (size_t j = kInterSize / 2 + 1; j < num_point_; ++j) {
                minimizer.lineTo(xs_[j], ys_[j]);
            }
            minimizer.finish();
            path.lineTo(xs_[num_point_ - 1] + .1f, c_height_ * 1.5f);
            path.closeSubPath();
            if (thread.threadShouldExit()) {
                return;
            }
        }
        for (size_t i = 0; i < 3; i++) {
            if (is_on[i]) {
                paths_[i].publish();
            }
        }
        if (thread.threadShouldExit()) {
            return;
        }
        // update collision
        if (coll_on && ((pre_on && post_on) || (side_on && post_on))) {
            if (side_on) {
                zldsp::analyzer::SpectrumCollision<float>::createGradientPs(
                    receivers_[1].getAbsSqrFFTBuffer(), receivers_[2].getAbsSqrFFTBuffer(),
                    current_ps_, coll_ps_, coll_strength_ref_.load(std::memory_order::relaxed));
            } else {
                zldsp::analyzer::SpectrumCollision<float>::createGradientPs(
                    receivers_[1].getAbsSqrFFTBuffer(), receivers_[0].getAbsSqrFFTBuffer(),
                    current_ps_, coll_ps_, coll_strength_ref_.load(std::memory_order::relaxed));
            }
            if (thread.threadShouldExit()) {
                return;
            }
            const auto width = width_.load(std::memory_order::relaxed);
            auto& gradient{gradient_.getWriter()};
            gradient.clearColours();
            gradient.point1 = {0.f, 0.f};
            gradient.point2 = {width, 0.f};
            gradient.isRadial = false;
            GradientMinimizer gradient_minimizer(gradient, base_.getColourByIdx(zlgui::kCollisionColour));
            gradient_minimizer.start(0.f, 0.f);
            for (size_t i = 1; i < num_point_ - 1; ++i) {
                gradient_minimizer.addColour(xs_[i] / width, coll_ps_[i]);
            }
            gradient_minimizer.addColour(xs_[num_point_ - 1], coll_ps_[num_point_ - 1]);
            gradient_minimizer.finish();
            gradient_.publish();
        }
    }

    void FFTPanel::setRefreshRate(const double refresh_rate) {
        refresh_rate_.store(static_cast<float>(refresh_rate), std::memory_order::relaxed);
        to_update_decay_.signal();
    }

    void FFTPanel::lookAndFeelChanged() {
        const auto extra_speed = base_.getFFTExtraSpeed();
        spectrum_extra_decay_speed_.store(extra_speed * extra_speed + 0.1f, std::memory_order::relaxed);
        to_update_decay_.signal();

        spectrum_extra_tilt_slope_.store(base_.getFFTExtraTilt(), std::memory_order::relaxed);
        to_update_tilt_.signal();
    }

    void FFTPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kFFTFrozen, property)) {
            const auto is_fft_frozen = base_.getPanelProperty(zlgui::PanelSettingIdx::kFFTFrozen);
            is_fft_frozen_.store(is_fft_frozen, std::memory_order::relaxed);
        }
    }
}
