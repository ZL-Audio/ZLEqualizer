# ZLEqualizer

LICENSE and CODE are available at [https://github.com/ZL-Audio/ZLEqualizer](https://github.com/ZL-Audio/ZLEqualizer)

# Changelog

## 0.6.2

DSP

- improve `Matched Phase`, `Mixed Phase`, `Linear Phase` performance

GUI

- fix hang-up when re-open GUI window in some hosts (see #194)

**WARNING for macOS users**

> There are one x86_64 version (for Intel chips) and one arm64 version (for Apple M chips). Please choose the correct installer.

**WARNING for users who build from source**

> The building instruction has been updated. Please read it carefully.

## 0.6.1

DSP

- improve LR/MS mode performance

GUI

- change top panels look and feel (now the panels are become half transparent when they are not open)
- fix pop-up floating window visibility issues (see #187)
- add multi-band target gain/Q changes (see #187)
- add pitch note supports for slider text editors
- add text editor supports for pop-up floating window frequency value
- improve pop-up floating window performance

Multilingual

- fix Italiano translations by [pms967](https://github.com/pms967) (already in 0.6.0, I forgot to mention it)
- fix 日本語 translations by [ikumi90s](https://github.com/ikumi90s)

## 0.6.0

- BREAKING: use the new compressor model from ZL Compressor
  - dynamic filters will sound a bit different
  - dynamic filters (especially if under dynamic high-quality mode) become more efficient
  - dynamic filters will produce expected results under extreme knee width settings (see #156)
- BREAKING: smooth filter frequency/gain/Q changes
  - quick move/automation of frequency/gain/Q will have fewer artifacts
  - filter parameter automation will take more CPU usage cause it now changes filters per sample (see the manual)
- change the external side-chain button position to the top-right corner
  - since it is not a per-band parameter, top-right corner seems to be a better place
- change several button icons (see the manual)
  - the old ones are not intuitive enough
- add English 简体中文 繁體中文 Italiano 日本語 Deutsch Español tooltips
  - 日本語 Deutsch Español tooltips are translated by ChatGPT. I cannot ensure the accuracy of the translation. If you observe incorrect translations, please report them to me or submit a pull request.
- add FFT db scale choice (see #147 and the manual)
- add loudness match (see #154 and the manual)
- add solo shortcuts (by right-mouse-button double-clicking with Ctrl/Command on the band button)
- add side-chain swap (see the manual)
- fix macOS standalone audio input permission request (see #146)
- fix curve db scale (see #148)
- fix multi-band selection (see #149)
- improve GUI performance
- improve DSP performance (especially for high-order filters and dynamic filters)
- improve stability

## 0.5.0

- BREAKING: change dynamic link to UI setting (the parameter is now shared across all instances)
- add Equalization Match (See the manual for details)
- add UI setting import/export
- fix incorrect channel layout (See #138)
- fix incorrect solo display (See #138)
- fix double-click behaviour (See #141)
- improve solo display
- improve GUI performance (especially when you open the UI setting panel)

## 0.4.5

- fix nlopt library linking issue in 0.4.4 (See #132) which breaks Linux and macOS binaries
- improve DSP stability

## 0.4.4

- fix uninitialized static gain compensation in some edge cases
- fix data race in some edge cases

## 0.4.3

- change the top panel layout (move global bypass and sgc buttons outside)
- change the UI setting panel layout
- add the resizing to default size function to UI button
- add default pass filter order & slider double-clicking to the UI setting panel
- add colour maps to the UI setting panel
- add fft resolution to the UI setting panel
- fix incorrect latency when the samplerate changes after loading the plugin
- fix combobox displays in `Standalone`
- improve `matched phase` `mixed phase` under high samplerates
- improve spectrum display accuracy in the high-end

## 0.4.2

- add & change slider value editor shortcuts
- open value editor with Ctrl/Command + mouse double-clicking for ALL sliders
- fix incorrect curve display when adjusting Scale
- fix macOS compatibility issue (now it should support macOS 10.13 and above)
- improve `mixed phase` phase shift continuity

## 0.4.1

- fix some debug code

## 0.4.0

- add parallel filter structure
- add match-phase filter structure
- add mixed-phase filter structure
- add linear-phase filter structure
- add & change shortcuts
  - fine-adjust dragger with Shift
  - find-adjust slider with Shift (changed from Ctrl/Command)
  - add a dynamic filter with Ctrl/Command + mouse double-clicking
  - turn on/off dynamic with Ctrl/Command + mouse double-clicking on the dragger
- change real curves to ideal curves
- change default FFT order to 12 to increase low-frequency details
- add phase-flip button and change the `Output` panel layout
- add more freedom to the plugin window size
- improve DSP & GUI stability
- improve DSP & GUI performance

## 0.3.5

- BREAKING: add auto dynamic threshold. When you press the dynamic threshold learning button, it will not only start the threshold learning but also automatically adjust the threshold. See the manual for more information.
- BREAKING: fix the reset button in the UI setting panel. When you press the reset button, it will reset colours to defaults.
- add standalone to releases
- change the plugin logo

## 0.3.4

- BREAKING: remove light/dark colour modes
- add text/background/shadow/glow colours to UI setting panel
- add multiband bypass/off
- add gain tag
- change bypass buttons LAF (now the light is off when the filter is bypassed)
- improve performance slightly

## 0.3.3

- add more colours to UI setting panel
- add AAX to macOS & Windows release

## 0.3.2

- bump to JUCE 8 (except for Linux)
- improve multi-band selection
- improve GUI performance

## 0.3.1

- add filter parameter tags
- fix mouse-wheel issues #63
- fix incorrect FFT curve at start-up
- improve stability

## 0.3.0

- add state-variable filter structure
- add dynamic link
- add dynamic high-quality
- fix collision detection
- improve IIR filter performance
- improve spectrum display

## 0.2.0

- add UI setting panel
- add static gain compensation
- add mono support
- add LV2 to Windows Release
- fix incorrect 6dB/oct high-pass filter & high-order peak filter
- fix incorrect RMS and lookahead behavior (when the sample rate changes)
- fix resizing problems on some hosts
- improve macOS and Windows installation
- improve GUI performance
- improve stability

## 0.1.16

- fix incorrect dynamic relative behaviour (endless loop, introduced in 0.1.14)
- fix incorrect RMS behaviour (compressor not working, introduced in 0.1.14)

## 0.1.15

- fix incorrect notch filter (introduced in 0.1.14)
- add slider mousewheel fine adjustment (with Ctrl/Command)
- change mousewheel behaviour when dynamic is on

## 0.1.14

- improve stability
- improve GUI performance when the dynamic is off
- support fixed gain/freq dragging
- add double-precision input support
- add the LV2 format to the Linux release

## 0.1.13

- makes plugin compatible with older libstdc++ (Linux)
- hard-clip auto-gain to prevent audio pops
- improve stability

## 0.1.12

- improve collision detection GUI
- improve coeff updating

## 0.1.11

- fix bugs of collision detection display

## 0.1.10

- improve collision detection GUI

## 0.1.9

- add multiple-filter simultaneous adjustment
- add auto gain compensation

## 0.1.8

- improve performance

## 0.1.7

- improve collision detecting
- fix minor bugs

## 0.1.6

- add collision detecting

## 0.1.5

- BREAKING: change the method of calculating high order cascading Q values
- fix minor bugs

## 0.1.4

- add overall bypass
- fix initial target filter gain

## 0.1.3

- improve GUI performance
- adjust RMS range and initial target filter gain

## 0.1.2

- improve GUI performance
- support Direct2D rendering

## 0.1.1

- improve GUI appearance

## 0.1.0

- add scale and output gain adjustment
- add pitch label and reset button to filter button popup window
- add target filter knob and side filter knob
- improve GUI performance

## 0.0.10

- adjust padding of state panels
- fix text colour of text editors

## 0.0.9

- improve GUI performance
- fix filter gain when added by double-clicking
- seperate pre/post/side FFT freezing
- add more dynamic settings

## 0.0.8

- replace FFT analyzer icon with text
- fix callbox display on Windows

## 0.0.7

- improve GUI performance significantly on macOS

## 0.0.6

- improve GUI performance slightly

## 0.0.5

- add turning on/off pre/post/side FFT seperately
- improve FFT GUI performance

## 0.0.4

- add FFT style, speed and tilt adjustment

## 0.0.3

- adjust FFT speed and resolution
- add rotary slider value editing

## 0.0.2

- improve FFT analyzer performance
- remove LV2 target from CMake to speed up the CI