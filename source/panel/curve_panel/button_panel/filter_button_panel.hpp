// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <limits>

#include "../../../dsp/dsp.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../../PluginProcessor.hpp"

#include "button_pop_up.hpp"

namespace zlpanel {
    class FilterButtonPanel final : public juce::Component,
                                    private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit FilterButtonPanel(size_t bandIdx, PluginProcessor &processor, zlgui::UIBase &base);

        ~FilterButtonPanel() override;

        void resized() override;

        zlgui::Dragger &getDragger() { return dragger_; }

        zlgui::Dragger &getTargetDragger() { return target_dragger_; }

        zlgui::Dragger &getSideDragger() { return side_dragger_; }

        ButtonPopUp &getPopUp() { return button_pop_up_; }

        bool getSelected() { return dragger_.getButton().getToggleState(); }

        void setSelected(bool f);

        void setMaximumDB(float db);

        void updateDraggers() {
            if (to_update_draggers_.exchange(false)) {
                handleAsyncUpdate();
            }
        }

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void lookAndFeelChanged() override;

        void visibilityChanged() override;

    private:
        PluginProcessor &processor_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_, &parameters_NA_ref_;
        zlgui::UIBase &ui_base_;
        zlgui::Dragger dragger_, target_dragger_, side_dragger_;
        ButtonPopUp button_pop_up_;
        std::unique_ptr<zlgui::DraggerParameterAttach> base_attach_, target_attach_, side_attach_;
        std::atomic<float> maximum_db_{zlstate::maximumDB::dBs[static_cast<size_t>(zlstate::maximumDB::defaultI)]};
        std::atomic<zldsp::filter::FilterType> f_type_;
        std::atomic<zlp::lrType::lrTypes> lr_type_;
        const size_t band_idx_;
        std::atomic<float> &c_band_idx_;
        std::atomic<bool> is_dynamic_has_target_{false}, is_selected_target_{false}, is_active_target_{false};

        static constexpr std::array kIDs{zlp::fType::ID, zlp::lrType::ID, zlp::dynamicON::ID};
        static constexpr std::array kNAIDs{zlstate::active::ID};
        static constexpr auto kScale = 1.5f;

        std::atomic<bool> to_update_draggers_{false};
        std::atomic<bool> to_update_attachment_{false}, to_update_bounds_{false};
        std::atomic<bool> to_update_target_attachment_{false}, to_update_dragger_label_{false};

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void handleAsyncUpdate();

        void updateAttachment();

        void updateBounds();

        void updateTargetAttachment();

        void updateDraggerLabel();

        const juce::NormalisableRange<float> kFreqRange{
            10.f, 20000.f,
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return std::exp(valueToRemap * std::log(
                                    rangeEnd / rangeStart)) * rangeStart;
            },
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return std::log(valueToRemap / rangeStart) / std::log(
                           rangeEnd / rangeStart);
            },
            [](float rangeStart, float rangeEnd, float valueToRemap) {
                return juce::jlimit(
                    rangeStart, rangeEnd, valueToRemap);
            }
        };
    };
} // zlpanel
