// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "tooltip_labels.hpp"
#include "translations/en.hpp"
#include "translations/zh_Hans.hpp"
#include "translations/zh_Hant.hpp"
#include "translations/it.hpp"
#include "translations/ja.hpp"
#include "translations/de.hpp"
#include "translations/es.hpp"

namespace zlpanel::multilingual {
    enum TooltipLanguage {
        kOff,
        kLang_system,
        kLang_en,
        kLang_zh_Hans,
        kLang_zh_Hant,
        kLang_it,
        kLang_ja,
        kLang_de,
        kLang_es,
        kLangNum
    };

    class TooltipHelper {
    public:
        explicit TooltipHelper(TooltipLanguage language);

        std::string getToolTipText(TooltipLabel label) const;

    private:
        TooltipLanguage language_;
    };
} // zlpanel
