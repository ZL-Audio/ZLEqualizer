// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_MULTILINGUAL_ZH_CN_HPP
#define ZL_INTERFACE_MULTILINGUAL_ZH_CN_HPP

#include <array>

namespace zlInterface::multilingual::zh_Hans {
    static constexpr std::array texts = {
        "按下：打开所选频段。\n释放：旁通所选频段。",
        "按下：监听当前频段所作用的音频。",
        "选择所选频段的类型。Peak（峰值）、Low Shelf（低频增益）、Low Pass（低通）、High Shelf（高频增益）、High Pass（高通）、Notch（陷波）、Band Pass（带通）、Tilt Shelf（倾斜增益）。",
        "选择所选频段的斜率。较高的斜率会使滤波器的响应曲线变化更陡峭。",
        "选择所选频段的立体声模式。Stereo（立体声）、Left（左声道）、Right（右声道）、Mid（中声道）、Side（侧声道）。",
        "调整所选频段的中心频率。",
        "调整所选频段的增益。",
        "调整所选频段的 Q 值。Q 值越大，带宽越窄。",
        "选择频段（使用左右箭头）。",
        "按下：打开所选频段的动态行为。",
        "按下：打开所选频段的自动阈值。\n释放：关闭自动阈值，并应用学习到的阈值。",
        "点击：关闭所选频段。",
        "释放：旁通所选频段的动态行为。",
        "按下：监听当前频段所使用侧链的音频",
        "按下：将动态阈值设置为相对模式。\n释放：将动态阈值设置为绝对模式。",
        "按下：交换侧链（侧链将处于相反的立体声模式）。",
        "调整所选频段的动态行为阈值。",
        "调整所选频段的动态行为拐点宽度。实际拐点宽度 = 显示值 × 60 dB。",
        "调整所选频段的动态行为的攻击时间。",
        "调整所选频段的动态行为的释放时间。",
        "调整应用于侧链信号的带通滤波器的中心频率。",
        "调整应用于侧链信号的带通滤波器的 Q 值。",
        "按下：使用外部侧链。\n释放：使用内部侧链。",
        "按下：开启静态增益补偿 (SGC)。SGC 根据滤波器参数估算补偿量。SGC 计算不精确，但不会影响信号的动态。",
        "按下：旁通插件。",
        "调整所有增益参数的缩放比例。",
        "按下：翻转输出信号的相位。",
        "按下：开启自动增益补偿 (AGC)。AGC 在长时间内是精确的，但会影响信号的动态。",
        "按下：开始测量输入信号和输出信号的整体响度。\n释放：关闭 AGC，并更新输出增益为两者响度差值。",
        "调整插件的输出增益。",
        "选择输入频谱分析仪的状态。FRZ 可冻结分析仪。",
        "选择输出频谱分析仪的状态。FRZ 可冻结分析仪。",
        "选择侧链频谱分析仪的状态。FRZ 可冻结分析仪。",
        "调整频谱分析仪的衰减速度。",
        "调整频谱分析仪的斜率（不会影响实际信号）。",
        "调整侧链信号的前瞻时间（引入延迟）。",
        "调整 RMS 长度。RMS 越大，攻击/释放会更慢、更平滑。",
        "调整释放的平滑度。",
        "选择高质量选项的状态。开启高质量模式时，动态行为将逐样本调整滤波器状态，使动态效果更平滑并减少伪影。",
        "选择碰撞检测的状态。",
        "调整碰撞检测的强度。增加强度会使检测区域变得更大、更暗。",
        "调整碰撞检测的缩放比例。增加缩放比例会使检测区域变得更暗。",
        "选择滤波器的结构。",
        "选择零延迟模式的状态。开启零延迟模式时，额外延迟为零，但缓冲区大小会略微影响动态效果和参数自动化。",
        "选择目标曲线。\nSide：从侧链信号学习。\nPreset：从预设文件加载。\nFlat：设为平坦 -4.5 dB/oct 线。",
        "调整曲线学习模型的权重。权重越大，模型越倾向于从信号的响亮部分学习。",
        "点击：开始曲线学习。",
        "点击：将当前目标曲线保存为预设文件。",
        "调整差异曲线的平滑度。",
        "调整差异曲线的斜率。",
        "选择拟合算法。\nLD：基于局部梯度的算法。（较快）\nGN：基于全局无梯度的算法。（推荐）\nGN+：基于全局无梯度的算法（较慢，允许滤波器具有更高的斜率）。",
        "点击：开始曲线拟合。",
        "调整用于拟合的频段数量。当拟合完成后，曲线拟合模型会建议频段数量，之后你可以进行调整。",
        "选择滤波器响应曲线（左侧）和频谱分析仪（右侧）的 dB 刻度。",
        "点击：打开均衡匹配界面。均衡匹配分为四步：\n1. 选择目标。\n2. 开始学习。\n3. 调整差值。\n4. 开始拟合。",
        "均衡器中最常见的滤波器结构。滤波器响应非常接近模拟原型。适合于调制。",
        "分频器中最常见的滤波器结构。滤波器会引起更多的相位偏移。适合于快速调制。",
        "峰值/搁架滤波器并行放置。滤波器响应与显示的曲线不同。适合于调制。",
        "具有额外校正的最小相位结构。滤波器响应几乎与模拟原型相同。不适合于调制。",
        "具有额外校正的最小相位结构。滤波器具有模拟原型的幅度响应，并且在高频相位偏移更小。不适合于调制。",
        "滤波器具有模拟原型的幅度响应和零相位响应。不适合于调制。动态效果不工作。",
        "双击：打开用户界面设置。"
    };
}

#endif //ZL_INTERFACE_MULTILINGUAL_ZH_CN_HPP
