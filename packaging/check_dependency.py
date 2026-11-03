import os
from pathlib import Path
import platform
import plistlib
import re
import shutil
import subprocess
import sys


def find_vst3_binaries(path: Path, system: str) -> list[Path]:
    if not path.exists():
        raise RuntimeError(f"VST3_PATH does not exist: {path}")
    if system == "Windows" and path.is_file() and path.suffix.lower() == ".vst3":
        return [path]  # Legacy Windows single-file VST3.
    if not path.is_dir():
        raise RuntimeError(f"Expected a VST3 bundle directory: {path}")

    contents = path / "Contents"
    if system == "Darwin":
        executable_dir = contents / "MacOS"
        plist_path = contents / "Info.plist"
        if plist_path.is_file():
            with plist_path.open("rb") as stream:
                info = plistlib.load(stream)
            if not isinstance(info, dict):
                raise RuntimeError(f"Invalid bundle dictionary in {plist_path}")
            name = info.get("CFBundleExecutable")
            if not isinstance(name, str) or not name or Path(name).name != name:
                raise RuntimeError(f"Invalid CFBundleExecutable in {plist_path}")
            binary = executable_dir / name
            if binary.resolve().parent != executable_dir.resolve():
                raise RuntimeError(f"Executable escapes bundle directory: {binary}")
            binaries = [binary] if binary.is_file() else []
        else:
            binaries = sorted(p for p in executable_dir.glob("*") if p.is_file())
    elif system == "Linux":
        binaries = sorted(p for p in contents.glob("*-linux/*.so") if p.is_file())
    elif system == "Windows":
        binaries = sorted(p for p in contents.glob("*-win/*.vst3") if p.is_file())
    else:
        raise RuntimeError(f"Unsupported platform: {system}")
    if not binaries:
        raise RuntimeError(f"No {system} VST3 executable found in {path}")
    return binaries


def run_tool(args: list[str], *, echo_output: bool = True) -> str:
    print("\n> " + " ".join(str(arg) for arg in args), flush=True)
    env = dict(os.environ, LC_ALL="C", VSLANG="1033")
    try:
        result = subprocess.run(args, capture_output=True, text=True, errors="replace", env=env)
    except OSError as error:
        raise RuntimeError(f"Unable to run {args[0]}: {error}") from error
    if echo_output or result.returncode:
        print(result.stdout, end="" if result.stdout.endswith("\n") else "\n")
    if result.stderr:
        print(result.stderr, file=sys.stderr, end="" if result.stderr.endswith("\n") else "\n")
    if result.returncode:
        raise RuntimeError(f"{args[0]} failed with exit status {result.returncode}")
    # Some loaders report missing libraries/versions on stderr despite exit zero.
    return result.stdout + ("\n" + result.stderr if result.stderr else "")


def macos_minimum_versions(output: str) -> list[tuple[str, str, str]]:
    versions = []
    arch = "unspecified"
    command = ""
    fields = {}

    def finish():
        if command == "LC_BUILD_VERSION" and fields.get("platform", "").lower() in {"1", "macos"}:
            minimum = fields.get("minos")
        elif command == "LC_VERSION_MIN_MACOSX":
            minimum = fields.get("version")
        else:
            minimum = None
        if minimum:
            versions.append((arch, minimum, fields.get("sdk", "unknown")))

    for line in output.splitlines():
        arch_match = re.search(r"\(architecture ([^)]+)\)", line)
        if arch_match or re.match(r"\s*Load command \d+", line):
            finish()
            command, fields = "", {}
            if arch_match:
                arch = arch_match.group(1)
        parts = line.split()
        if len(parts) >= 2:
            if parts[0] == "cmd":
                command = parts[1]
            elif parts[0] in {"platform", "minos", "version", "sdk"}:
                fields[parts[0]] = parts[1]
    finish()
    return versions


def linux_required_versions(output: str) -> dict[str, str]:
    """Highest numeric symbol requirements, excluding exported definitions."""
    versions = {}
    in_needs = False
    for line in output.splitlines():
        if re.match(r"\s*Version (needs|definition|symbols) section", line):
            in_needs = "Version needs section" in line
        if not in_needs:
            continue
        for family, version in re.findall(r"\bName:\s+(GLIBCXX|GLIBC|CXXABI)_(\d+(?:\.\d+)+)\b", line):
            current = versions.get(family, "0")
            if tuple(map(int, version.split("."))) > tuple(map(int, current.split("."))):
                versions[family] = version
    return versions


