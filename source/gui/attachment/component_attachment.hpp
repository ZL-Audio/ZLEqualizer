// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace zlgui::attachment {
    class ComponentAttachment {
    public:
        virtual ~ComponentAttachment() = default;

        virtual void updateComponent() = 0;

    protected:
        class ParameterUpdateGuard {
        public:
            explicit ParameterUpdateGuard(bool& flag) noexcept
                : flag_(flag), previous_value_(flag) {
                flag_ = true;
            }

            ~ParameterUpdateGuard() {
                flag_ = previous_value_;
            }

            ParameterUpdateGuard(const ParameterUpdateGuard&) = delete;
            ParameterUpdateGuard& operator=(const ParameterUpdateGuard&) = delete;
            ParameterUpdateGuard(ParameterUpdateGuard&&) = delete;
            ParameterUpdateGuard& operator=(ParameterUpdateGuard&&) = delete;

        private:
            bool& flag_;
            bool previous_value_;
        };

        [[nodiscard]] ParameterUpdateGuard beginParameterUpdate() noexcept {
            return ParameterUpdateGuard{updating_from_parameter_};
        }

        bool isUpdatingFromParameter() const noexcept {
            return updating_from_parameter_;
        }

    private:
        bool updating_from_parameter_{false};
    };
}
