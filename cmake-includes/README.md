# CMake helpers

These helpers are maintained as part of this repository and loaded by the root
`CMakeLists.txt`. They were originally based on Pamplejuce; its license is retained
in [licenses/pamplejuce_LICENSE.txt](../licenses/pamplejuce_LICENSE.txt).

| Helper | Purpose | Include after |
| --- | --- | --- |
| `GitVersion.cmake` | Resolve version metadata and write the generated `VERSION` file. | Setting `CMAKE_MODULE_PATH`, before `project()`. |
| `Sanitizer.cmake` | Configure optional Address or Thread Sanitizer flags. | `project()`, before adding JUCE or other targets. |
| `JUCEDefaults.cmake` | Configure IDE source groups, the Windows runtime, and compiler diagnostics. | `project()`, before adding JUCE or other targets. |
| `SharedCodeDefaults.cmake` | Require C++20 and configure release optimization flags. | Creating `SharedCode`. |
| `Assets.cmake` | Embed all files under `assets` in the `Assets` target. | Adding JUCE. |
| `XcodePrettify.cmake` | Organize sources and plugin targets, and enable plugin Xcode schemes. | Creating plugin targets, `SharedCode`, and `Assets`, and populating `SourceFiles`. |
| `GitHubENV.cmake` | Write packaging metadata and artifact paths to `.env` when `CI` is defined. | Creating plugin targets and setting the project metadata. |

`GitVersion.cmake` sets `CURRENT_VERSION`, `MAJOR_VERSION`, and `CURRENT_HASH`.
It uses the latest reachable Git tag, strips an optional leading `v`, and reads
the current short commit hash. If Git or the corresponding metadata is unavailable,
it falls back to version `0.0.0` and hash `0000000`. The plugin receives these values
through `ZL_PLUGIN_CURRENT_VERSION` and `ZL_PLUGIN_CURRENT_HASH`.

Sanitizers are disabled by default. Enable one with
`-DWITH_ADDRESS_SANITIZER=ON` or `-DWITH_THREAD_SANITIZER=ON`; they cannot be enabled
together.

The root `CMakeLists.txt` owns the macOS deployment target, plugin formats, and
Highway configuration.
