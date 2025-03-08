// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_DEFINES_H
#define ZL_INTERFACE_DEFINES_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../state/state_definitions.hpp"
#include "multilingual/multilingual.hpp"

namespace zlInterface {
    enum colourIdx {
        textColour,
        backgroundColour,
        shadowColour,
        glowColour,
        preColour,
        postColour,
        sideColour,
        gridColour,
        tagColour,
        gainColour,
        sideLoudnessColour,
        colourNum
    };

    enum sensitivityIdx {
        mouseWheel,
        mouseWheelFine,
        mouseDrag,
        mouseDragFine,
        sensitivityNum
    };

    enum settingIdx {
        uiSettingPanelShow,
        matchPanelShow,
        matchPanelFit,
        matchLowCut,
        matchHighCut,
        matchShift,
        matchFitRunning,
        settingNum
    };

    inline std::array identifiers{
        juce::Identifier("ui_setting_panel_show"),
        juce::Identifier("match_panel_show"), juce::Identifier("match_panel_fit"),
        juce::Identifier("match_low_cut"), juce::Identifier("match_high_cut"),
        juce::Identifier("match_shift"), juce::Identifier("match_fit_running")
    };

    inline std::array<std::string, colourNum> colourNames{
        "text", "background",
        "shadow", "glow",
        "pre", "post", "side",
        "grid", "tag", "gain", "side_loudness"
    };

    static constexpr size_t ColorMap1Size = 10;
    static constexpr size_t ColorMap2Size = 6;

    inline std::array colourMaps = {
        std::vector<juce::Colour>{
            juce::Colour(31, 119, 180),
            juce::Colour(255, 127, 14),
            juce::Colour(44, 160, 44),
            juce::Colour(214, 39, 40),
            juce::Colour(148, 103, 189),
            juce::Colour(140, 86, 75),
            juce::Colour(227, 119, 194),
            juce::Colour(127, 127, 127),
            juce::Colour(188, 189, 34),
            juce::Colour(23, 190, 207)
        },
        std::vector<juce::Colour>{
            juce::Colour(224, 136, 75),
            juce::Colour(0, 128, 241),
            juce::Colour(211, 95, 211),
            juce::Colour(41, 216, 215),
            juce::Colour(107, 152, 66),
            juce::Colour(115, 169, 180),
            juce::Colour(28, 136, 61),
            juce::Colour(128, 128, 128),
            juce::Colour(67, 66, 221),
            juce::Colour(232, 65, 48),
        },
        std::vector<juce::Colour>{
            juce::Colour(76, 114, 176),
            juce::Colour(85, 168, 104),
            juce::Colour(196, 78, 82),
            juce::Colour(129, 114, 178),
            juce::Colour(255, 196, 0),
            juce::Colour(100, 181, 205),
        },
        std::vector<juce::Colour>{
            juce::Colour(179, 141, 79),
            juce::Colour(170, 87, 151),
            juce::Colour(59, 177, 173),
            juce::Colour(126, 141, 77),
            juce::Colour(51, 70, 139),
            juce::Colour(155, 74, 50),
        },
        std::vector<juce::Colour>{
            juce::Colour(0, 63, 255),
            juce::Colour(3, 237, 58),
            juce::Colour(232, 0, 11),
            juce::Colour(138, 43, 226),
            juce::Colour(255, 196, 0),
            juce::Colour(0, 215, 255),
        },
        std::vector<juce::Colour>{
            juce::Colour(255, 192, 0),
            juce::Colour(252, 18, 197),
            juce::Colour(23, 255, 244),
            juce::Colour(117, 212, 29),
            juce::Colour(0, 59, 255),
            juce::Colour(255, 40, 0),
        }
    };

    auto inline constexpr FontTiny = 0.5f;
    auto inline constexpr FontSmall = 0.75f;
    auto inline constexpr FontNormal = 1.0f;
    auto inline constexpr FontLarge = 1.25f;
    auto inline constexpr FontHuge = 1.5f;
    auto inline constexpr FontHuge2 = 3.0f;
    auto inline constexpr FontHuge3 = 4.5f;

