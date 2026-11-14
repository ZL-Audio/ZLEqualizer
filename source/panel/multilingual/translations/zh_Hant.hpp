// Copyright (C) 2026 - zsliu98
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
    inline constexpr std::array kTexts = {
        // band basic controls
        "切換頻段旁通狀態。",
        "切換頻段獨奏監聽。",
        "選擇濾波器類型。",
        "選擇濾波器斜率。斜率越高，濾波器的頻率響應曲線變化越陡峭。",
        "選擇立體聲通道模式。",
        "調節中心頻率或截止頻率。",
        "調節基礎增益與動態目標增益。",
        "調節品質因數 (Q 值)。Q 值越大，頻寬越窄。",
        "切換此頻段的動態均衡行為。",
        "關閉此頻段。",
        "選擇上一個頻段。",
        "選擇下一個頻段。",
        "切換低通和高通濾波器的響應縮放。",
        "調節低通或高通濾波器的幅度縮放比例。",
        // band dynamic controls
        "切換此頻段的動態處理旁通狀態。",
        "切換動態閾值自動學習。關閉時將自動設定閾值與拐點。",
        "切換動態處理的相對閾值模式。",
        "切換側鏈立體聲通道互換。",
        "調節動態處理的閾值。",
        "調節動態處理的拐點寬度。",
        "調節動態處理的啟動時間。",
        "調節動態處理的釋放時間。",
        "將側鏈濾波器的頻率和 Q 值與此頻段關聯。",
        "選擇側鏈濾波器類型。",
        "選擇側鏈濾波器斜率。",
        "調節側鏈濾波器頻率。",
        "調節側鏈濾波器 Q 值。",
        "切換額外的動態偵測設定面板。",
        "調節電平偵測的 RMS 積分視窗長度。",
        "調節峰值偵測與 RMS 電平偵測之間的混合比例。",
        "調節套用於動態包絡的平滑程度。",
        // top controls
        "切換介面設定面板。",
        "切換預設瀏覽器。",
        "切換分析儀設定面板。",
        "切換輸出設定面板。",
        "選擇濾波器結構。",
        "標準經典的數位結構。不適合激進的參數自動化。",
        "用於合成器濾波器和分頻器的穩定結構。適合激進的參數自動化，但會引入較大的相位偏移。",
        "以並行方式處理平緩的擱架與峰值濾波器，實現高效且自然的動態處理。",
        "模擬類比原型的幅度與相位響應。請勿在此模式下自動化濾波器參數。",
        "模擬類比原型的幅度響應並清理高頻相位。請勿在此模式下自動化濾波器參數。",
        "模擬類比原型的幅度響應並清理中高頻相位。請勿在此模式下自動化濾波器參數。",
        "切換插件旁通狀態。",
        "切換外部側鏈輸入。",
        "切換均衡匹配面板。",
        // preset browser
        "在系統檔案管理器中開啟預設資料夾。",
        "刪除所選預設分組。",
        "刪除所選預設。",
        "關閉預設瀏覽器。",
        // output controls
        "調節額外的輸出增益。",
        "調節所有濾波器基礎增益與目標增益的整體縮放比例。",
        "切換靜態增益補償 (SGC)。SGC 為近似補償，但不會影響動態。",
        "切換響度增益匹配。開啟時測量輸入與輸出響度，關閉時將輸出增益設為兩者的響度差值。",
        "切換自動增益補償 (AGC)。AGC 長期更精確，但會影響動態。",
        "切換輸出訊號的相位反轉。",
        "調節側鏈訊號的前瞻時間。",
        // analyzer controls
        "切換輸入訊號頻譜的顯示。",
        "切換輸出訊號頻譜的顯示。",
        "切換側鏈訊號頻譜的顯示。",
        "選擇頻譜分析儀的衰減速度。",
        "選擇頻譜分析儀的傾斜斜率。",
        "選擇頻譜分析儀的頻率平滑寬度。",
        "選擇頻譜分析儀的頻率平滑模式。\nOCT：等比例分數倍頻程平滑。\nERB：符合人耳聽覺的等效矩形頻寬平滑。",
        "切換頻譜凍結功能。將滑鼠懸停在分析儀上時凍結頻譜。",
        "選擇頻譜分析儀顯示的立體聲通道模式。",
        "切換頻率碰撞偵測。",
        "調節碰撞偵測的強度。",
        "切換數值讀數的顯示。",
        "切換電平表欄的顯示。",
        // curve scale & piano roll
        "選擇均衡響應曲線的分貝顯示範圍。",
        "選擇頻譜分析儀顯示的上限分貝值。",
        "選擇頻譜分析儀顯示的最小分貝下限。",
        "切換鋼琴捲簾的顯示。",
        // eq match controls
        "將目標曲線儲存為預設檔案。",
        "切換差異曲線的手動繪製。",
        "選擇目標曲線來源。",
        "從側鏈訊號中學習目標曲線。",
        "將目標曲線設定為平坦直線。",
        "從預設檔案中載入目標曲線。",
        "將目標曲線設定為平衡曲線。",
        "將目標曲線設定為自然曲線。",
        "調節差異曲線的垂直偏移。",
        "調節差異曲線的垂直縮放比例。",
        "調節差異曲線的傾斜斜率。",
        "選擇均衡匹配允許的最大增益幅度限制。",
        "開始將均衡頻段擬合到差異曲線。",
        "調節用於擬合的均衡頻段數量。",
        "正在分析主鏈訊號和側鏈訊號。\n請在差異曲線穩定後選擇目標曲線並調整差異曲線，然後開始擬合。",
        "正在將均衡頻段擬合到差異曲線，此過程最多需要幾秒鐘。",
        "擬合已完成。如有需要，可調整用於擬合的頻段數量。",
        " "
    };
}
