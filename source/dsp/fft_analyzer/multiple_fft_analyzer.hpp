// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../state/state_definitions.hpp"
#include "../interpolation/interpolation.hpp"
#include "../fft/fft.hpp"

namespace zldsp::analyzer {
    /**
     * a fft analyzer which make sure that multiple FFTs are synchronized in time
     * @tparam FloatType the float type of input audio buffers
     * @tparam FFTNum the number of FFTs
     * @tparam PointNum the number of output points
     */
    template<typename FloatType, size_t FFTNum, size_t PointNum>
    class MultipleFFTAnalyzer final {
    private:
        juce::SpinLock spin_lock_;
        static constexpr float kMinFreq = 10.f, kMaxFreq = 22000.f, kMinDB = -72.f;
        static constexpr float kMinFreqLog2 = 3.321928094887362f;
        static constexpr float kMaxFreqLog2 = 14.425215903299383f;

    public:
        explicit MultipleFFTAnalyzer(const size_t fft_order = 12) {
            default_fft_order_ = fft_order;
            bin_size_ = (1 << (default_fft_order_ - 1)) + 1;

            for (auto &db: smoothed_dbs_) {
                db.resize(bin_size_);
            }

            prepareAkima();

            tilt_slope_.store(zlstate::ffTTilt::slopes[static_cast<size_t>(zlstate::ffTTilt::defaultI)]);
            interplot_freqs_[0] = kMinFreq;
            for (size_t i = 1; i < PointNum; ++i) {
                const float temp = static_cast<float>(i) / static_cast<float>(PointNum - 1) * (
                                       kMaxFreqLog2 - kMinFreqLog2) + kMinFreqLog2;
                interplot_freqs_[i] = std::pow(2.f, temp);
            }
            for (auto &f: ready_flags_) {
                f.store(false);
            }
            for (auto &db: interplot_dbs_) {
                std::fill(db.begin(), db.end(), kMinDB * 2.f);
            }
            reset();
        }

        ~MultipleFFTAnalyzer() = default;

        void prepareAkima() {
            std::vector<size_t> seq_input_indices{};
            seq_input_indices.push_back(0);
            size_t i = 1, i0 = 1;
            const float delta = std::pow(
                static_cast<float>(bin_size_), .75f / static_cast<float>(PointNum));
            while (i < bin_size_ - 1) {
                while (static_cast<float>(i) / static_cast<float>(i0) < delta) {
                    i += 1;
                    if (i >= bin_size_ - 1) {
                        break;
                    }
                }
                i0 = i;
                seq_input_indices.push_back(i);
            }

            seq_input_starts_.reserve(seq_input_indices.size());
            seq_input_ends_.reserve(seq_input_indices.size());
            seq_input_starts_.push_back(0);
            seq_input_ends_.push_back(1);
            for (size_t idx = 1; idx < seq_input_indices.size() - 1; ++idx) {
                seq_input_starts_.push_back(seq_input_ends_.back());
                seq_input_ends_.push_back(
                    static_cast<std::vector<float>::difference_type>(
                        seq_input_indices[idx] + seq_input_indices[idx + 1]) / 2);
            }
            seq_input_starts_.push_back(seq_input_ends_.back());
            seq_input_ends_.push_back(static_cast<std::vector<float>::difference_type>(bin_size_) - 1);

            seq_input_freqs_.resize(seq_input_indices.size());
            seq_input_dbs_.resize(seq_input_indices.size());
            seq_akima_ = std::make_unique<zldsp::interpolation::SeqMakima<float> >(
                seq_input_freqs_.data(), seq_input_dbs_.data(), seq_input_freqs_.size(), 0.f, 0.f);
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            juce::GenericScopedLock lock(spin_lock_);
            sample_rate_.store(static_cast<float>(spec.sampleRate));
            if (spec.sampleRate <= 50000) {
                setOrder(static_cast<int>(default_fft_order_));
            } else if (spec.sampleRate <= 100000) {
                setOrder(static_cast<int>(default_fft_order_) + 1);
            } else if (spec.sampleRate <= 200000) {
                setOrder(static_cast<int>(default_fft_order_) + 2);
            } else {
                setOrder(static_cast<int>(default_fft_order_) + 3);
            }
            reset();
            is_prepared_.store(true);
        }