    auto inline constexpr RefreshFreqHz = 60;

    struct fillRoundedShadowRectangleArgs {
        float blurRadius = 0.5f;
        bool curveTopLeft = true, curveTopRight = true, curveBottomLeft = true, curveBottomRight = true;
        bool fit = true, flip = false;
        bool drawBright = true, drawDark = true, drawMain = true;
        juce::Colour mainColour = juce::Colours::white.withAlpha(0.f);
        juce::Colour darkShadowColor = juce::Colours::white.withAlpha(0.f);
        juce::Colour brightShadowColor = juce::Colours::white.withAlpha(0.f);
        bool changeMain = false, changeDark = false, changeBright = false;
    };

    struct fillShadowEllipseArgs {
        float blurRadius = 0.5f;
        bool fit = true, flip = false;
        bool drawBright = true, drawDark = true;
        juce::Colour mainColour = juce::Colours::white.withAlpha(0.f);
        juce::Colour darkShadowColor = juce::Colours::white.withAlpha(0.f);
        juce::Colour brightShadowColor = juce::Colours::white.withAlpha(0.f);
        bool changeMain = false, changeDark = false, changeBright = false;
    };

    inline std::string formatFloat(float x, int precision) {
        std::stringstream stream;
        precision = std::max(0, precision);
        stream << std::fixed << std::setprecision(precision) << x;
        return stream.str();
    }

    inline std::string fixFormatFloat(float x, int length) {
        auto y = std::abs(x);
        if (y < 10) {
            return formatFloat(x, length - 1);
        } else if (y < 100) {
            return formatFloat(x, length - 2);
        } else if (y < 1000) {
            return formatFloat(x, length - 3);
        } else if (y < 10000) {
            return formatFloat(x, length - 4);
        } else {
            return formatFloat(x, length - 5);
        }
    }

    class UIBase {
    public:
        explicit UIBase(juce::AudioProcessorValueTreeState &apvts)
            : state(apvts), fontSize{0.f} {
            loadFromAPVTS();
            updateActualLangIdx();
        }

        void setFontSize(const float fSize) { fontSize = fSize; }

        inline float getFontSize() const { return fontSize; }

        inline juce::Colour getTextColor() const {
            return customColours[static_cast<size_t>(textColour)];
        }

        inline juce::Colour getTextInactiveColor() const { return getTextColor().withAlpha(0.5f); }

        inline juce::Colour getTextHideColor() const { return getTextColor().withAlpha(0.25f); }

        inline juce::Colour getBackgroundColor() const {
            return customColours[static_cast<size_t>(backgroundColour)];
        }

        inline juce::Colour getBackgroundInactiveColor() const { return getBackgroundColor().withAlpha(0.8f); }

        inline juce::Colour getBackgroundHideColor() const { return getBackgroundColor().withAlpha(0.5f); }

        inline juce::Colour getDarkShadowColor() const {
            return customColours[static_cast<size_t>(shadowColour)];
        }

        inline juce::Colour getBrightShadowColor() const {
            return customColours[static_cast<size_t>(glowColour)];
        }

        inline juce::Colour getColorMap1(const size_t idx) const {
            return colourMaps[cMap1Idx][idx % colourMaps[cMap1Idx].size()];
        }

        inline juce::Colour getColorMap2(const size_t idx) const {
            return colourMaps[cMap2Idx][idx % colourMaps[cMap2Idx].size()];
        }

        static juce::Rectangle<float> getRoundedShadowRectangleArea(juce::Rectangle<float> boxBounds,
                                                                    float cornerSize,
                                                                    const fillRoundedShadowRectangleArgs &margs);

        juce::Rectangle<float> fillRoundedShadowRectangle(juce::Graphics &g,
                                                          juce::Rectangle<float> boxBounds,
                                                          float cornerSize,
                                                          const fillRoundedShadowRectangleArgs &margs) const;

