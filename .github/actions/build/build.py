"""Execute the composite action's CMake build without shell-parsing its inputs.

Shared defaults live in action.yml. Run CMake from the selected source directory
so project metadata and relative build/artifact paths retain their usual meaning.
"""

from __future__ import annotations

import json
import os
from pathlib import Path
import shlex
import subprocess
import sys


def configure_command(settings: dict[str, str], runner_os: str) -> list[str]:
    for key in ("formats", "simd", "build-type", "build-dir"):
        if not settings.get(key):
            raise ValueError(f"The build action requires a nonempty {key} input")
    command = [
        "cmake", "-S", ".", "-B", settings["build-dir"], "-G", "Ninja",
        f"-DCMAKE_BUILD_TYPE={settings['build-type']}",
    ]
    if runner_os == "Linux":
        version = settings["clang-version"]
        command += [f"-DCMAKE_C_COMPILER=clang-{version}", f"-DCMAKE_CXX_COMPILER=clang++-{version}"]
    elif runner_os == "Windows":
        command += ["-DCMAKE_C_COMPILER=clang-cl", "-DCMAKE_CXX_COMPILER=clang-cl"]
    elif runner_os == "macOS":
        target = settings["macos-deployment-target"]
        if not target:
            raise ValueError("macos-deployment-target must not be empty")
        command.append(f"-DCMAKE_OSX_DEPLOYMENT_TARGET={target}")
        if settings.get("architecture"):
            command.append(f"-DCMAKE_OSX_ARCHITECTURES={settings['architecture']}")
    else:
        raise ValueError(f"Unsupported runner OS: {runner_os}")

    command += [f"-DZL_JUCE_FORMATS={settings['formats']}", f"-DZL_HWY_STATIC_TARGET={settings['simd']}"]
    sanitizer = settings["sanitizer"]
    if sanitizer in {"none", "address", "thread"}:
        command += [
            f"-DWITH_ADDRESS_SANITIZER={'ON' if sanitizer == 'address' else 'OFF'}",
            f"-DWITH_THREAD_SANITIZER={'ON' if sanitizer == 'thread' else 'OFF'}",
        ]
    else:
        raise ValueError(f"Unknown sanitizer: {sanitizer}")
    command += [line.strip() for line in settings.get("cmake-extra-args", "").splitlines() if line.strip()]
    return command


def build_command(settings: dict[str, str]) -> list[str]:
    return ["cmake", "--build", settings["build-dir"], "--config", settings["build-type"]]


def run_build(settings: dict[str, str], runner_os: str, workspace: Path, github_env: Path | None) -> None:
    source = (workspace / settings["source-dir"]).resolve()
    if not source.is_dir():
        raise ValueError(f"Source directory does not exist: {source}")
    import_env = settings["import-env"]
    if import_env not in {"true", "false"}:
        raise ValueError("import-env must be true or false")
    if import_env == "true" and github_env is None:
        raise ValueError("GITHUB_ENV is required when import-env is true")

    environment = dict(os.environ, BUILD_TYPE=settings["build-type"], BUILD_DIR=settings["build-dir"])
    for command in (configure_command(settings, runner_os), build_command(settings)):
        print(f"[{source}] {shlex.join(command)}", flush=True)
        subprocess.run(command, cwd=source, env=environment, check=True)

    if import_env == "true":
        # Never import pluginval's .env over the main plugin's artifact paths.
        # Reading the file as data also preserves spaces in names without shell evaluation.
        artifact_env = (source / ".env").read_text(encoding="utf-8")
        print(artifact_env, end="" if artifact_env.endswith("\n") else "\n", flush=True)
        with github_env.open("a", encoding="utf-8") as output:
            output.write(f"BUILD_TYPE={settings['build-type']}\nBUILD_DIR={settings['build-dir']}\n")
            output.write(artifact_env)
            if not artifact_env.endswith("\n"):
                output.write("\n")


def main() -> int:
    try:
        settings = json.loads(os.environ["ZL_BUILD_SETTINGS"])
        env_path = os.environ.get("GITHUB_ENV")
        run_build(settings, os.environ["RUNNER_OS"], Path(os.environ["GITHUB_WORKSPACE"]),
                  Path(env_path) if env_path else None)
        return 0
    except (KeyError, ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f"Build failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
