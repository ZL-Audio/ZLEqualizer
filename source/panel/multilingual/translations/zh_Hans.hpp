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

namespace zlpanel::multilingual::zh_Hans {
    static constexpr std::array kTexts = {
        "释放：旁通此频段。",
        "按下：监听此频段。",
        "选择滤波器类型。",
        "选择滤波器斜率。较高的斜率会使滤波器的响应曲线变化更陡峭。",
        "选择立体声模式。",
        "控制频率。",
        "控制基础增益与目标增益。",
        "控制品质因数 (Q 值)。Q 值越大，带宽越窄。",
        "按下：开启动态行为。",
        "点击：关闭此频段。",
        "释放：旁通动态行为。",
        "按下：开启动态学习行为。\n释放：关闭动态学习行为，并设置阈值 (Threshold) 与拐点 (Knee)。\n详情参见手册。",
        "按下：开启动态相对行为。\n详情参见手册。",
        "按下：更改侧链立体声模式。",
        "控制动态行为的阈值。\n详情参见手册。",
        "控制动态行为的拐点宽度。\n详情参见手册。",
        "控制动态行为的攻击时间。\n详情参见手册。",
        "控制动态行为的释放时间。\n详情参见手册。",
        "按下：将此频段与侧链频段关联。",
        "选择侧链滤波器类型。",
        "选择侧链滤波器斜率。",
        "控制侧链滤波器频率。",
        "控制侧链滤波器 Q 值。",

        "释放：旁通插件。",
        "按下：使用外部侧链。\n释放：使用内部侧链。",

        "控制额外的输出增益。",
        "控制所有滤波器基础增益和目标增益的缩放比例。",
        "按下：开启静态增益补偿 (SGC)。SGC 不精确，但不会影响动态。",
        "按下：开始测量输入信号和输出信号的整体响度。\n释放：关闭 AGC，并更新输出增益为两者响度差值。",
        "按下：开启自动增益补偿 (AGC)。AGC 长期更精确，但会影响动态。",
        "按下：翻转输出信号的相位。",
        "控制侧链信号的前瞻时间。",

        "按下：开启输入信号频谱分析仪。",
        "按下：开启输出信号频谱分析仪。",
        "按下：开启侧链信号频谱分析仪。",
        "选择频谱分析仪的衰减速度。",
        "选择频谱分析仪的倾斜斜率。",
        "按下：开启冻结功能。将鼠标悬停在分析仪上可冻结频谱。",
        "按下：开启碰撞检测。",
        "控制碰撞检测的强度。",

        "选择滤波器结构。",
        "标准、经典的数字结构。不适合激进的参数自动化。",
        "用于合成器滤波器和分频器的稳定结构。适合激进的参数自动化。会导致较大的相位偏移。",
        "平缓的搁架 (Shelf) 与峰值 (Peak) 滤波器被并行处理。高效且自然的动态处理。",
        "模拟模拟原型的幅度与相位响应。请勿在此模式下自动化滤波器参数。",
        "模拟模拟原型的幅度响应并清理高频相位。请勿在此模式下自动化滤波器参数。",
        "模拟模拟原型的幅度响应并清理中高频相位。请勿在此模式下自动化滤波器参数。",

        "双击：打开用户界面设置。",

        "按下：打开均衡匹配面板。",
        "点击：将目标曲线保存为预设文件。",
        "选择：选择目标曲线。",
        "从侧链信号中学习目标曲线。",
        "将目标曲线设置为一条平坦的线。",
        "从预设文件中加载目标曲线。",
        "控制差异曲线的偏移。",
        "控制差异曲线的平滑度。",
        "控制差异曲线的斜率。",
        "点击：开始拟合过程。",
        "控制频段数量。",
        "均衡匹配模型正在分析主链信号和侧链信号。\n源信号必须是主链信号，但您可以选择目标信号。\n源信号与目标信号之间的差异即为差异曲线。\n当差异曲线稳定后，您可以在开始拟合过程前对其进行进一步调整。",
        "均衡匹配模型正在运行拟合过程。此过程最多需要几秒钟。",
        "均衡匹配模型已完成拟合过程。您现在可以更改用于拟合的频段数量。",
        "按下：启用差异曲线绘制。"
    };
}
