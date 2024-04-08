// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef INTERNAL_SETTING_PANEL_HPP
#define INTERNAL_SETTING_PANEL_HPP

#include "../../gui/gui.hpp"

namespace zlPanel {

class InternalSettingPanel final : public juce::Component  {
public:
    void loadSetting();

    void saveSetting();

    void resetSetting();
};

} // zlPanel

#endif //INTERNAL_SETTING_PANEL_HPP
