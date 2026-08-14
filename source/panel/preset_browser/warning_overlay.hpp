// Copyright (C) 2026 - zsliu98
// This file is part of ZLSpectrumEqualizer
//
// ZLSpectrumEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSpectrumEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSpectrumEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../gui/button/click_text_button/click_text_button.hpp"
#include "../../gui/interface_definitions.hpp"
#include "../../gui/label/name_look_and_feel.hpp"
#include "../background/panel_background.hpp"

namespace zlpanel {
    class WarningOverlay final : public juce::Component {
    public:
        explicit WarningOverlay(zlgui::UIBase& base);

        ~WarningOverlay() override;

        void paint(juce::Graphics& g) override;

        void resized() override;

        bool keyPressed(const juce::KeyPress& key) override;

        void show(const juce::String& message, const juce::String& action, bool destructive,
                  std::function<void()> on_confirm);

        void showMessage(const juce::String& message);

        void hide();

    private:
        zlgui::UIBase& base_;
        PanelBackground background_;
        zlgui::label::NameLookAndFeel message_laf_;
        juce::Label message_label_;
        zlgui::button::ClickTextButton cancel_button_;
        zlgui::button::ClickTextButton confirm_button_;
        std::function<void()> on_confirm_;
        bool destructive_{false};

        juce::Rectangle<int> getCardBounds() const;

        juce::Rectangle<int> getCancelBounds() const;

        juce::Rectangle<int> getConfirmBounds() const;

        juce::Rectangle<int> getMessageBounds() const;

        void confirm();
    };
}
