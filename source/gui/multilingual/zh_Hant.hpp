// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_MULTILINGUAL_ZH_TW_HPP
#define ZL_INTERFACE_MULTILINGUAL_ZH_TW_HPP

#include <array>

namespace zlInterface::multilingual::zh_Hant {
    static constexpr std::array texts = {
        "按下：開啟所選頻段。\n釋放：旁通所選頻段。",
        "按下：獨奏當前頻段所作用的音訊。",
        "選擇所選頻段的類型。Peak（鐘型）、Low Shelf（低頻增益）、Low Pass（低通）、High Shelf（高頻增益）、High Pass（高通）、Notch（帶阻）、Band Pass（帶通）、Tilt Shelf（傾斜增益）。",
        "選擇所選頻段的斜率。較高的斜率會使濾波器的響應曲線變化更陡峭。",
        "選擇所選頻段的立體聲模式。Stereo（立體聲）、Left（左聲道）、Right（右聲道）、Mid（中聲道）、Side（側聲道）。",
        "調整所選頻段的中心頻率。",
        "調整所選頻段的增益。",
        "調整所選頻段的 Q 值。Q 值越大，頻寬越窄。",
        "選擇頻段（使用左右箭頭）。",
        "按下：開啟所選頻段的動態行為。",
        "按下：開啟所選頻段的自動閾值。\n釋放：關閉自動閾值，並應用學習到的閾值。",
        "點擊：關閉所選頻段。",
        "釋放：旁通所選頻段的動態行為。",
        "按下：獨奏當前頻段所使用側鏈的音訊。",
        "按下：將動態閾值設置為相對模式。\n釋放：將動態閾值設置為絕對模式。",
        "按下：交換側鏈（側鏈將處於相反的立體聲模式）。",
        "調整所選頻段的動態行為閾值。",
        "調整所選頻段的動態行為拐點寬度。實際拐點寬度 = 顯示值 × 60 dB。",
        "調整所選頻段的動態行為的攻擊時間。",
        "調整所選頻段的動態行為的釋放時間。",
        "調整應用於側鏈訊號的帶通濾波器的中心頻率。",
        "調整應用於側鏈訊號的帶通濾波器的 Q 值。",
        "按下：使用外部側鏈。\n釋放：使用內部側鏈。",
        "按下：開啟靜態增益補償 (SGC)。SGC 根據濾波器參數估算補償量。SGC 計算不精確，但不會影響訊號的動態。",
        "按下：旁通插件。",
        "調整所有增益參數的縮放比例。",
        "按下：翻轉輸出訊號的相位。",
        "按下：開啟自動增益補償 (AGC)。AGC 在長時間內是精確的，但會影響訊號的動態。",
        "按下：開始測量輸入訊號和輸出訊號的整體響度。\n釋放：關閉 AGC，並更新輸出增益為兩者響度差值。",
        "調整插件的輸出增益。",
        "選擇輸入 FFT 分析儀的狀態。FRZ 可凍結分析儀。",
        "選擇輸出 FFT 分析儀的狀態。FRZ 可凍結分析儀。",
        "選擇側鏈 FFT 分析儀的狀態。FRZ 可凍結分析儀。",
        "調整 FFT 分析儀的衰減速度。",
        "調整 FFT 分析儀的斜率（不影響實際訊號）。",
        "調整側鏈訊號的前瞻時間（引入延遲）。",
        "調整 RMS 長度。RMS 越大，攻擊/釋放會更慢、更平滑。",
        "調整釋放的平滑度。",
        "選擇高品質選項的狀態。開啟高品質模式時，動態行為將逐樣本調整濾波器狀態，使動態效果更平滑並減少偽影。",
        "選擇碰撞檢測的狀態。",
        "調整碰撞檢測的強度。增加強度會使檢測區域變得更大、更暗。",
        "調整碰撞檢測的縮放比例。增加縮放比例會使檢測區域變得更暗。",
        "選擇濾波器的結構。",
        "選擇零延遲模式的狀態。開啟零延遲模式時，額外延遲為零，但緩衝區大小會略微影響動態效果和參數自動化。",
        "選擇目標曲線。\nSide：從側鏈訊號學習。\nPreset：從預設文件載入。\nFlat：設為平坦 -4.5 dB/oct 線。",
        "調整曲線學習模型的權重。權重越大，模型越傾向於從訊號的響亮部分學習。",
        "點擊：開始曲線學習。",
        "點擊：將當前目標曲線儲存為預設文件。",
        "調整差異曲線的平滑度。",
        "調整差異曲線的斜率。",
        "選擇擬合演算法。\nLD：基於局部梯度的演算法（較快）。\nGN：基於全域無梯度的演算法（推薦）。\nGN+：基於全域無梯度的演算法（較慢，允許濾波器具有更高的斜率）。",
        "點擊：開始曲線擬合。",
        "調整用於擬合的頻段數量。當擬合完成後，曲線擬合模型會建議頻段數量，之後你可以進行調整。",
        "選擇濾波器響應曲線（左側）和頻譜分析儀（右側）的 dB 刻度。",
        "點擊：打開均衡匹配界面。均衡匹配分為四步：\n1. 選擇目標。\n2. 開始學習。\n3. 調整差值。\n4. 開始擬合。",
        "均衡器中最常見的濾波器結構。濾波器響應非常接近模擬原型。適合於調制。",
        "分頻器中最常見的濾波器結構。濾波器會引起更多的相位偏移。適合於快速調制。",
        "峰值/擱架濾波器並行放置。濾波器響應與顯示的曲線不同。適合於調制。",
        "具有額外校正的最小相位結構。濾波器響應幾乎與模擬原型相同。不適合於調制。",
        "具有額外校正的最小相位結構。濾波器具有模擬原型的幅度響應，並且在高頻相位偏移更小。不適合於調制。",
        "濾波器具有模擬原型的幅度響應和零相位響應。不適合於調制。動態效果不工作。",
        "雙擊：打開用戶界面設置"
    };
}

#endif //ZL_INTERFACE_MULTILINGUAL_ZH_TW_HPP
