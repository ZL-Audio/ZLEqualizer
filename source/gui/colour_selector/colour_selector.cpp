// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "colour_selector.hpp"

namespace zlgui::colour_selector {
    class SelectorBox final : public juce::Component {
    public:
        explicit SelectorBox(const int selectorFlags, zlgui::UIBase& base)
            : selector_(selectorFlags,
                        juce::roundToInt(base.getFontSize() * 0.5f),
                        juce::roundToInt(base.getFontSize() * 0.33f)),
              padding_(base.getFontSize() * .5f) {
            selector_.setColour(juce::ColourSelector::ColourIds::backgroundColourId, base.getBackgroundColour());
            addAndMakeVisible(selector_);
        }

        ~SelectorBox() override {
            setLookAndFeel(nullptr);
        }

        void resized() override {
            auto bound = getLocalBounds().toFloat();
            bound = bound.withSizeKeepingCentre(
                bound.getWidth() - padding_,
                bound.getHeight() - padding_);
            selector_.setBounds(bound.toNearestInt());
        }

        juce::ColourSelector& getSelector() { return selector_; }

    private:
        juce::ColourSelector selector_;
        const float padding_;
    };

    ColourSelector::ColourSelector(zlgui::UIBase& base, juce::Component& parent,
                                   const float width_s, const float height_s)
        : base_(base), laf_(base_), parent_ref_(parent),
          selector_width_s_(width_s), selector_height_s_(height_s) {
    }

    ColourSelector::~ColourSelector() {
        closePopup();
    }

    void ColourSelector::closePopup() {
        if (active_selector_ != nullptr) {
            active_selector_->removeChangeListener(this);
            active_selector_ = nullptr;
        }
        if (auto* box = callout_box_.getComponent()) {
            box->exitModalState(0);
            box->setVisible(false);
            if (auto* parent = box->getParentComponent()) {
                parent->removeChildComponent(box);
            }
            box->setLookAndFeel(nullptr);
            callout_box_ = nullptr;
        }
    }

    void ColourSelector::paint(juce::Graphics& g) {
        g.fillAll(base_.getTextColour().withAlpha(.875f));
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - base_.getFontSize() * .375f,
                                            bound.getHeight() - base_.getFontSize() * .375f);
        g.setColour(base_.getBackgroundColour());
        g.fillRect(bound);
        g.setColour(colour_);
        g.fillRect(bound);
    }

    void ColourSelector::mouseDown(const juce::MouseEvent& event) {
        juce::ignoreUnused(event);
        closePopup();
        auto colour_selector = std::make_unique<SelectorBox>(
            juce::ColourSelector::ColourSelectorOptions::showColourspace, base_);
        colour_selector->getSelector().setCurrentColour(colour_);
        colour_selector->getSelector().addChangeListener(this);
        active_selector_ = &colour_selector->getSelector();
        colour_selector->setSize(juce::roundToInt(selector_width_s_ * base_.getFontSize()),
                                 juce::roundToInt(selector_height_s_ * base_.getFontSize()));
        auto& box = juce::CallOutBox::launchAsynchronously(std::move(colour_selector),
                                                           parent_ref_.getLocalArea(this, getLocalBounds()),
                                                           &parent_ref_);
        callout_box_ = &box;
        box.setLookAndFeel(&laf_);
        box.setArrowSize(0);
        box.sendLookAndFeelChange();
    }

    void ColourSelector::changeListenerCallback(juce::ChangeBroadcaster* source) {
        if (const auto* cs = dynamic_cast<juce::ColourSelector*>(source)) {
            colour_ = cs->getCurrentColour().withAlpha(colour_.getAlpha());
            repaint();
        }
    }
} // zlgui
