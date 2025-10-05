// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#include "tooltip_helper.hpp"

namespace zlpanel::multilingual {
    TooltipHelper::TooltipHelper(const TooltipLanguage language) {
        if (language == kLang_system) {
            const auto displayed_lang = juce::SystemStats::getDisplayLanguage();
            if (displayed_lang == "zh" || displayed_lang == "chi" || displayed_lang == "zho"
                || displayed_lang.startsWith("zh-")) {
                if (displayed_lang.startsWith("zh-Hant") || displayed_lang.startsWith("zh-hant")) {
                    language_ = kLang_zh_Hant;
                } else {
                    language_ = kLang_zh_Hans;
                }
            } else if (displayed_lang == "it" || displayed_lang == "ita" ||
                       displayed_lang.startsWith("it-") || displayed_lang.startsWith("ita-")) {
                language_ = kLang_it;
            } else if (displayed_lang == "ja" || displayed_lang == "jpn" ||
                       displayed_lang.startsWith("ja-") || displayed_lang.startsWith("jpn-")) {
                language_ = kLang_ja;
            } else if (displayed_lang == "de" || displayed_lang == "deu" || displayed_lang == "ger" ||
                       displayed_lang.startsWith("de-") || displayed_lang.startsWith("deu-") ||
                       displayed_lang.startsWith("ger-")) {
                language_ = kLang_de;
            } else if (displayed_lang == "es" || displayed_lang == "spa" ||
                       displayed_lang.startsWith("es-") || displayed_lang.startsWith("spa-")) {
                language_ = kLang_es;
            } else {
                language_ = kLang_en;
            }
        } else {
            language_ = language;
        }
    }

    std::string TooltipHelper::getToolTipText(const TooltipLabel label) const {
        switch (language_) {
            case kOff: {
                return std::string();
            }
            case kLang_zh_Hans: {
                return zh_Hans::kTexts[static_cast<size_t>(label)];
            }
            case kLang_zh_Hant: {
                return zh_Hant::kTexts[static_cast<size_t>(label)];
            }
            case kLang_it: {
                return it::kTexts[static_cast<size_t>(label)];
            }
            case kLang_ja: {
                return ja::kTexts[static_cast<size_t>(label)];
            }
            case kLang_de: {
                return de::kTexts[static_cast<size_t>(label)];
            }
            case kLang_es: {
                return es::kTexts[static_cast<size_t>(label)];
            }
            case kLang_en:
            case kLang_system:
            case kLangNum:
            default: {
                return en::kTexts[static_cast<size_t>(label)];
            }
        }
    }
} // zlpanel
