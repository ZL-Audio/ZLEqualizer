// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "button_panel.hpp"

namespace zlpanel {
    ButtonPanel::ButtonPanel(PluginProcessor &processor,
                             zlgui::UIBase &base)
        : processor_ref_(processor),
          parameters_ref_(processor.parameters_), parameters_NA_ref_(processor.parameters_NA_),
          ui_base_(base), controller_ref_(processor.getController()),
          wheel_slider_{
              zlgui::SnappingSlider{base},
              zlgui::SnappingSlider{base},
              zlgui::SnappingSlider{base}
          } {
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            const auto suffix = zlp::appendSuffix("", i);
            freq_updaters_[i] = std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::freq::ID + suffix);
            gain_updaters_[i] = std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::gain::ID + suffix);
            q_updaters_[i] = std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::Q::ID + suffix);
            target_gain_updaters_[i] = std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::targetGain::ID + suffix);
            target_q_updaters_[i] = std::make_unique<zldsp::chore::ParaUpdater>(
                parameters_ref_, zlp::targetQ::ID + suffix);
        }
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            panels_[i] = std::make_unique<FilterButtonPanel>(i, processor_ref_, base);
            link_buttons_[i] = std::make_unique<LinkButtonPanel>(
                i, parameters_ref_, parameters_NA_ref_, base, panels_[i]->getSideDragger());
            // when main dragger is clicked, de-select target & side dragger
            panels_[i]->getDragger().getButton().onStateChange = [this]() {
                const auto idx = band_idx_.load();
                if (panels_[idx]->getDragger().getButton().getToggleState()) {
                    panels_[idx]->getTargetDragger().getButton().setToggleState(false, juce::sendNotification);
                    panels_[idx]->getSideDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            // when main dragger is clicked, de-select target & side dragger
            panels_[i]->getTargetDragger().getButton().onStateChange = [this]() {
                const auto idx = band_idx_.load();
                if (panels_[idx]->getTargetDragger().getButton().getToggleState()) {
                    panels_[idx]->getDragger().getButton().setToggleState(false, juce::sendNotification);
                    panels_[idx]->getSideDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            // when side dragger is clicked, de-select main & side dragger
            panels_[i]->getSideDragger().getButton().onStateChange = [this]() {
                const auto idx = band_idx_.load();
                if (panels_[idx]->getSideDragger().getButton().getToggleState()) {
                    panels_[idx]->getDragger().getButton().setToggleState(false, juce::sendNotification);
                    panels_[idx]->getTargetDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            // deselect the side dragger when link button is clicked
            link_buttons_[i]->getButton().getButton().onClick = [this]() {
                const auto idx = band_idx_.load();
                if (link_buttons_[idx]->getButton().getButton().getToggleState()) {
                    panels_[idx]->getSideDragger().getButton().setToggleState(false, juce::sendNotification);
                }
            };
            panels_[i]->addMouseListener(this, true);
        }
        for (const auto &idx: kNAIDs) {
            parameters_NA_ref_.addParameterListener(idx, this);
            parameterChanged(idx, parameters_NA_ref_.getRawParameterValue(idx)->load());
        }
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            addChildComponent(panels_[i].get());
            addChildComponent(link_buttons_[i].get());
        }
        addAndMakeVisible(lasso_component_);
        items_set_.addChangeListener(this);
    }

    ButtonPanel::~ButtonPanel() {
        for (const auto &idx: kNAIDs) {
            parameters_NA_ref_.removeParameterListener(idx, this);
        }
        for (size_t tempIdx = 0; tempIdx < panels_.size(); ++tempIdx) {
            for (const auto &idx: kIDs) {
                const auto actualIdx = zlp::appendSuffix(idx, tempIdx);
                parameters_ref_.removeParameterListener(actualIdx, this);
            }
        }
        items_set_.removeChangeListener(this);
        wheel_attachment_[0].reset();
        wheel_attachment_[1].reset();
        wheel_attachment_[2].reset();

        stopTimer();
    }

    void ButtonPanel::paint(juce::Graphics &g) {
        if (ui_base_.getColourByIdx(zlgui::kTagColour).getFloatAlpha() < 0.01f) {
            return;
        }
        const auto idx = band_idx_.load();
        const auto &p = panels_[idx];
        if (!p->isVisible()) {
            return;
        }
        const auto bound = p->getDragger().getButtonArea();
        g.setFont(ui_base_.getFontSize() * zlgui::kFontLarge);
        if (p->getDragger().getButton().getToggleState()) {
            const auto &f{controller_ref_.getBaseFilter(idx)};
            const auto button_c = p->getDragger().getButtonPos();
            const auto freq_p = (button_c.getX() - bound.getX()) / bound.getWidth();
            const auto gain_p = (button_c.getY() - bound.getY()) / bound.getHeight();
            drawFilterParas(g, f.getFilterType(), freq_p, gain_p, getLocalBounds().toFloat());
        } else if (p->getTargetDragger().getButton().getToggleState()) {
            const auto &f{controller_ref_.getTargetFilter(idx)};
            const auto button_c = p->getDragger().getButtonPos();
            const auto freq_p = (button_c.getX() - bound.getX()) / bound.getWidth();
            const auto gain_p = (button_c.getY() - bound.getY()) / bound.getHeight();
            drawFilterParas(g, f.getFilterType(), freq_p, gain_p, getLocalBounds().toFloat());
        } else if (p->getSideDragger().getButton().getToggleState()) {
            const auto &f{controller_ref_.getFilter(idx).getSideFilter()};
            const auto button_c = p->getSideDragger().getButtonPos();
            const auto freq_p = (button_c.getX() - bound.getX()) / bound.getWidth();
            const auto gain_p = (button_c.getY() - bound.getY()) / bound.getHeight();
            drawFilterParas(g, f.getFilterType(), freq_p, gain_p, getLocalBounds().toFloat());
        }
    }

    void ButtonPanel::drawFilterParas(juce::Graphics &g, const zldsp::filter::FilterType ftype,
                                      const float freq_p, const float gain_p, const juce::Rectangle<float> &bound) {
        switch (ftype) {
            case zldsp::filter::FilterType::kPeak:
            case zldsp::filter::FilterType::kBandShelf: {
                drawGain(g, gain_p, bound, freq_p < .5f);
                break;
            }
            case zldsp::filter::FilterType::kLowShelf: {
                drawGain(g, gain_p, bound, true);
                break;
            }
            case zldsp::filter::FilterType::kHighShelf: {
                drawGain(g, gain_p, bound, false);
                break;
            }
            case zldsp::filter::FilterType::kTiltShelf: {
                drawGain(g, gain_p * .5f, bound, false);
                break;
            }
            case zldsp::filter::FilterType::kNotch:
            case zldsp::filter::FilterType::kLowPass:
            case zldsp::filter::FilterType::kHighPass:
            case zldsp::filter::FilterType::kBandPass: {
                break;
            }
        }
        drawFreq(g, freq_p, bound, false);
    }

    void ButtonPanel::drawFreq(juce::Graphics &g, const float freq_p, const juce::Rectangle<float> &bound,
                               const bool is_top) {
        juce::ignoreUnused(is_top);
        const auto freq = std::exp(freq_p * std::log(2000.f)) * 10.f;
        const auto freq_string = freq < 100 ? juce::String(freq, 2, false) : juce::String(freq, 1, false);
        const auto p = std::clamp(freq_p, 0.025f, 0.975f) * 0.9873247325443818f;
        auto text_bound = juce::Rectangle<float>(ui_base_.getFontSize() * 5, ui_base_.getFontSize() * 1.5f);
        text_bound = text_bound.withCentre({bound.getWidth() * p, bound.getBottom() - 0.75f * ui_base_.getFontSize()});
        const auto colour = ui_base_.getColourByIdx(zlgui::kTagColour);
        g.setColour(ui_base_.getBackgroundColor().withAlpha(colour.getFloatAlpha()));
        g.fillRect(text_bound);
        g.setColour(colour);
        g.drawText(freq_string, text_bound, juce::Justification::centredBottom, false);
    }

    void ButtonPanel::drawGain(juce::Graphics &g, const float gain_p, const juce::Rectangle<float> &bound,
                               const bool is_left) {
        const auto gain = (-2.f * gain_p + 1.f) * maximum_db_.load();
        const auto temp_bound = bound.withSizeKeepingCentre(bound.getWidth(),
                                                           bound.getHeight() - 2 * ui_base_.getFontSize());
        const auto gain_string = std::abs(gain) < 10 ? juce::String(gain, 2, false) : juce::String(gain, 1, false);
        auto text_bound = juce::Rectangle<float>(ui_base_.getFontSize() * 2.7f, ui_base_.getFontSize() * 1.5f);
        if (is_left) {
            text_bound = text_bound.withCentre({
                ui_base_.getFontSize() * 1.35f,
                temp_bound.getY() + gain_p * temp_bound.getHeight()
            });
        } else {
            text_bound = text_bound.withCentre({
                temp_bound.getRight() - ui_base_.getFontSize() * 1.35f,
                temp_bound.getY() + gain_p * temp_bound.getHeight()
            });
        }
        const auto colour = ui_base_.getColourByIdx(zlgui::kTagColour);
        g.setColour(ui_base_.getBackgroundColor().withAlpha(colour.getFloatAlpha()));
        g.fillRect(text_bound);
        g.setColour(colour);
        g.drawText(gain_string, text_bound, juce::Justification::centred, false);
    }

    void ButtonPanel::resized() {
        for (const auto &p: panels_) {
            p->setBounds(getLocalBounds());
        }
        for (const auto &p: link_buttons_) {
            p->setBounds(getLocalBounds());
        }
    }

    void ButtonPanel::mouseEnter(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        startTimer(500);
    }

    void ButtonPanel::mouseExit(const juce::MouseEvent &event) {
        juce::ignoreUnused(event);
        stopTimer();
    }

    void ButtonPanel::timerCallback() {
        ui_base_.closeAllBox();
        stopTimer();
    }

    void ButtonPanel::mouseDown(const juce::MouseEvent &event) {
        ui_base_.closeAllBox();
        if (event.originalComponent != this) {
            is_left_click_.store(!event.mods.isRightButtonDown());
            return;
        }
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            panels_[i]->setSelected(false);
        }

        items_set_.deselectAll();
        previous_lasso_num_ = 0;
        lasso_component_.setColour(juce::LassoComponent<size_t>::lassoFillColourId,
                                 ui_base_.getTextColor().withMultipliedAlpha(.25f));
        lasso_component_.setColour(juce::LassoComponent<size_t>::lassoOutlineColourId,
                                 ui_base_.getTextColor().withMultipliedAlpha(.375f));
        lasso_component_.setVisible(true);
        lasso_component_.beginLasso(event, this);
    }

    void ButtonPanel::mouseUp(const juce::MouseEvent &event) {
        if (event.originalComponent != this) {
            return;
        }
        lasso_component_.endLasso();
    }

    void ButtonPanel::mouseDrag(const juce::MouseEvent &event) {
        if (event.originalComponent != this) {
            return;
        }
        lasso_component_.dragLasso(event);
    }

    void ButtonPanel::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) {
        juce::MouseEvent e{
            event.source, event.position,
            event.mods.withoutMouseButtons(),
            event.pressure, event.orientation, event.rotation,
            event.tiltX, event.tiltY,
            event.eventComponent, event.originalComponent,
            event.eventTime, event.mouseDownPosition, event.mouseDownTime,
            event.getNumberOfClicks(), false
        };
        if (event.originalComponent == this) {
            wheel_slider_[0].mouseWheelMove(e, wheel);
            wheel_slider_[1].mouseWheelMove(e, wheel);
        } else if (!panels_[band_idx_.load()]->isParentOf(event.originalComponent)) {
            wheel_slider_[0].mouseWheelMove(e, wheel);
            wheel_slider_[1].mouseWheelMove(e, wheel);
        } else {
            const auto &p = panels_[band_idx_.load()];
            if (p.get()->getDragger().isParentOf(event.originalComponent)) {
                wheel_slider_[0].mouseWheelMove(e, wheel);
            } else if (p.get()->getTargetDragger().isParentOf(event.originalComponent)) {
                wheel_slider_[1].mouseWheelMove(e, wheel);
            } else if (p.get()->getSideDragger().isParentOf(event.originalComponent)) {
                if (!p.get()->getSideDragger().getButton().getToggleState()) {
                    p.get()->getSideDragger().getButton().setToggleState(true, juce::sendNotificationSync);
                }
                wheel_slider_[2].mouseWheelMove(e, wheel);
            }
        }
    }

    void ButtonPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (event.originalComponent != this) {
            return;
        }
        const auto idx = findAvailableBand();
        if (idx >= zlstate::kBandNUM) {
            return;
        }
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * ui_base_.getFontSize());
        const auto point = event.getPosition().toFloat();
        const auto x = point.getX(), y = point.getY();
        const auto freq = xtoFreq(x, bound);
        const auto db = yToDB(y, maximum_db_.load(), bound);

        std::vector<std::string> init_IDs;
        std::vector<float> init_values;

        if (freq < 20.f) {
            init_IDs.emplace_back(zlp::fType::ID);
            init_values.emplace_back(zlp::fType::convertTo01(zldsp::filter::FilterType::kHighPass));
            init_IDs.emplace_back(zlp::slope::ID);
            init_values.emplace_back(zlp::slope::convertTo01(ui_base_.getDefaultPassFilterSlope()));
        } else if (freq < 50.f) {
            init_IDs.emplace_back(zlp::fType::ID);
            init_values.emplace_back(zlp::fType::convertTo01(zldsp::filter::FilterType::kLowShelf));
            init_IDs.emplace_back(zlp::gain::ID);
            init_values.emplace_back(zlp::gain::convertTo01(
                juce::jlimit(-maximum_db_.load(), maximum_db_.load(), 2 * db)
            ));
        } else if (freq < 5000.f) {
            init_IDs.emplace_back(zlp::fType::ID);
            init_values.emplace_back(zlp::fType::convertTo01(zldsp::filter::FilterType::kPeak));
            init_IDs.emplace_back(zlp::gain::ID);
            init_values.emplace_back(zlp::gain::convertTo01(
                juce::jlimit(-maximum_db_.load(), maximum_db_.load(), db)
            ));
        } else if (freq < 15000.f) {
            init_IDs.emplace_back(zlp::fType::ID);
            init_values.emplace_back(zlp::fType::convertTo01(zldsp::filter::FilterType::kHighShelf));
            init_IDs.emplace_back(zlp::gain::ID);
            init_values.emplace_back(zlp::gain::convertTo01(
                juce::jlimit(-maximum_db_.load(), maximum_db_.load(), 2 * db)
            ));
        } else {
            init_IDs.emplace_back(zlp::fType::ID);
            init_values.emplace_back(zlp::fType::convertTo01(zldsp::filter::FilterType::kLowPass));
            init_IDs.emplace_back(zlp::slope::ID);
            init_values.emplace_back(zlp::slope::convertTo01(ui_base_.getDefaultPassFilterSlope()));
        }
        init_IDs.emplace_back(zlp::freq::ID);
        init_values.emplace_back(zlp::freq::convertTo01(freq));
        init_IDs.emplace_back(zlp::Q::ID);
        init_values.emplace_back(zlp::Q::convertTo01(zlp::Q::defaultV));
        init_IDs.emplace_back(zlp::bypass::ID);
        init_values.emplace_back(zlp::bypass::convertTo01(false));
        // turn on dynamic is command is down
        if (event.mods.isCommandDown()) {
            init_IDs.emplace_back(zlp::dynamicON::ID);
            init_values.emplace_back(zlp::dynamicON::convertTo01(true));
            init_IDs.emplace_back(zlp::singleDynLink::ID);
            init_values.emplace_back(zlp::singleDynLink::convertTo01(ui_base_.getDynLink()));
        }

        for (size_t i = 0; i < init_IDs.size(); ++i) {
            const auto paraID = zlp::appendSuffix(init_IDs[i], idx);
            auto *para = parameters_ref_.getParameter(paraID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(init_values[i]);
            para->endChangeGesture();
        }

        if (event.mods.isCommandDown()) {
            processor_ref_.getFiltersAttach().turnOnDynamic(idx);
        }

        if (idx != band_idx_.load()) {
            auto *para = parameters_NA_ref_.getParameter(zlstate::selectedBandIdx::ID);
            para->beginChangeGesture();
            para->setValueNotifyingHost(zlstate::selectedBandIdx::convertTo01(static_cast<int>(idx)));
            para->endChangeGesture();
        } else {
            to_attach_group_.store(true);
        }
    }

    void ButtonPanel::parameterChanged(const juce::String &parameter_id, float new_value) {
        if (parameter_id == zlstate::selectedBandIdx::ID) {
            const auto idx = static_cast<size_t>(new_value);
            band_idx_.store(idx);
            to_attach_group_.store(true);
        } else if (parameter_id == zlstate::maximumDB::ID) {
            const auto idx = static_cast<size_t>(new_value);
            for (const auto &p: panels_) {
                p->setMaximumDB(zlstate::maximumDB::dBs[idx]);
            }
            maximum_db_.store(zlstate::maximumDB::dBs[idx]);
        } else {
            // the parameter is freq/gain/Q/targetGain/targetQ
            if (!is_during_lasso_.load()) return;
            const auto current_band = band_idx_.load();
            if (!ui_base_.getIsBandSelected(current_band)) return;
            const auto value = static_cast<double>(new_value);
            if (parameter_id.startsWith(zlp::freq::ID)) {
                const auto ratio = static_cast<float>(value / previous_freqs_[current_band].load());
                for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                    if (idx != current_band && ui_base_.getIsBandSelected(idx)) {
                        const auto shift_freq = previous_freqs_[idx].load() * ratio;
                        const auto legal_freq = zlp::freq::range.snapToLegalValue(shift_freq);
                        freq_updaters_[idx]->update(zlp::freq::range.convertTo0to1(legal_freq));
                    }
                }
            } else if (parameter_id.startsWith(zlp::gain::ID)) {
                if (is_left_click_.load()) {
                    if (std::abs(previous_gains_[current_band].load()) <= 0.1f) return;
                    const auto scale = new_value / previous_gains_[current_band].load();
                    for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                        if (idx != current_band && ui_base_.getIsBandSelected(idx)) {
                            const auto shift_gain = scale * previous_gains_[idx].load();
                            const auto legal_gain = juce::jlimit(-maximum_db_.load(), maximum_db_.load(), shift_gain);
                            gain_updaters_[idx]->update(zlp::gain::convertTo01(legal_gain));
                        }
                    }
                } else {
                    const auto shift = new_value - previous_gains_[current_band].load();
                    for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                        if (idx != current_band && ui_base_.getIsBandSelected(idx)) {
                            const auto shift_gain = shift + previous_gains_[idx].load();
                            const auto legal_gain = juce::jlimit(-maximum_db_.load(), maximum_db_.load(), shift_gain);
                            gain_updaters_[idx]->update(zlp::gain::convertTo01(legal_gain));
                        }
                    }
                }
            } else if (parameter_id.startsWith(zlp::Q::ID)) {
                const auto ratio = static_cast<float>(value / previous_qs_[current_band].load());
                for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                    if (idx != current_band && ui_base_.getIsBandSelected(idx)) {
                        const auto shift_q = ratio * previous_qs_[idx].load();
                        const auto legal_q = zlp::Q::range.snapToLegalValue(shift_q);
                        q_updaters_[idx]->update(zlp::Q::range.convertTo0to1(legal_q));
                    }
                }
            } else if (parameter_id.startsWith(zlp::targetGain::ID)) {
                if (is_left_click_.load()) {
                    if (std::abs(previous_target_gains_[current_band].load()) <= 0.1f) return;
                    const auto scale = new_value / previous_target_gains_[current_band].load();
                    for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                        if (idx != current_band && ui_base_.getIsBandSelected(idx)) {
                            const auto shift_gain = scale * previous_target_gains_[idx].load();
                            const auto legal_gain = juce::jlimit(-maximum_db_.load(), maximum_db_.load(), shift_gain);
                            target_gain_updaters_[idx]->update(zlp::targetGain::convertTo01(legal_gain));
                        }
                    }
                } else {
                    const auto shift = new_value - previous_target_gains_[current_band].load();
                    for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                        if (idx != current_band && ui_base_.getIsBandSelected(idx)) {
                            const auto shift_gain = shift + previous_target_gains_[idx].load();
                            const auto legal_gain = juce::jlimit(-maximum_db_.load(), maximum_db_.load(), shift_gain);
                            target_gain_updaters_[idx]->update(zlp::targetGain::convertTo01(legal_gain));
                        }
                    }
                }
            } else if (parameter_id.startsWith(zlp::targetQ::ID)) {
                const auto ratio = static_cast<float>(value / previous_target_qs_[current_band].load());
                for (size_t idx = 0; idx < zlstate::kBandNUM; ++idx) {
                    if (idx != current_band && ui_base_.getIsBandSelected(idx)) {
                        const auto shift_q = ratio * previous_target_qs_[idx].load();
                        const auto legal_q = zlp::Q::range.snapToLegalValue(shift_q);
                        target_q_updaters_[idx]->update(zlp::targetQ::range.convertTo0to1(legal_q));
                    }
                }
            }
        }
    }

    void ButtonPanel::attachGroup(const size_t idx) {
        loadPreviousParameters();
        for (size_t oldIdx = 0; oldIdx < zlstate::kBandNUM; ++oldIdx) {
            for (const auto &parameter: kIDs) {
                parameters_ref_.removeParameterListener(zlp::appendSuffix(parameter, oldIdx), this);
            }
        }
        for (const auto &parameter: kIDs) {
            parameters_ref_.addParameterListener(zlp::appendSuffix(parameter, idx), this);
        }
    }

    void ButtonPanel::updateAttach() {
        if (to_attach_group_.exchange(false)) {
            const auto idx = band_idx_.load();
            attachGroup(idx);
            for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
                panels_[i]->setSelected(i == idx);
            }
            panels_[idx]->toFront(false);
            wheel_attachment_[0].reset();
            wheel_attachment_[0] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                parameters_ref_, zlp::appendSuffix(zlp::Q::ID, band_idx_.load()), wheel_slider_[0]);
            wheel_attachment_[1].reset();
            wheel_attachment_[1] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                parameters_ref_, zlp::appendSuffix(zlp::targetQ::ID, band_idx_.load()), wheel_slider_[1]);
            wheel_attachment_[2].reset();
            wheel_attachment_[2] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                parameters_ref_, zlp::appendSuffix(zlp::sideQ::ID, band_idx_.load()), wheel_slider_[2]);
        }
    }

    size_t ButtonPanel::findAvailableBand() const {
        for (size_t i = 0; i < zlstate::kBandNUM; ++i) {
            const auto idx = zlstate::appendSuffix(zlstate::active::ID, i);
            const auto isActive = parameters_NA_ref_.getRawParameterValue(idx)->load() > .5f;
            if (!isActive) { return i; }
        }
        return zlstate::kBandNUM;
    }

    void ButtonPanel::findLassoItemsInArea(juce::Array<size_t> &items_found, const juce::Rectangle<int> &area) {
        juce::ignoreUnused(items_found, area);
        const auto float_area = area.toFloat();
        for (size_t idx = 0; idx < panels_.size(); ++idx) {
            if (panels_[idx]->isVisible()) {
                const auto transform = panels_[idx]->getDragger().getButton().getTransform();
                const auto dragger_position = panels_[idx]->getDragger().getPosition().toFloat();
                const auto b_center = dragger_position.transformedBy(transform);
                if (float_area.contains(b_center)) {
                    items_found.add(idx);
                }
            }
        }
    }

    juce::SelectedItemSet<size_t> &ButtonPanel::getLassoSelection() {
        return items_set_;
    }

    void ButtonPanel::changeListenerCallback(juce::ChangeBroadcaster *source) {
        juce::ignoreUnused(source);
        int current_selected_num = 0;
        size_t current_first_select_idx = 0;
        const auto c_band_idx = band_idx_.load();
        bool is_current_band_selected = false;
        for (size_t idx = 0; idx < panels_.size(); ++idx) {
            const auto f1 = items_set_.isSelected(idx);
            if (f1) {
                if (current_selected_num == 0) {
                    current_first_select_idx = idx;
                }
                if (idx == c_band_idx) {
                    is_current_band_selected = true;
                }
                current_selected_num += 1;
            }
            ui_base_.setIsBandSelected(idx, f1);
            const auto f2 = panels_[idx]->getDragger().getLAF().getIsSelected();
            if (f1 != f2) {
                panels_[idx]->getDragger().getLAF().setIsSelected(f1);
                panels_[idx]->getDragger().getButton().repaint();
            }
        }
        if (current_selected_num > 0) {
            if (previous_lasso_num_ == 0 || !is_current_band_selected) {
                panels_[current_first_select_idx]->setSelected(true);
            }
            previous_lasso_num_ = current_selected_num;
            loadPreviousParameters();
            is_during_lasso_.store(true);
        } else {
            is_during_lasso_.store(false);
        }
    }

    void ButtonPanel::loadPreviousParameters() {
        for (size_t idx = 0; idx < panels_.size(); ++idx) {
            previous_freqs_[idx].store(zlp::freq::range.convertFrom0to1(
                freq_updaters_[idx]->getPara()->getValue()));
            previous_gains_[idx].store(zlp::gain::range.convertFrom0to1(
                gain_updaters_[idx]->getPara()->getValue()));
            previous_qs_[idx].store(zlp::Q::range.convertFrom0to1(
                q_updaters_[idx]->getPara()->getValue()));
            previous_target_gains_[idx].store(zlp::targetGain::range.convertFrom0to1(
                target_gain_updaters_[idx]->getPara()->getValue()));
            previous_target_qs_[idx].store(zlp::targetQ::range.convertFrom0to1(
                target_q_updaters_[idx]->getPara()->getValue()));
        }
    }
} // zlpanel
