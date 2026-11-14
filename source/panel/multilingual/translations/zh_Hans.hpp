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

namespace zlpanel::multilingual::zh_Hans {
    inline constexpr std::array kTexts = {
        // band basic controls
        "切换频段旁通状态。",
        "切换频段独奏监听。",
        "选择滤波器类型。",
        "选择滤波器斜率。斜率越高，滤波器的频率响应曲线变化越陡峭。",
        "选择立体声通道模式。",
        "调节中心频率或截止频率。",
        "调节基础增益与动态目标增益。",
        "调节品质因数 (Q 值)。Q 值越大，带宽越窄。",
        "切换此频段的动态均衡行为。",
        "关闭此频段。",
        "选择上一个频段。",
        "选择下一个频段。",
        "切换低通和高通滤波器的响应缩放。",
        "调节低通或高通滤波器的幅度缩放比例。",
        // band dynamic controls
        "切换此频段的动态处理旁通状态。",
        "切换动态阈值自动学习。关闭时将自动设定阈值与拐点。",
        "切换动态处理的相对阈值模式。",
        "切换侧链立体声通道互换。",
        "调节动态处理的阈值。",
        "调节动态处理的拐点宽度。",
        "调节动态处理的启动时间。",
        "调节动态处理的释放时间。",
        "将侧链滤波器的频率和 Q 值与此频段关联。",
        "选择侧链滤波器类型。",
        "选择侧链滤波器斜率。",
        "调节侧链滤波器频率。",
        "调节侧链滤波器 Q 值。",
        "切换额外的动态检测设置面板。",
        "调节电平检测的 RMS 积分窗口长度。",
        "调节峰值检测与 RMS 电平检测之间的混合比例。",
        "调节应用于动态包络的平滑程度。",
        // top controls
        "切换界面设置面板。",
        "切换预设浏览器。",
        "切换分析仪设置面板。",
        "切换输出设置面板。",
        "选择滤波器结构。",
        "标准经典的数字结构。不适合激进的参数自动化。",
        "用于合成器滤波器和分频器的稳定结构。适合激进的参数自动化，但会引入较大的相位偏移。",
        "以并行方式处理平缓的搁架与峰值滤波器，实现高效且自然的动态处理。",
        "模拟模拟原型的幅度与相位响应。请勿在此模式下自动化滤波器参数。",
        "模拟模拟原型的幅度响应并清理高频相位。请勿在此模式下自动化滤波器参数。",
        "模拟模拟原型的幅度响应并清理中高频相位。请勿在此模式下自动化滤波器参数。",
        "切换插件旁通状态。",
        "切换外部侧链输入。",
        "切换均衡匹配面板。",
        // preset browser
        "在系统文件管理器中打开预设文件夹。",
        "删除所选预设分组。",
        "删除所选预设。",
        "关闭预设浏览器。",
        // output controls
        "调节额外的输出增益。",
        "调节所有滤波器基础增益与目标增益的整体缩放比例。",
        "切换静态增益补偿 (SGC)。SGC 为近似补偿，但不会影响动态。",
        "切换响度增益匹配。开启时测量输入与输出响度，关闭时将输出增益设为两者的响度差值。",
        "切换自动增益补偿 (AGC)。AGC 长期更精确，但会影响动态。",
        "切换输出信号的相位反转。",
        "调节侧链信号的前瞻时间。",
        // analyzer controls
        "切换输入信号频谱的显示。",
        "切换输出信号频谱的显示。",
        "切换侧链信号频谱的显示。",
        "选择频谱分析仪的衰减速度。",
        "选择频谱分析仪的倾斜斜率。",
        "选择频谱分析仪的频率平滑宽度。",
        "选择频谱分析仪的频率平滑模式。\nOCT：等比例分数倍频程平滑。\nERB：符合人耳听觉的等效矩形带宽平滑。",
        "切换频谱冻结功能。将鼠标悬停在分析仪上时冻结频谱。",
        "选择频谱分析仪显示的立体声通道模式。",
        "切换频率碰撞检测。",
        "调节碰撞检测的强度。",
        "切换数值读数的显示。",
        "切换电平表栏的显示。",
        // curve scale & piano roll
        "选择均衡响应曲线的分贝显示范围。",
        "选择频谱分析仪显示的上限分贝值。",
        "选择频谱分析仪显示的最小分贝下限。",
        "切换钢琴卷帘的显示。",
        // eq match controls
        "将目标曲线保存为预设文件。",
        "切换差异曲线的手动绘制。",
        "选择目标曲线来源。",
        "从侧链信号中学习目标曲线。",
        "将目标曲线设置为平坦直线。",
        "从预设文件中加载目标曲线。",
        "将目标曲线设置为平衡曲线。",
        "将目标曲线设置为自然曲线。",
        "调节差异曲线的垂直偏移。",
        "调节差异曲线的垂直缩放比例。",
        "调节差异曲线的倾斜斜率。",
        "选择均衡匹配允许的最大增益幅度限制。",
        "开始将均衡频段拟合到差异曲线。",
        "调节用于拟合的均衡频段数量。",
        "正在分析主链信号和侧链信号。\n请在差异曲线稳定后选择目标曲线并调整差异曲线，然后开始拟合。",
        "正在将均衡频段拟合到差异曲线，此过程最多需要几秒钟。",
        "拟合已完成。如有需要，可调整用于拟合的频段数量。",
        " "
    };
}
