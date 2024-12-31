// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "match_analyzer_panel.hpp"

namespace zlPanel {
    MatchAnalyzerPanel::MatchAnalyzerPanel(zlEqMatch::EqMatchAnalyzer<double> &analyzer,
                                           juce::AudioProcessorValueTreeState &parametersNA,
                                           zlInterface::UIBase &base)
        : analyzerRef(analyzer), parametersNARef(parametersNA), uiBase(base),
          lowDragger(base), highDragger(base), shiftDragger(base),
          labelLAF(uiBase) {
        parametersNARef.addParameterListener(zlState::maximumDB::ID, this);
        parameterChanged(zlState::maximumDB::ID, parametersNARef.getRawParameterValue(zlState::maximumDB::ID)->load());
        setInterceptsMouseClicks(false, true);
        uiBase.getValueTree().addListener(this);
        runningLabel.setText("Running", juce::dontSendNotification);
        runningLabel.setJustificationType(juce::Justification::centred);
        labelLAF.setFontScale(5.f);
        runningLabel.setLookAndFeel(&labelLAF);
        addChildComponent(runningLabel); {
            lowDragger.getLAF().setDraggerShape(zlInterface::DraggerLookAndFeel::DraggerShape::rightArrow);
            lowDragger.setYPortion(.5f);
            lowDragger.setXYEnabled(true, false);
        } {
            highDragger.getLAF().setDraggerShape(zlInterface::DraggerLookAndFeel::DraggerShape::leftArrow);
            highDragger.setYPortion(.5f);
            highDragger.setXYEnabled(true, false);
        } {
            shiftDragger.getLAF().setDraggerShape(zlInterface::DraggerLookAndFeel::DraggerShape::upDownArrow);
            shiftDragger.setXPortion(0.508304953195832f);
            shiftDragger.setXYEnabled(false, true);
        }
        for (auto &d: {&lowDragger, &highDragger, &shiftDragger}) {
            d->setScale(scale);
            d->setActive(true);
            d->getButton().setToggleState(true, juce::dontSendNotification);
            d->getLAF().setIsSelected(true);
            d->setInterceptsMouseClicks(false, true);
            d->addListener(this);
            addAndMakeVisible(d);
        }
        lookAndFeelChanged();
    }

    MatchAnalyzerPanel::~MatchAnalyzerPanel() {
        uiBase.getValueTree().removeListener(this);
        parametersNARef.removeParameterListener(zlState::maximumDB::ID, this);
    }

