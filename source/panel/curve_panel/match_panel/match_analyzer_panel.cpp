// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_analyzer_panel.hpp"

namespace zlpanel {
    MatchAnalyzerPanel::MatchAnalyzerPanel(zldsp::eq_match::EqMatchAnalyzer<double> &analyzer,
                                           juce::AudioProcessorValueTreeState &parameters_NA,
                                           zlgui::UIBase &base)
        : analyzer_ref_(analyzer), parameters_NA_ref_(parameters_NA), ui_base_(base),
          low_dragger_(base), high_dragger_(base), shift_dragger_(base),
          match_label_(base) {
        parameters_NA_ref_.addParameterListener(zlstate::maximumDB::ID, this);
        parameterChanged(zlstate::maximumDB::ID, parameters_NA_ref_.getRawParameterValue(zlstate::maximumDB::ID)->load());
        setInterceptsMouseClicks(true, true);
        ui_base_.getValueTree().addListener(this); {
            low_dragger_.getLAF().setDraggerShape(zlgui::DraggerLookAndFeel::DraggerShape::rightArrow);
            low_dragger_.setYPortion(.5f);
            low_dragger_.setXYEnabled(true, false);
        } {
            high_dragger_.getLAF().setDraggerShape(zlgui::DraggerLookAndFeel::DraggerShape::leftArrow);
            high_dragger_.setYPortion(.5f);
            high_dragger_.setXYEnabled(true, false);
        } {
            shift_dragger_.getLAF().setDraggerShape(zlgui::DraggerLookAndFeel::DraggerShape::upDownArrow);
            shift_dragger_.setXPortion(0.508304953195832f);
            shift_dragger_.setXYEnabled(false, true);
        }
        for (auto &d: {&low_dragger_, &high_dragger_, &shift_dragger_}) {
            d->setScale(kScale);
            d->getButton().setToggleState(true, juce::dontSendNotification);
            d->getLAF().setIsSelected(true);
            d->setInterceptsMouseClicks(false, true);
            d->addListener(this);
            addAndMakeVisible(d);
        }
        addChildComponent(match_label_);
        lookAndFeelChanged();
        visibilityChanged();
    }

    MatchAnalyzerPanel::~MatchAnalyzerPanel() {
        ui_base_.getValueTree().removeListener(this);
        parameters_NA_ref_.removeParameterListener(zlstate::maximumDB::ID, this);
    }

