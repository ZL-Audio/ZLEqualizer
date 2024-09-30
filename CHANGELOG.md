# ZLEqualizer

LICENSE and CODE are available at [https://github.com/ZL-Audio/ZLEqualizer](https://github.com/ZL-Audio/ZLEqualizer)

# Changelog

## 0.3.6

- add parallel filter structure
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

After installation, please load the plugin in an empty DAW session and check its functionality.

**WARNING for Linux users**: If you build it locally, please build with `juce7` branch.

**WARNING for Windows users**: The C++ runtime is now statically linked. However, you may have to install the [Latest Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) (if you are not able to open the plugin).

**WARNING for Pro Tool users**: AAX is updated in this version. If Pro Tools cannot recognize the plugin (or display an error message), please open a discussion.

- BREAKING: add auto dynamic threshold. When you press the dynamic threshold learning button, it will not only start the threshold learning but also automatically adjust the threshold. See the manual for more information.
- BREAKING: fix the reset button in the UI setting panel. When you press the reset button, it will reset colours to defaults.
- add standalone to releases
- change the plugin logo

## 0.3.4

After installation, please load the plugin in an empty DAW session and check its functionality.

**WARNING for Linux users**: If you build it locally, please build with `juce7` branch.

**WARNING for Windows users**: The C++ runtime is now statically linked. However, you may have to install the [Latest Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) (if you are not able to open the plugin).

**WARNING for Pro Tool users**: AAX is not updated in this version.

- BREAKING: remove light/dark colour modes
- add text/background/shadow/glow colours to UI setting panel
- add multiband bypass/off
- add gain tag
- change bypass buttons LAF (now the light is off when the filter is bypassed)
- improve performance slightly

## 0.3.3

After installation, please load the plugin in an empty DAW session and check its functionality.

**WARNING for Linux users**: If you build it locally, please build with `juce7` branch.

**WARNING for Windows users**: The C++ runtime is now statically linked. However, you may have to install the [Latest Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) (if you are not able to open the plugin).

**WARNING for Pro Tools users**: This is the first AAX plugin from me. If Pro Tools cannot recognize the plugin (or display an error message), please reach out to me.

- add more colours to UI setting panel
- add AAX to macOS & Windows release

## 0.3.2

Several dependencies have been updated. After installation, please load the plugin in an empty DAW session and check its functionality.

**WARNING for Linux users**: If you build it locally, please build with `juce7` branch since JUCE 8 has some issues on Linux.

**WARNING for Windows users**: You may have to install the [Latest Microsoft Visual C++ Redistributable](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) (if you are not able to open the plugin).

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