    void MatchAnalyzerPanel::paint(juce::Graphics &g) {
        if (std::abs(currentMaximumDB - maximumDB.load()) >= 1e-3f) {
            currentMaximumDB = maximumDB.load();
            const auto actualShift = static_cast<float>(uiBase.getProperty(zlInterface::settingIdx::matchShift)) * .5f;
            shiftDragger.setYPortion(actualShift / currentMaximumDB + .5f);
        }
        g.fillAll(uiBase.getColourByIdx(zlInterface::backgroundColour).withAlpha(backgroundAlpha));
        const auto thickness = uiBase.getFontSize() * 0.2f * uiBase.getSumCurveThickness();
        juce::GenericScopedTryLock lock{pathLock};
        if (!lock.isLocked()) { return; }
        if (showAverage) {
            g.setColour(uiBase.getColourByIdx(zlInterface::sideColour).withAlpha(.5f));
            g.strokePath(recentPath2,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(uiBase.getColourByIdx(zlInterface::preColour).withAlpha(.5f));
            g.strokePath(recentPath1,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour(uiBase.getColorMap2(2));
        g.strokePath(recentPath3,
                     juce::PathStrokeType(thickness * 1.5f,
                                          juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        if (const auto xP = lowDragger.getXPortion(); xP > 0.001f) {
            auto bound = getLocalBounds().toFloat();
            bound = bound.removeFromLeft(bound.getWidth() * xP);
            g.setColour(uiBase.getBackgroundColor().withAlpha(.75f));
            g.fillRect(bound);
        }
        if (const auto yP = highDragger.getXPortion(); yP < .999f) {
            auto bound = getLocalBounds().toFloat();
            bound = bound.removeFromRight(bound.getWidth() * (1.f - yP));
            g.setColour(uiBase.getBackgroundColor().withAlpha(.75f));
            g.fillRect(bound);
        }
    }

    void MatchAnalyzerPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        leftCorner.store({bound.getX() * 0.9f, bound.getBottom() * 1.1f});
        rightCorner.store({bound.getRight() * 1.1f, bound.getBottom() * 1.1f});
        atomicBound.store(bound);
        dBScale.store((1.f + uiBase.getFontSize() * 2.f / bound.getHeight()) * 2.f);
        runningLabel.setBounds(bound.withSizeKeepingCentre(
            bound.getWidth() * .5f, uiBase.getFontSize() * 5.f).toNearestInt());
        lowDragger.setBounds(getLocalBounds());
        highDragger.setBounds(getLocalBounds());
        shiftDragger.setBounds(getLocalBounds());
        shiftDragger.setPadding(0.f, 0.f, uiBase.getFontSize(), uiBase.getFontSize());
    }

    void MatchAnalyzerPanel::updatePaths() {
        analyzerRef.updatePaths(path1, path2, path3,
                                atomicBound.load(),
                                {-72.f, -72.f, -maximumDB.load() * dBScale.load()}); {
            juce::GenericScopedLock lock{pathLock};
            recentPath1 = path1;
            recentPath2 = path2;
            recentPath3 = path3;
        }
        analyzerRef.checkRun();
    }

    void MatchAnalyzerPanel::visibilityChanged() {
        lowDragger.setXPortion(0.f);
        highDragger.setXPortion(1.f);
        shiftDragger.setYPortion(.5f);
        draggerValueChanged(&lowDragger);
        draggerValueChanged(&highDragger);
        draggerValueChanged(&shiftDragger);
    }

    void MatchAnalyzerPanel::valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) {
        if (zlInterface::UIBase::isProperty(zlInterface::settingIdx::matchPanelFit, property)) {
            const auto f = static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchPanelFit));
            backgroundAlpha = f ? .2f : .5f;
            showAverage = !f;
        } else if (zlInterface::UIBase::isProperty(zlInterface::settingIdx::matchFitRunning, property)) {
            runningLabel.setVisible(static_cast<bool>(uiBase.getProperty(zlInterface::settingIdx::matchFitRunning)));
        }
    }

    void MatchAnalyzerPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID);
        maximumDB.store(zlState::maximumDB::dBs[static_cast<size_t>(newValue)]);
    }

    void MatchAnalyzerPanel::lookAndFeelChanged() {
        for (auto &d: {&lowDragger, &highDragger, &shiftDragger}) {
            d->getLAF().setColour(uiBase.getTextColor());
        }
    }

    void MatchAnalyzerPanel::draggerValueChanged(zlInterface::Dragger *dragger) {
        if (dragger == &lowDragger) {
            uiBase.setProperty(zlInterface::settingIdx::matchLowCut, lowDragger.getXPortion());
        } else if (dragger == &highDragger) {
            uiBase.setProperty(zlInterface::settingIdx::matchHighCut, highDragger.getXPortion());
        } else if (dragger == &shiftDragger) {
            const auto actualShift = (shiftDragger.getYPortion() - .5f) * maximumDB.load() * 2.f;
            uiBase.setProperty(zlInterface::settingIdx::matchShift, actualShift);
            analyzerRef.setShift(actualShift);
        }
    }

    void MatchAnalyzerPanel::updateDraggers() {
        for (auto &d: {&lowDragger, &highDragger, &shiftDragger}) {
            d->updateButton();
        }
    }
} // zlPanel