        juce::Rectangle<float> fillRoundedInnerShadowRectangle(juce::Graphics &g,
                                                               juce::Rectangle<float> boxBounds,
                                                               float cornerSize,
                                                               const fillRoundedShadowRectangleArgs &margs) const;

        static juce::Rectangle<float> getShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                           float cornerSize,
                                                           const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawShadowEllipse(juce::Graphics &g,
                                                 juce::Rectangle<float> boxBounds,
                                                 float cornerSize,
                                                 const fillShadowEllipseArgs &margs) const;

        static juce::Rectangle<float> getInnerShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                                float cornerSize,
                                                                const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawInnerShadowEllipse(juce::Graphics &g,
                                                      juce::Rectangle<float> boxBounds,
                                                      float cornerSize,
                                                      const fillShadowEllipseArgs &margs) const;

        juce::Colour getColourByIdx(colourIdx idx) const {
            return customColours[static_cast<size_t>(idx)];
        }

        void setColourByIdx(const colourIdx idx, const juce::Colour colour) {
            customColours[static_cast<size_t>(idx)] = colour;
        }

        inline float getSensitivity(const sensitivityIdx idx) const {
            return wheelSensitivity[static_cast<size_t>(idx)];
        }

        void setSensitivity(const float v, const sensitivityIdx idx) {
            wheelSensitivity[static_cast<size_t>(idx)] = v;
        }

        size_t getRotaryStyleID() const {
            return rotaryStyleId;
        }

        juce::Slider::SliderStyle getRotaryStyle() const {
            return zlState::rotaryStyle::styles[rotaryStyleId];
        }

        void setRotaryStyleID(const size_t x) {
            rotaryStyleId = x;
        }

        float getRotaryDragSensitivity() const {
            return rotaryDragSensitivity;
        }

        void setRotaryDragSensitivity(const float x) {
            rotaryDragSensitivity = x;
        }

        juce::int64 getRefreshRateMS() const {
            return static_cast<juce::int64>(zlState::refreshRate::ms[refreshRateId.load()]);
        }

        size_t getRefreshRateID() const {
            return refreshRateId.load();
        }

        void setRefreshRateID(const size_t x) {
            refreshRateId.store(x);
        }

        float getFFTExtraTilt() const {
            return fftExtraTilt.load();
        }

        void setFFTExtraTilt(const float x) {
            fftExtraTilt.store(x);
        }

        float getFFTExtraSpeed() const {
            return fftExtraSpeed.load();
        }

        void setFFTExtraSpeed(const float x) {
            fftExtraSpeed.store(x);
        }

        float getSingleCurveThickness() const {
            return singleCurveThickness.load();
        }

        void setSingleCurveThickness(const float x) {
            singleCurveThickness.store(x);
        }

        float getSumCurveThickness() const {
            return sumCurveThickness.load();
        }

        void setSumCurveThickness(const float x) {
            sumCurveThickness.store(x);
        }

        void loadFromAPVTS();

        void saveToAPVTS() const;

        bool getIsBandSelected(const size_t x) const { return isBandSelected[x].load(); }

        void setIsBandSelected(const size_t x, const bool f) { isBandSelected[x].store(f); }

        bool getIsMouseWheelShiftReverse() const { return isMouseWheelShiftReverse.load(); }

        void setIsMouseWheelShiftReverse(const bool x) { isMouseWheelShiftReverse.store(x); }

        bool getIsSliderDoubleClickOpenEditor() const { return isSliderDoubleClickOpenEditor.load(); }

        void setIsSliderDoubleClickOpenEditor(const bool x) { isSliderDoubleClickOpenEditor.store(x); }

        int getDefaultPassFilterSlope() const { return defaultPassFilterSlope.load(); }

        void setDefaultPassFilterSlope(const int x) { defaultPassFilterSlope.store(x); }

        size_t getCMap1Idx() const { return cMap1Idx; }

        void setCMap1Idx(const size_t x) { cMap1Idx = x; }

        size_t getCMap2Idx() const { return cMap2Idx; }