def inspect_macos(binary: Path):
    run_tool(["file", str(binary)])
    arches = run_tool(["lipo", "-archs", str(binary)]).strip()
    run_tool(["otool", "-arch", "all", "-L", str(binary)])
    commands = run_tool(["otool", "-arch", "all", "-l", str(binary)], echo_output=False)
    # Keep relevant load-command evidence without all segment/section tables.
    for block in re.split(r"(?=Load command \d+)", commands):
        if re.search(r"\bcmd (LC_BUILD_VERSION|LC_VERSION_MIN_MACOSX|LC_RPATH)\b", block):
            print(block.rstrip())
    versions = macos_minimum_versions(commands)
    if not versions:
        raise RuntimeError(f"No macOS deployment minimum found in {binary}")
    print("\nRecorded macOS deployment requirements:")
    for arch, minimum, sdk in versions:
        print(f"  {arches if arch == 'unspecified' else arch}: macOS {minimum} or later; build SDK {sdk}")
    print("The SDK and dylib compatibility/current versions are not minimum OS versions.")
    print("These are recorded deployment minima; dependency/API availability and DAW support still need target-OS testing.")


def inspect_linux(binary: Path):
    run_tool(["file", str(binary)])
    run_tool(["readelf", "--wide", "--file-header", "--dynamic", "--notes", str(binary)])
    versions = run_tool(["readelf", "--wide", "--version-info", str(binary)])
    requirements = linux_required_versions(versions)
    print("\nHighest directly required runtime symbol versions:")
    for family in ("GLIBC", "GLIBCXX", "CXXABI"):
        print(f"  {family}: {requirements.get(family, 'no numeric requirement recorded')}")
    print("Full version-needs entries above also cover nonnumeric requirements and other libraries.")
    print("These requirements do not identify an exact Linux distribution or minimum kernel.")
    print("ELF ABI/kernel notes, if present, are metadata rather than a tested kernel support guarantee.")
    print("\nDependency resolution on this build machine:")
    resolution = run_tool(["ldd", str(binary)])
    if re.search(r"\bnot found\b", resolution):
        raise RuntimeError(f"Unresolved runtime dependencies in {binary}")
    print("Resolution here does not prove compatibility on older systems; indirect and dynamically loaded libraries also matter.")


def find_dumpbin() -> str:
    available = shutil.which("dumpbin")
    if available:
        return available
    program_files = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    vswhere = Path(program_files) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    host = "arm64" if platform.machine().lower() in {"arm64", "aarch64"} else "x64"
    matches = run_tool([
        str(vswhere), "-products", "*", "-find",
        rf"VC\Tools\MSVC\**\bin\Host{host}\{host}\dumpbin.exe",
    ])
    candidates = [line.strip() for line in matches.splitlines() if Path(line.strip()).is_file()]
    if not candidates:
        raise RuntimeError("dumpbin was not found; install the Visual Studio C++ build tools or run from a developer prompt")
    return sorted(candidates)[-1]


def inspect_windows(binary: Path):
    dumpbin = find_dumpbin()
    run_tool([dumpbin, "/nologo", "/headers", str(binary)])
    run_tool([dumpbin, "/nologo", "/dependents", str(binary)])
    run_tool([dumpbin, "/nologo", "/imports", str(binary)])
    print("\nWindows compatibility: architecture and OS/subsystem versions above are PE header declarations.")
    print("They do not establish the oldest compatible Windows release. Imported APIs (including delay imports),")
    print("runtime-loaded DLLs, and the DAW may require newer Windows. Validate on the oldest supported release.")


def main() -> int:
    try:
        value = os.environ.get("VST3_PATH", "").strip()
        if not value:
            raise RuntimeError("VST3_PATH must name the built VST3 bundle")
        system = platform.system()
        binaries = find_vst3_binaries(Path(value).resolve(), system)
        print(f"VST3 dependency and compatibility report: {value}")
        print(f"Inspection machine: {system} {platform.release()} ({platform.machine()}); not a target OS requirement")
        for binary in binaries:
            print(f"\nInspecting VST3 module: {binary}")
            {"Darwin": inspect_macos, "Linux": inspect_linux, "Windows": inspect_windows}[system](binary)
        print("\nVST3 inspection completed successfully (metadata inspection, not OS/DAW certification).")
        return 0
    except (RuntimeError, OSError, ValueError, plistlib.InvalidFileException) as error:
        print(f"VST3 inspection failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