        void reset() {
            for (auto &f: to_reset_) {
                f.store(true);
            }
        }

        void setOrder(int fft_order) {
            fft_.setOrder(static_cast<size_t>(fft_order));
            window_.setWindow(fft_.getSize(), juce::dsp::WindowingFunction<float>::hann,
                             1.f / static_cast<float>(fft_.getSize()), true, true);
            fft_size_.store(fft_.getSize());

            delta_t_.store(sample_rate_.load() / static_cast<float>(fft_size_.load()));
            decay_rate_.store(zlstate::ffTSpeed::speeds[static_cast<size_t>(zlstate::ffTSpeed::defaultI)]);

            const auto currentDeltaT = .5f * delta_t_.load();
            for (size_t idx = 0; idx < seq_input_freqs_.size(); ++idx) {
                seq_input_freqs_[idx] = static_cast<float>(seq_input_starts_[idx] + seq_input_ends_[idx] - 1) * currentDeltaT;
            }
            for (size_t i = 0; i < FFTNum; ++i) {
                std::fill(smoothed_dbs_[i].begin(), smoothed_dbs_[i].end(), kMinDB * 2.f);
            }

            const auto tempSize = fft_.getSize();
            fft_buffer_.resize(tempSize * 2);
            abstract_fifo_.setTotalSize(static_cast<int>(tempSize));
            for (size_t i = 0; i < FFTNum; ++i) {
                sample_fifos_[i].resize(tempSize);
                circular_buffers_[i].resize(tempSize);
            }
        }

