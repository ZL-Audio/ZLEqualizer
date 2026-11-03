import os
import re
import sys
import xml.etree.cElementTree as ET
from pathlib import Path
import shutil
import subprocess
import tempfile


def require_package(path, command):
    if not path.is_file() or path.stat().st_size == 0:
        raise ValueError(f"{command} did not create a nonempty package: {path}")


def attach_icon(package_path):
    icon_path = Path("packaging/installer.icns")
    if not icon_path.is_file():
        return

    print("Attach icns")
    try:
        with tempfile.TemporaryDirectory(prefix="zl-plugin-package-icon-") as temporary_dir:
            temporary_path = Path(temporary_dir)
            shutil.copyfile(icon_path, temporary_path / "installer.icns")
            resource_path = temporary_path / "icon.rsrc"
            resource_path.write_text('read \'icns\' (-16455) "installer.icns";\n', encoding="utf-8")
            subprocess.run(
                ["Rez", "-append", "icon.rsrc", "-o", str(package_path.resolve())],
                cwd=temporary_dir, check=True)
        subprocess.run(["SetFile", "-a", "C", str(package_path)], check=True)
    except (OSError, subprocess.CalledProcessError) as error:
        # Decoration is optional; the package has already been built successfully.
        print(f"Warning: could not attach installer icon: {error}", file=sys.stderr)


def build_package():

    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    bundle_id = os.getenv("BUNDLE_ID", "")
    build_dir = os.getenv("BUILD_DIR", "")
    artifact_name = os.getenv("ARTIFACT_NAME", "")
    developer_id_app = os.getenv("DEVELOPER_ID_APPLICATION", "")
    macos_arch = os.getenv("MACOS_ARCH", "x86_64,arm64")
    macos_deployment_target = os.getenv("MACOS_DEPLOYMENT_TARGET", "").strip()

    if not build_dir.strip():
        raise ValueError("BUILD_DIR must be set and nonempty")
    if not artifact_name.strip():
        raise ValueError("ARTIFACT_NAME must be set and nonempty")
    if not re.fullmatch(r"[1-9][0-9]*(?:\.[0-9]+){0,2}", macos_deployment_target):
        raise ValueError("MACOS_DEPLOYMENT_TARGET must be a numeric macOS version matching "
                         "the deployment target used to build the packaged artifacts")
    build_path = Path(build_dir)
    if not build_path.is_dir():
        raise NotADirectoryError(f"BUILD_DIR is not an existing directory: {build_path}")

    formats = [
        ("VST3", "vst3", "/Library/Audio/Plug-Ins/VST3"),
        ("AU", "au", "/Library/Audio/Plug-Ins/Components"),
        ("LV2", "lv2", "/Library/Audio/Plug-Ins/LV2"),
        ("CLAP", "clap", "/Library/Audio/Plug-Ins/CLAP"),
        ("AAX", "aax", "/Library/Application Support/Avid/Audio/Plug-Ins"),
        ("Standalone", "standalone", "/Applications"),
    ]
    artifacts = []
    for plugin_format, extension, install_path in formats:
        env_var = plugin_format + "_PATH"
        if env_var not in os.environ:
            continue
        plugin_path = os.environ[env_var]
        if not plugin_path.strip():
            raise ValueError(f"{env_var} is set but empty")
        if not Path(plugin_path).exists():
            raise FileNotFoundError(f"{env_var} does not exist: {plugin_path}")
        artifacts.append((plugin_format, extension, install_path, plugin_path))
    if not artifacts:
        raise ValueError("No plugin artifacts were provided. Set at least one supported *_PATH variable.")

    standalone_entitlements = None
    if developer_id_app and any(artifact[0] == "Standalone" for artifact in artifacts):
        default_entitlements = (build_path / f"{project_name}_artefacts" / "JuceLibraryCode"
                                / f"{project_name}_Standalone.entitlements")
        standalone_entitlements = Path(os.getenv("Standalone_ENTITLEMENTS_PATH", str(default_entitlements)))
        if not standalone_entitlements.is_file():
            raise FileNotFoundError(f"Standalone entitlements file does not exist: {standalone_entitlements}")

    # root
    root = ET.Element("installer-gui-script", minSpecVersion="1")
    # title
    title = ET.SubElement(root, "title")
    title.text = "{} {}".format(product_name, version)
    # EULA
    if os.path.isfile("packaging/EULA"):
        eula = ET.SubElement(root, "license", file="EULA")
        eula.set("mime-type", "text/plain")
    # readme
    if os.path.isfile("packaging/Readme.rtf"):
        readme = ET.SubElement(root, "readme", file="Readme.rtf")
    # options
    options = ET.SubElement(root, "options",
                            customize="always",
                            rootVolumeOnly="true",
                            hostArchitectures=macos_arch)
    # domain
    domain = ET.SubElement(root, "domain",
                           enable_anywhere="false",
                           enable_currentUserHome="false",
                           enable_localSystem="true")
    volume_check = ET.SubElement(root, "volume-check", script="true")
    allowed_os_versions = ET.SubElement(volume_check, "allowed-os-versions")
    ET.SubElement(allowed_os_versions, "os-version", min=macos_deployment_target)
    # choices outline
    outline = ET.SubElement(root, "choices-outline")

    print("Create packages")
    for plugin_format, extension, install_path, plugin_path in artifacts:
        if developer_id_app != "" and extension != "aax":
            signing_command = ["codesign", "--force", "-s", developer_id_app, "-v", plugin_path,
                               "--strict", "--options=runtime", "--timestamp"]
            if plugin_format == "Standalone":
                # Apply application entitlements only to the standalone executable.
                signing_command.extend(["--entitlements", str(standalone_entitlements)])
            else:
                signing_command.append("--deep")
            subprocess.run(signing_command, check=True)

        identifier = "{}.{}.{}.pkg".format(bundle_id, project_name, extension)
        pkg_path = build_path / "{}.{}.pkg".format(product_name, extension).replace(" ", "_")

        subprocess.run(
            ["pkgbuild", "--identifier", identifier, "--version", version,
             "--component", plugin_path, "--install-location", install_path, str(pkg_path)],
            check=True)
        require_package(pkg_path, "pkgbuild")
        ref = ET.SubElement(root, "pkg-ref",
                            id=identifier, version=version, onConclusion="none")
        ref.text = pkg_path.name
        choice = ET.SubElement(root, "choice", id=identifier,
                               visible="true", start_selected="true",
                               title="{} {}".format(product_name, plugin_format))
        ET.SubElement(choice, "pkg-ref", id=identifier)
        ET.SubElement(outline, "line", choice=identifier)

    # write xml
    print("")
    print("Create distribution xml")
    ET.indent(root, space="\t", level=0)
    tree = ET.ElementTree(root)
    tree.write("packaging/distribution.xml", encoding="utf-8", xml_declaration=True)

    print(ET.tostring(root, encoding='utf8').decode())

    print("")
    print("Create final package")
    command_list = ["--distribution", "packaging/distribution.xml",
                    "--package-path", build_dir,
                    "--resources", "packaging",
                    artifact_name + "_unsigned.pkg"]

    subprocess.run(["productbuild"] + command_list, check=True)
    package_path = Path(artifact_name + "_unsigned.pkg")
    require_package(package_path, "productbuild")

    attach_icon(package_path)
    print("")


def main():
    try:
        build_package()
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"Packaging failed: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
