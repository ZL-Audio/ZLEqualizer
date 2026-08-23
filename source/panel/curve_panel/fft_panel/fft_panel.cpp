// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"
#include "../../../zlp/sample_rate_helper.hpp"

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
        fft_top_db_idx_ref_(*p.parameters_NA_.getRawParameterValue(zlstate::PFFTTopDB::kID)),
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
        collision_colour_ = base_.getColourByIdx(zlgui::kCollisionColour);
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
        const std::array<bool, kNumSources> is_on{pre_on, post_on, side_on};
        for (size_t i = 0; i < kNumSources; ++i) {
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
        const std::array<bool, kNumSources> is_on{pre_on, post_on, side_on};
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
            const auto middle_fft_order = static_cast<int>(zlp::getScaledOrder(sample_rate, 12));
            const std::array fft_orders{
                middle_fft_order + 2, middle_fft_order, middle_fft_order - 2
            };
            for (size_t i = 0; i < processors_.size(); ++i) {
                processors_[i].prepare(fft_orders[i]);
            }
            // equalize the expected white-noise power of the normalized Hann windows
            const auto reference_window_power = processors_[kMiddleResolution].getWindowSqrSum();
            for (size_t i = 0; i < processors_.size(); ++i) {
                noise_power_scales_[i] = static_cast<float>(
                    reference_window_power / processors_[i].getWindowSqrSum());
            }
            history_size_ = static_cast<int>(processors_[kLowResolution].getFFTSize());
            for (auto& receiver : receivers_) {
                receiver.prepare(2);
            }
            for (size_t i = 0; i < smoothers_.size(); ++i) {
                smoothers_[i].prepare(processors_[i].getFFTSize());
            }
            update_smooth = true;
            frequencies_ = zldsp::analyzer::SpectrumBlender::createFrequencyGrid(
                processors_[kLowResolution].getFFTSize(),
                processors_[kMiddleResolution].getFFTSize(),
                processors_[kHighResolution].getFFTSize(), sample_rate);
            tilter_.prepareSpectrum(frequencies_.size());
            for (auto& decayer : decayers_) {
                decayer.prepareSpectrum(frequencies_.size());
            }

            xs_.resize(frequencies_.size());
            ys_.resize(frequencies_.size());

            current_ps_.resize(frequencies_.size());
            coll_ps_.resize(frequencies_.size());
            std::ranges::fill(current_ps_, 0.f);
            std::ranges::fill(coll_ps_, 0.f);
            for (size_t source = 0; source < resolution_spectra_.size(); ++source) {
                spectra_[source].resize(frequencies_.size());
                for (size_t resolution = 0; resolution < processors_.size(); ++resolution) {
                    resolution_spectra_[source][resolution].resize(
                        processors_[resolution].getFFTSize() / 2 + 1);
                }
            }

            to_update_xs_para_.signal();
            to_update_ys_para_.signal();
        }
        // receiver pull data
        auto& fifo{sender.getAbstractFIFO()};
        auto num_read = fifo.getNumReady() / 4 * 3;
        if (num_read > history_size_) {
            (void)fifo.prepareToRead(num_read - history_size_);
            fifo.finishRead(num_read - history_size_);
            num_read = history_size_;
        }
        const auto range = fifo.prepareToRead(num_read);
        for (size_t i = 0; i < kNumSources; i++) {
            if (is_on[i]) {
                receivers_[i].pull(range, sender.getSampleFIFOs()[i]);
            }
        }
        fifo.finishRead(num_read);
        sender.getLock().unlock();
        if (thread.threadShouldExit()) {
            return;
        }
        if (history_size_ <= 0) {
            return;
        }
        // update analyzer db range
        const auto top_db = zlstate::PFFTTopDB::kDBs[static_cast<size_t>(std::round(
            fft_top_db_idx_ref_.load(std::memory_order::relaxed)))];
        if (std::abs(top_db - c_fft_top_db_) > .1f) {
            c_fft_top_db_ = top_db;
            to_update_decay_.signal();
            to_update_ys_para_.signal();
        }
        const auto range_db = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(
            fft_min_db_idx_ref_.load(std::memory_order::relaxed)))];
        if (std::abs(range_db - c_fft_range_db_) > .1f) {
            c_fft_range_db_ = range_db;
            to_update_decay_.signal();
            to_update_ys_para_.signal();
        }
        // update tilt
        const auto fft_tilt_idx = static_cast<int>(std::round(
            fft_tilt_idx_ref_.load(std::memory_order::relaxed)));
        if (fft_tilt_idx != fft_tilt_idx_) {
            fft_tilt_idx_ = fft_tilt_idx;
            to_update_tilt_.signal();
        }
        if (to_update_tilt_.check()) {
            tilter_.setTiltSlope(
                frequencies_,
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
                decayer.setDecaySpeed(refresh_rate, c_fft_top_db_ + c_fft_range_db_,
                                      static_cast<float>(0.15 / decay_speed));
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
                for (auto& smoother : smoothers_) {
                    smoother.setSmoothOCT(
                        zlstate::PFFTSmoothOCTValue::kValues[static_cast<size_t>(fft_smooth_oct_value_idx)]);
                }
            } else {
                for (auto& smoother : smoothers_) {
                    smoother.setSmoothERB(
                        sample_rate,
                        zlstate::PFFTSmoothERBValue::kValues[static_cast<size_t>(fft_smooth_erb_value_idx)]);
                }
            }
        }
        // update xs para
        if (to_update_xs_para_.check()) {
            const auto fft_max = freq_helper::getFFTMax(sample_rate);
            c_width_ = width_.load(std::memory_order::relaxed) * kFFTSizeOverWidth;
            c_width_ *= static_cast<float>(std::log((sample_rate * .5 - 0.1) * 0.1) / std::log(fft_max * 0.1));
            const auto temp_scale = static_cast<float>(1.0 / std::log(sample_rate * 0.5 / 10.0)) * c_width_;
            const auto temp_bias = std::log(static_cast<float>(10.0)) * temp_scale;
            num_point_ = xs_.size();
            xs_[0] = std::log(frequencies_[1] * 0.5f) * temp_scale - temp_bias;
            for (size_t i = 1; i < xs_.size(); ++i) {
                xs_[i] = std::log(frequencies_[i]) * temp_scale - temp_bias;
                if (xs_[i] > c_width_) {
                    num_point_ = i + 1;
                    break;
                }
            }
        }
        // update ys para
        if (to_update_ys_para_.check()) {
            c_height_ = height_.load(std::memory_order::relaxed);
            const auto font_size = font_size_.load(std::memory_order::relaxed);
            const auto bottom_area_height = getBottomAreaHeight(font_size);
            const auto height0 = font_size * kDraggerScale;
            const auto height1 = c_height_ - static_cast<float>(bottom_area_height) - height0;
            y_k_ = (height1 - height0) / c_fft_range_db_;
            y_b_ = height0 - c_fft_top_db_ * y_k_;
        }
        if (num_point_ < 3) {
            return;
        }
        // update each path
        const auto fft_stereo = static_cast<zldsp::analyzer::StereoType>(std::round(
            stereo_ref_.load(std::memory_order::relaxed)));
        const auto fft_frozen = is_fft_frozen_.load(std::memory_order::relaxed);
        for (size_t i = 0; i < kNumSources; i++) {
            if (!is_on[i]) {
                continue;
            }
            for (size_t resolution = 0; resolution < kNumResolutions; ++resolution) {
                auto& resolution_spectrum = resolution_spectra_[i][resolution];
                receivers_[i].forward(processors_[resolution], fft_stereo, resolution_spectrum);
                zldsp::vector::multiply(resolution_spectrum.data(), noise_power_scales_[resolution],
                                        resolution_spectrum.size());
                smoothers_[resolution].smooth(resolution_spectrum);
            }
            auto& spectrum{spectra_[i]};
            zldsp::analyzer::SpectrumBlender::blend(
                spectrum, frequencies_,
                resolution_spectra_[i][kLowResolution],
                resolution_spectra_[i][kMiddleResolution],
                resolution_spectra_[i][kHighResolution],
                sample_rate);
            zldsp::vector::sqr_mag_to_db(spectrum.data(), spectrum.size());
            tilter_.tilt(std::span{spectrum.data(), spectrum.size()});
            decayers_[i].decay(std::span{spectrum.data(), spectrum.size()}, fft_frozen);
            zldsp::vector::fma(ys_.data(), spectrum.data(), y_k_, y_b_, num_point_);

            auto& path{paths_[i].getWriter()};
            path.clear();
            PathMinimizer<5> minimizer{path};
            path.startNewSubPath(xs_.front() - .1f, c_height_ * 1.5f);
            minimizer.startNewSubPath<false>(xs_.front(), ys_.front());
            for (size_t j = 1; j < num_point_; ++j) {
                minimizer.lineTo(xs_[j], ys_[j]);
            }
            minimizer.finish();
            path.lineTo(xs_[num_point_ - 1] + .1f, c_height_ * 1.5f);
            path.closeSubPath();
            if (thread.threadShouldExit()) {
                return;
            }
        }
        for (size_t i = 0; i < kNumSources; i++) {
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
                    spectra_[1], spectra_[2],
                    current_ps_, coll_ps_, coll_strength_ref_.load(std::memory_order::relaxed));
            } else {
                zldsp::analyzer::SpectrumCollision<float>::createGradientPs(
                    spectra_[1], spectra_[0],
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
            GradientMinimizer gradient_minimizer(gradient, collision_colour_);
            gradient_minimizer.start(0.f, 0.f);
            for (size_t i = 1; i < num_point_ - 1; ++i) {
                gradient_minimizer.addColour(xs_[i] / width, coll_ps_[i]);
            }
            gradient_minimizer.addColour(xs_[num_point_ - 1] / width, coll_ps_[num_point_ - 1]);
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