    void MatchAnalyzerPanel::paint(juce::Graphics &g) {
        if (std::abs(c_maximum_db_ - maximum_db_.load()) >= 1e-3f) {
            c_maximum_db_ = maximum_db_.load();
            const auto actualShift = static_cast<float>(ui_base_.getProperty(zlgui::SettingIdx::kMatchShift)) * .5f;
            shift_dragger_.setYPortion(actualShift / c_maximum_db_ + .5f);
        }
        g.fillAll(ui_base_.getColourByIdx(zlgui::kBackgroundColour).withAlpha(background_alpha_));
        const auto thickness = ui_base_.getFontSize() * 0.2f * ui_base_.getSumCurveThickness();
        juce::GenericScopedTryLock lock{path_lock_};
        if (!lock.isLocked()) { return; }
        if (show_average_) {
            g.setColour(ui_base_.getColourByIdx(zlgui::kSideColour).withAlpha(.5f));
            g.strokePath(recent_path2_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(ui_base_.getColourByIdx(zlgui::kPreColour).withAlpha(.5f));
            g.strokePath(recent_path1_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour(ui_base_.getColorMap2(2));
        g.strokePath(recent_path3_,
                     juce::PathStrokeType(thickness * 1.5f,
                                          juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        if (const auto x_p = low_dragger_.getXPortion(); x_p > 0.001f) {
            auto bound = getLocalBounds().toFloat();
            bound = bound.removeFromLeft(bound.getWidth() * x_p);
            g.setColour(ui_base_.getBackgroundColor().withAlpha(.75f));
            g.fillRect(bound);
        }
        if (const auto y_p = high_dragger_.getXPortion(); y_p < .999f) {
            auto bound = getLocalBounds().toFloat();
            bound = bound.removeFromRight(bound.getWidth() * (1.f - y_p));
            g.setColour(ui_base_.getBackgroundColor().withAlpha(.75f));
            g.fillRect(bound);
        }
    }

    void MatchAnalyzerPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        left_corner_.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        right_corner_.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomic_bound_.store(bound);
        db_scale_.store((1.f + ui_base_.getFontSize() * 2.f / bound.getHeight()) * 2.f);
        match_label_.setBounds(getLocalBounds());
        low_dragger_.setBounds(getLocalBounds());
        low_dragger_.setButtonArea(bound);
        high_dragger_.setBounds(getLocalBounds());
        high_dragger_.setButtonArea(bound);
        shift_dragger_.setBounds(getLocalBounds());
        bound.removeFromTop(ui_base_.getFontSize());
        bound.removeFromBottom(ui_base_.getFontSize());
        shift_dragger_.setButtonArea(bound);
    }

    void MatchAnalyzerPanel::updatePaths() {
        analyzer_ref_.updatePaths(path1_, path2_, path3_,
                                atomic_bound_.load(),
                                {-72.f, -72.f, -maximum_db_.load() * db_scale_.load()}); {
            juce::GenericScopedLock lock{path_lock_};
            recent_path1_ = path1_;
            recent_path2_ = path2_;
            recent_path3_ = path3_;
        }
        analyzer_ref_.checkRun();
    }

    void MatchAnalyzerPanel::visibilityChanged() {
        if (!isVisible()) {
            low_dragger_.setXPortion(0.f);
            high_dragger_.setXPortion(1.f);
            shift_dragger_.setYPortion(.5f);
            draggerValueChanged(&low_dragger_);
            draggerValueChanged(&high_dragger_);
            draggerValueChanged(&shift_dragger_);
            analyzer_ref_.clearDrawingDiffs();
            updateDraggers();
        }
    }

    void MatchAnalyzerPanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) {
        if (zlgui::UIBase::isProperty(zlgui::SettingIdx::kMatchPanelFit, property)) {
            const auto f = static_cast<bool>(ui_base_.getProperty(zlgui::SettingIdx::kMatchPanelFit));
            background_alpha_ = f ? .2f : .5f;
            show_average_ = !f;
        } else if (zlgui::UIBase::isProperty(zlgui::SettingIdx::kMatchFitRunning, property)) {
            match_label_.setVisible(static_cast<bool>(ui_base_.getProperty(zlgui::SettingIdx::kMatchFitRunning)));
        }
    }

    void MatchAnalyzerPanel::parameterChanged(const juce::String &parameter_id, const float new_value) {
        juce::ignoreUnused(parameter_id);
        maximum_db_.store(zlstate::maximumDB::dBs[static_cast<size_t>(new_value)]);
    }

    void MatchAnalyzerPanel::lookAndFeelChanged() {
        for (auto &d: {&low_dragger_, &high_dragger_, &shift_dragger_}) {
            d->getLAF().setColour(ui_base_.getTextColor());
        }
    }

    void MatchAnalyzerPanel::draggerValueChanged(zlgui::Dragger *dragger) {
        if (dragger == &low_dragger_) {
            ui_base_.setProperty(zlgui::SettingIdx::kMatchLowCut, low_dragger_.getXPortion());
        } else if (dragger == &high_dragger_) {
            ui_base_.setProperty(zlgui::SettingIdx::kMatchHighCut, high_dragger_.getXPortion());
        } else if (dragger == &shift_dragger_) {
            const auto actualShift = (shift_dragger_.getYPortion() - .5f) * maximum_db_.load() * 2.f;
            ui_base_.setProperty(zlgui::SettingIdx::kMatchShift, actualShift);
            analyzer_ref_.setShift(actualShift);
        }
    }

    void MatchAnalyzerPanel::updateDraggers() {
        for (auto &d: {&low_dragger_, &high_dragger_, &shift_dragger_}) {
            d->updateButton();
        }
    }

    void MatchAnalyzerPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.mods.isCommandDown()) {
            getIdxDBromPoint(event.getPosition(), pre_draw_idx_, pre_draw_db_);
            if (event.mods.isRightButtonDown()) {
                pre_draw_db_ = 0.f;
            }
        }
    }

    void MatchAnalyzerPanel::mouseDrag(const juce::MouseEvent &event) {
        // draw if command down
        if (event.mods.isCommandDown()) {
            size_t currentDrawIdx;
            float currentDrawDB;
            getIdxDBromPoint(event.getPosition(), currentDrawIdx, currentDrawDB);
            // if right button down, clear draws
            if (event.mods.isRightButtonDown()) {
                if (currentDrawIdx == pre_draw_idx_) {
                    analyzer_ref_.clearDrawingDiffs(currentDrawIdx);
                } else if (currentDrawIdx < pre_draw_idx_) {
                    for (size_t idx = currentDrawIdx; idx < pre_draw_idx_; ++idx) {
                        analyzer_ref_.clearDrawingDiffs(idx);
                    }
                } else {
                    for (size_t idx = pre_draw_idx_ + 1; idx <= currentDrawIdx; ++idx) {
                        analyzer_ref_.clearDrawingDiffs(idx);
                    }
                }
            } else {
                // if shift down, draw as zeros
                if (event.mods.isShiftDown()) {
                    currentDrawDB = 0.f;
                }
                if (currentDrawIdx == pre_draw_idx_) {
                    analyzer_ref_.setDrawingDiffs(currentDrawIdx, currentDrawDB);
                } else if (currentDrawIdx < pre_draw_idx_) {
                    float dB = currentDrawDB;
                    const float deltaDB = (pre_draw_db_ - currentDrawDB) / static_cast<float>(pre_draw_idx_ - currentDrawIdx);
                    for (size_t idx = currentDrawIdx; idx < pre_draw_idx_; ++idx) {
                        analyzer_ref_.setDrawingDiffs(idx, dB);
                        dB += deltaDB;
                    }
                } else {
                    float dB = pre_draw_db_;
                    const float deltaDB = (currentDrawDB - pre_draw_db_) / static_cast<float>(currentDrawIdx - pre_draw_idx_);
                    for (size_t idx = pre_draw_idx_ + 1; idx <= currentDrawIdx; ++idx) {
                        analyzer_ref_.setDrawingDiffs(idx, dB);
                        dB += deltaDB;
                    }
                }
            }
            // store current idx and DB
            pre_draw_idx_ = currentDrawIdx;
            pre_draw_db_ = currentDrawDB;
        }
    }

    void MatchAnalyzerPanel::mouseDoubleClick(const juce::MouseEvent &event) {
        if (!event.mods.isCommandDown() && event.mods.isShiftDown()) {
            analyzer_ref_.clearDrawingDiffs();
        }
    }
} // zlpanel
