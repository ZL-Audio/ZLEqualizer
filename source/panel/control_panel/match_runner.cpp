// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_runner.hpp"

namespace zlpanel {
    MatchRunner::MatchRunner(PluginProcessor& p, zlgui::UIBase& base,
                             zlgui::slider::CompactLinearSlider<false, false, false>& num_band_slider) :
        Thread("match_runner"),
        p_ref_(p), base_(base), num_band_slider_(num_band_slider) {
        num_band_slider_.getSlider().addListener(this);
    }

    MatchRunner::~MatchRunner() {
        num_band_slider_.getSlider().removeListener(this);
        stopThread(-1);
    }

    void MatchRunner::run() {
        std::vector<float> freqs;
        std::vector<float> diffs;
        freqs.reserve(zlp::Controller::kAnalyzerPointNum);
        diffs.reserve(zlp::Controller::kAnalyzerPointNum);
        p_ref_.getController().getEQMatchAnalyzer().saveFreq(freqs);
        p_ref_.getController().getEQMatchAnalyzer().saveDiffs(diffs);
        zldsp::eq_match::EqMatchOptimizer optimizer{p_ref_.getAtomicSampleRate(), freqs, diffs};

        std::vector<zldsp::filter::FilterParameters> paras{};
        const auto suggest_num_band = optimizer.fit(paras, zlp::kBandNum,
                                                    [this]() { return this->threadShouldExit(); },
                                                    1.0);
        {
            std::lock_guard<std::mutex> lock{mutex_};
            suggest_num_band_ = suggest_num_band;
            paras_.assign(paras.begin(), paras.end());
        }
        triggerAsyncUpdate();
    }

    void MatchRunner::handleAsyncUpdate() {
        size_t suggest_num_band;
        {
            std::lock_guard<std::mutex> lock{mutex_};
            suggest_num_band = suggest_num_band_;
            copy_paras_.assign(paras_.begin(), paras_.end());
        }
        num_band_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(
            0.0, static_cast<double>(copy_paras_.size()), 1.0));
        num_band_slider_.getSlider().setValue(static_cast<double>(suggest_num_band), juce::dontSendNotification);
        num_band_slider_.getSlider().setDoubleClickReturnValue(true, static_cast<double>(suggest_num_band));
        num_band_slider_.updateDisplayValue();
        sliderValueChanged(&num_band_slider_.getSlider());
        base_.setPanelProperty(zlgui::PanelSettingIdx::kMatchPanel, 3.0);
        num_band_slider_.setAlpha(1.f);
        num_band_slider_.setInterceptsMouseClicks(true, true);
    }

    void MatchRunner::sliderValueChanged(juce::Slider*) {
        const auto& parameters{p_ref_.parameters_};
        const auto num_band =
            std::min(static_cast<size_t>(num_band_slider_.getSlider().getValue()), copy_paras_.size());
        for (size_t band = 0; band < num_band; ++band) {
            const auto band_s = std::to_string(band);
            const auto& paras{copy_paras_[band]};
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
                auto* para = parameters.getParameter(IDs[i] + band_s);
                updateValue(para, para->convertTo0to1(values[i]));
            }
        }
        for (size_t band = num_band; band < zlp::kBandNum; ++band) {
            const auto band_s = std::to_string(band);
            auto* para = parameters.getParameter(zlp::PFilterStatus::kID + band_s);
            updateValue(para, 0.f);
        }
    }
}
