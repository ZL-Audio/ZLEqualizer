// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

// This file is also dual licensed under the Apache License, Version 2.0. You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>

#pragma once

#include <array>

namespace zlpanel::multilingual::zh_Hant {
    static constexpr std::array kTexts = {
        "釋放：旁通此頻段。",
        "按下：監聽此頻段。",
        "選擇濾波器類型。",
        "選擇濾波器斜率。較高的斜率會使濾波器的響應曲線變化更陡峭。",
        "選擇立體聲模式。",
        "控制頻率。",
        "控制基礎增益與目標增益。",
        "控制品質因數 (Q 值)。Q 值越大，頻寬越窄。",
        "按下：開啟動態行為。",
        "點擊：關閉此頻段。",
        "釋放：旁通動態行為。",
        "按下：開啟動態學習行為。\n釋放：關閉動態學習行為，並設定閾值 (Threshold) 與拐點 (Knee)。\n詳情參見手冊。",
        "按下：開啟動態相對行為。\n詳情參見手冊。",
        "按下：變更側鏈立體聲模式。",
        "控制動態行為的閾值。\n詳情參見手冊。",
        "控制動態行為的拐點寬度。\n詳情參見手冊。",
        "控制動態行為的攻擊時間。\n詳情參見手冊。",
        "控制動態行為的釋放時間。\n詳情參見手冊。",
        "按下：將此頻段與側鏈頻段關聯。",
        "選擇側鏈濾波器類型。",
        "選擇側鏈濾波器斜率。",
        "控制側鏈濾波器頻率。",
        "控制側鏈濾波器 Q 值。",

        "釋放：旁通插件。",
        "按下：使用外部側鏈。\n釋放：使用內部側鏈。",

        "控制額外的輸出增益。",
        "控制所有濾波器基礎增益和目標增益的縮放比例。",
        "按下：開啟靜態增益補償 (SGC)。SGC 不精確，但不會影響動態。",
        "按下：開始測量輸入訊號和輸出訊號的整體響度。\n釋放：關閉 AGC，並更新輸出增益為兩者響度差值。",
        "按下：開啟自動增益補償 (AGC)。AGC 長期更精確，但會影響動態。",
        "按下：翻轉輸出訊號的相位。",
        "控制側鏈訊號的前瞻時間。",

        "按下：開啟輸入訊號頻譜分析儀。",
        "按下：開啟輸出訊號頻譜分析儀。",
        "按下：開啟側鏈訊號頻譜分析儀。",
        "選擇頻譜分析儀的衰減速度。",
        "選擇頻譜分析儀的傾斜斜率。",
        "按下：開啟凍結功能。將滑鼠懸停在分析儀上可凍結頻譜。",
        "按下：開啟碰撞偵測。",
        "控制碰撞偵測的強度。",

        "選擇濾波器結構。",
        "標準、經典的數位結構。不適合激進的參數自動化。",
        "用於合成器濾波器和分頻器的穩定結構。適合激進的參數自動化。會導致較大的相位偏移。",
        "平緩的擱架 (Shelf) 與峰值 (Peak) 濾波器被並行處理。高效且自然的動態處理。",
        "模擬類比原型的幅度與相位響應。請勿在此模式下自動化濾波器參數。",
        "模擬類比原型的幅度響應並清理高頻相位。請勿在此模式下自動化濾波器參數。",
        "模擬類比原型的幅度響應並清理中高頻相位。請勿在此模式下自動化濾波器參數。",

        "雙擊：開啟使用者介面設定。",

        "按下：開啟均衡匹配面板。",
        "點擊：將當前目標曲線儲存為預設檔案。",
        "選擇：選擇目標曲線。",
        "從側鏈訊號中學習目標曲線。",
        "將目標曲線設定為一條平坦的線。",
        "從預設檔案中載入目標曲線。",
        "控制差異曲線的偏移。",
        "控制差異曲線的平滑度。",
        "控制差異曲線的斜率。",
        "點擊：開始擬合過程。",
        "控制頻段數量。",
        "均衡匹配模型正在分析主鏈訊號和側鏈訊號。\n來源訊號必須是主鏈訊號，但您可以選擇目標訊號。\n來源訊號與目標訊號之間的差異即為差異曲線。\n當差異曲線穩定後，您可以在開始擬合過程前對其進行進一步調整。",
        "均衡匹配模型正在執行擬合過程。此過程最多需要幾秒鐘。",
        "均衡匹配模型已完成擬合過程。您現在可以變更用於擬合的頻段數量。",
        "按下：啟用差異曲線繪製。"
    };
}