        void setCMap2Idx(const size_t x) { cMap2Idx = x; }

        int getFFTOrderIdx() const { return fftOrderIdx; }

        void setFFTOrderIdx(const int x) { fftOrderIdx = x; }

        bool getDynLink() const { return dynLink.load(); }

        void setDynLink(const bool x) { dynLink.store(x); }

        int getRenderingEngine() const { return renderingEngine.load(); }

        void setRenderingEngine(const int x) { renderingEngine.store(x); }

        bool getTooltipON() const { return tooltipON; }

        void setTooltipON(const bool x) { tooltipON = x; }

        int getLangIdx() const { return static_cast<int>(langIdx); }

        void setLangIdx(const int x) { langIdx = static_cast<multilingual::languages>(x); }

        juce::ValueTree &getValueTree() { return valueTree; }

        juce::var getProperty(const settingIdx idx) const {
            return valueTree.getProperty(identifiers[static_cast<size_t>(idx)]);
        }

        void setProperty(const settingIdx idx, const juce::var &v) {
            valueTree.setProperty(identifiers[idx], v, nullptr);
        }

        static bool isProperty(const settingIdx idx, const juce::Identifier &property) {
            return property == identifiers[static_cast<size_t>(idx)];
        }

        std::string getToolTipText(const zlInterface::multilingual::labels label) const {
            switch (langIdx) {
                case multilingual::languages::lang_en: {
                    return multilingual::en::texts[static_cast<size_t>(label)];
                }
                case multilingual::languages::lang_zh_Hans: {
                    return multilingual::zh_Hans::texts[static_cast<size_t>(label)];
                }
                case multilingual::languages::lang_zh_Hant: {
                    return multilingual::zh_Hant::texts[static_cast<size_t>(label)];
                }
                case multilingual::languages::lang_it: {
                    return multilingual::it::texts[static_cast<size_t>(label)];
                }
                case multilingual::languages::lang_ja: {
                    return multilingual::ja::texts[static_cast<size_t>(label)];
                }
                case multilingual::languages::lang_de: {
                    return multilingual::de::texts[static_cast<size_t>(label)];
                }
                case multilingual::languages::lang_system:
                case multilingual::languages::langNum:
                default: {
                    return multilingual::en::texts[static_cast<size_t>(label)];
                }
            }
        }

    private:
        juce::AudioProcessorValueTreeState &state;
        juce::ValueTree valueTree{"ui_setting"};

        float fontSize{0.f};
        std::array<juce::Colour, colourNum> customColours;
        std::array<float, sensitivityNum> wheelSensitivity{1.f, 0.12f, 1.f, .25f};
        size_t rotaryStyleId{0};
        std::atomic<size_t> refreshRateId{2};
        float rotaryDragSensitivity{1.f};
        std::atomic<float> fftExtraTilt{0.f}, fftExtraSpeed{1.f};
        std::atomic<float> singleCurveThickness{1.f}, sumCurveThickness{1.f};
        std::array<std::atomic<bool>, zlState::bandNUM> isBandSelected{};
        std::atomic<bool> isMouseWheelShiftReverse{false};
        std::atomic<bool> isSliderDoubleClickOpenEditor{false};
        std::atomic<int> defaultPassFilterSlope{1};
        std::atomic<bool> dynLink{true};
        std::atomic<int> renderingEngine{1};

        float loadPara(const std::string &id) const {
            return state.getRawParameterValue(id)->load();
        }

        void savePara(const std::string &id, const float x) const {
            const auto para = state.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(x);
            para->endChangeGesture();
        }

        size_t cMap1Idx{zlState::colourMapIdx::defaultDark};
        size_t cMap2Idx{zlState::colourMapIdx::seabornBrightDark};

        int fftOrderIdx{1};

        bool tooltipON{true};
        multilingual::languages langIdx{multilingual::languages::lang_en};
        multilingual::languages actualLangIdx{multilingual::languages::lang_en};

        void updateActualLangIdx();
    };
}

#endif //ZL_INTERFACE_DEFINES_H