        /**
         * put input samples into FIFOs
         * @param buffers
         */
        void process(std::array<std::reference_wrapper<juce::AudioBuffer<FloatType> >, FFTNum> buffers) {
            int free_space = abstract_fifo_.getFreeSpace();
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!is_on_[i].load()) continue;
                free_space = std::min(free_space, buffers[i].get().getNumSamples());
            }
            if (free_space == 0) { return; }
            const auto scope = abstract_fifo_.write(free_space);
            for (size_t i = 0; i < FFTNum; ++i) {
                if (!is_on_[i].load()) continue;
                int j = 0;
                const auto &buffer{buffers[i]};
                int shift = 0;
                for (; j < scope.blockSize1; ++j) {
                    FloatType sample{0};
                    for (int channel = 0; channel < buffer.get().getNumChannels(); ++channel) {
                        sample += buffer.get().getSample(channel, j);
                    }
                    sample_fifos_[i][static_cast<size_t>(shift + scope.startIndex1)] = static_cast<float>(sample);
                    shift += 1;
                }
                shift = 0;
                for (; j < scope.blockSize1 + scope.blockSize2; ++j) {
                    FloatType sample{0};
                    for (int channel = 0; channel < buffer.get().getNumChannels(); ++channel) {
                        sample += buffer.get().getSample(channel, j);
                    }
                    sample_fifos_[i][static_cast<size_t>(shift + scope.startIndex2)] = static_cast<float>(sample);
                    shift += 1;
                }
            }
        }

        /**
         * run the forward FFT
         */
        void run() {
            juce::GenericScopedLock lock(spin_lock_);
            if (!is_prepared_.load()) {
                return;
            }
            std::vector<size_t> is_on_vector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (is_on_[i].load()) is_on_vector.push_back(i);
            }

            juce::ScopedNoDenormals no_denormals; {
                const int num_ready = abstract_fifo_.getNumReady();
                const auto scope = abstract_fifo_.read(num_ready);
                const size_t num_replace = circular_buffers_[0].size() - static_cast<size_t>(num_ready);
                for (const auto &i: is_on_vector) {
                    auto &circular_buffer{circular_buffers_[i]};
                    auto &sample_fifo{sample_fifos_[i]};
                    size_t j = 0;
                    for (; j < num_replace; ++j) {
                        circular_buffer[j] = circular_buffer[j + static_cast<size_t>(num_ready)];
                    }
                    int shift = 0;
                    for (; j < num_replace + static_cast<size_t>(scope.blockSize1); ++j) {
                        circular_buffer[j] = sample_fifo[static_cast<size_t>(shift + scope.startIndex1)];
                        shift += 1;
                    }
                    shift = 0;
                    for (; j < num_replace + static_cast<size_t>(scope.blockSize1 + scope.blockSize2); ++j) {
                        circular_buffer[j] = sample_fifo[static_cast<size_t>(shift + scope.startIndex2)];
                        shift += 1;
                    }
                }
            } {
                for (const auto &i: is_on_vector) {
                    std::copy(circular_buffers_[i].begin(), circular_buffers_[i].end(), fft_buffer_.begin());
                    window_.multiply(fft_buffer_.data(), fft_size_.load());
                    fft_.forwardMagnitudeOnly(fft_buffer_.data());
                    const auto decay = actual_decay_rate_[i].load();
                    auto &smoothed_db{smoothed_dbs_[i]};
                    if (to_reset_[i].exchange(false)) {
                        std::fill(smoothed_db.begin(), smoothed_db.end(), kMinDB * 2.f);
                    }
                    for (size_t j = 0; j < smoothed_db.size(); ++j) {
                        const auto currentDB = juce::Decibels::gainToDecibels(fft_buffer_[j], -240.f);
                        smoothed_db[j] = currentDB < smoothed_db[j]
                                            ? smoothed_db[j] * decay + currentDB * (1 - decay)
                                            : currentDB;
                    }

                    for (size_t j = 0; j < seq_input_dbs_.size(); ++j) {
                        const auto startIdx = seq_input_starts_[j];
                        const auto endIdx = seq_input_ends_[j];
                        seq_input_dbs_[j] = std::reduce(
                                             smoothed_db.begin() + startIdx,
                                             smoothed_db.begin() + endIdx) / static_cast<float>(endIdx - startIdx);
                    }

                    seq_akima_->prepare();
                    seq_akima_->eval(interplot_freqs_.data(), pre_interplot_dbs_[i].data(), PointNum);
                }
            } {
                const float total_tilt = tilt_slope_.load() + extra_tilt_.load();
                const float tilt_shift_total = (kMaxFreqLog2 - kMinFreqLog2) * total_tilt;
                const float tilt_shift_delta = tilt_shift_total / static_cast<float>(PointNum - 1);

                for (const auto &i: is_on_vector) {
                    if (ready_flags_[i].load() == false) {
                        float tiltShift = -tilt_shift_total * .5f;
                        for (size_t idx = 0; idx < PointNum; ++idx) {
                            interplot_dbs_[i][idx] = tiltShift + pre_interplot_dbs_[i][idx];
                            tiltShift += tilt_shift_delta;
                        }
                        ready_flags_[i].store(true);
                    }
                }
            }
        }

        void createPath(std::array<std::reference_wrapper<juce::Path>, FFTNum> paths,
                        const juce::Rectangle<float> bound,
                        const float current_min_db = -72.f) {
            for (auto &p: paths) {
                p.get().clear();
            }
            std::vector<size_t> is_on_vector{};
            for (size_t i = 0; i < FFTNum; ++i) {
                if (is_on_[i].load()) is_on_vector.push_back(i);
            }
            for (const auto &i: is_on_vector) {
                if (ready_flags_[i].load() == true) {
                    ready_dbs_[i] = interplot_dbs_[i];
                    ready_flags_[i].store(false);
                }
            }
            constexpr auto cubic_num = (PointNum / 7) * 6;
            const float width = bound.getWidth(), height = bound.getHeight(), boundY = bound.getY();
            for (const auto &i: is_on_vector) {
                const auto &path{paths[i]};
                path.get().startNewSubPath(bound.getX(), bound.getBottom() + 10.f);
                for (size_t idx = 0; idx < PointNum - cubic_num; ++idx) {
                    const auto x = static_cast<float>(idx) / static_cast<float>(PointNum - 1) * width;
                    const auto y = replaceWithFinite(ready_dbs_[i][idx] / current_min_db * height + boundY);
                    path.get().lineTo(x, y);
                }
                for (size_t idx = PointNum - cubic_num; idx < PointNum - 2; idx += 3) {
                    const auto x1 = static_cast<float>(idx) / static_cast<float>(PointNum - 1) * width;
                    const auto y1 = replaceWithFinite(ready_dbs_[i][idx] / current_min_db * height + boundY);
                    const auto x2 = static_cast<float>(idx + 1) / static_cast<float>(PointNum - 1) * width;
                    const auto y2 = replaceWithFinite(ready_dbs_[i][idx + 1] / current_min_db * height + boundY);
                    const auto x3 = static_cast<float>(idx + 2) / static_cast<float>(PointNum - 1) * width;
                    const auto y3 = replaceWithFinite(ready_dbs_[i][idx + 2] / current_min_db * height + boundY);
                    path.get().cubicTo(x1, y1, x2, y2, x3, y3);
                }
            }
        }

        void setON(std::array<bool, FFTNum> fs) {
            for (size_t i = 0; i < FFTNum; ++i) {
                is_on_[i].store(fs[i]);
            }
        }

        inline size_t getFFTSize() const { return fft_size_.load(); }

        void setDecayRate(const size_t idx, const float x) {
            decay_rates_[idx].store(x);
            updateActualDecayRate();
        }

        void setRefreshRate(const float x) {
            refresh_rate_.store(x);
            updateActualDecayRate();
        }

        void setTiltSlope(const float x) { tilt_slope_.store(x); }

        void setExtraTilt(const float x) { extra_tilt_.store(x); }

        void setExtraSpeed(const float x) {
            extra_speed_.store(x);
            updateActualDecayRate();
        }

        void updateActualDecayRate() {
            for (size_t i = 0; i < FFTNum; ++i) {
                const auto x = 1 - (1 - decay_rates_[i].load()) * extra_speed_.load();
                actual_decay_rate_[i].store(std::pow(x, 23.4375f / refresh_rate_.load()));
            }
        }

        std::array<float, PointNum> &getInterplotDBs(const size_t i) {
            if (ready_flags_[i].load() == true) {
                ready_dbs_[i] = interplot_dbs_[i];
                ready_flags_[i].store(false);
            }
            return ready_dbs_[i];
        }

    private:
        size_t default_fft_order_ = 12;
        size_t bin_size_ = (1 << (default_fft_order_ - 1)) + 1;

        std::array<std::vector<float>, FFTNum> sample_fifos_;
        std::array<std::vector<float>, FFTNum> circular_buffers_;
        juce::AbstractFifo abstract_fifo_{1};

        std::vector<float> fft_buffer_;

        // smooth dbs over time
        std::array<std::vector<float>, FFTNum> smoothed_dbs_{};
        // smooth dbs over high frequency for Akimas input
        std::vector<float> seq_input_freqs_{};
        std::vector<std::vector<float>::difference_type> seq_input_starts_, seq_input_ends_;
        std::vector<size_t> seq_input_indices_;
        std::vector<float> seq_input_dbs_{};

        std::unique_ptr<zldsp::interpolation::SeqMakima<float> > seq_akima_;

        std::array<float, PointNum> interplot_freqs_{};
        std::array<std::array<float, PointNum>, FFTNum> pre_interplot_dbs_{};
        std::array<std::array<float, PointNum>, FFTNum> interplot_dbs_{};
        std::array<std::array<float, PointNum>, FFTNum> ready_dbs_{};
        std::array<std::atomic<bool>, FFTNum> ready_flags_;

        std::atomic<float> delta_t_, decay_rate_, refresh_rate_{60}, tilt_slope_;
        std::array<std::atomic<float>, FFTNum> decay_rates_{}, actual_decay_rate_{};
        std::atomic<float> extra_tilt_{0.f}, extra_speed_{1.f};

        zldsp::fft::KFREngine<float> fft_;
        zldsp::fft::WindowFunction<float> window_;
        std::atomic<size_t> fft_size_;

        std::atomic<float> sample_rate_;
        std::array<std::atomic<bool>, FFTNum> to_reset_;
        std::atomic<bool> is_prepared_{false};

        std::array<std::atomic<bool>, FFTNum> is_on_{};

        static inline float replaceWithFinite(const float x) {
            return std::isfinite(x) ? x : 100000.f;
        }
    };
}
