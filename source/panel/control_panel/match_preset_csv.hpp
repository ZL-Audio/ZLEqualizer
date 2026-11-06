// Copyright (C) 2026 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <istream>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace zlpanel::match_preset_csv {
    /**
     * parse and validate a match preset without changing the outputs on failure
     * @param input the CSV input stream
     * @param frequencies the output frequencies
     * @param dbs the output dB values
     * @return true on success, false on failure
     */
    inline bool parse(std::istream& input, std::vector<float>& frequencies,
                      std::vector<float>& dbs) {
        const auto parse_number = [](const std::string_view text, float& value) {
            std::istringstream field{std::string(text)};
            field.imbue(std::locale::classic());
            if (!(field >> value) || !std::isfinite(value)) {
                return false;
            }
            field >> std::ws;
            return field.eof();
        };

        std::vector<float> parsed_frequencies, parsed_dbs;
        std::string line;
        while (std::getline(input, line)) {
            // require exactly two columns per row
            const auto comma = line.find(',');
            if (comma == std::string::npos || line.find(',', comma + 1) != std::string::npos) {
                return false;
            }

            // require finite numbers with no trailing text
            float frequency{}, db{};
            const std::string_view row{line};
            if (!parse_number(row.substr(0, comma), frequency) ||
                !parse_number(row.substr(comma + 1), db)) {
                return false;
            }
            // frequencies must be strictly increasing
            if (!parsed_frequencies.empty() && frequency <= parsed_frequencies.back()) {
                return false;
            }
            parsed_frequencies.push_back(frequency);
            parsed_dbs.push_back(db);
        }
        // require complete input with at least two rows
        if (input.bad() || parsed_frequencies.size() < 2) {
            return false;
        }

        frequencies.swap(parsed_frequencies);
        dbs.swap(parsed_dbs);
        return true;
    }
}
