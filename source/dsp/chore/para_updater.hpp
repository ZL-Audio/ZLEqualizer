// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLCHORE_PARA_UPDATER_HPP
#define ZLCHORE_PARA_UPDATER_HPP

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlChore {
    class ParaUpdater final : private juce::AsyncUpdater {
    public:
        explicit ParaUpdater(const juce::AudioProcessorValueTreeState &parameter,
                             const std::string &parameterIdx) {
            para = parameter.getParameter(parameterIdx);
        }

        void update(const float paraValue) {
            value.store(paraValue);
            triggerAsyncUpdate();
        }

        void updateSync(const float paraValue) {
            para->beginChangeGesture();
            para->setValueNotifyingHost(paraValue);
            para->endChangeGesture();
        }

        juce::RangedAudioParameter *getPara() const { return para; }

    private:
        juce::RangedAudioParameter *para;
        std::atomic<float> value{};

        void handleAsyncUpdate() override {
            para->beginChangeGesture();
            para->setValueNotifyingHost(value.load());
            para->endChangeGesture();
        }
    };
}

#endif //ZLCHORE_PARA_UPDATER_HPP
