import os
import sys
import xml
import xml.etree.cElementTree as ET
import subprocess


def main():
    temp_dir = "./appletmp"
    subprocess.run(["mkdir", temp_dir])

    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    bundle_id = os.getenv("BUNDLE_ID", "")
    build_dir = os.getenv("BUILD_DIR", "")
    artifact_name = os.getenv("ARTIFACT_NAME", "")
    developer_id_app = os.getenv("DEVELOPER_ID_APPLICATION", "")
    macos_arch = os.getenv("MACOS_ARCH", "x86_64,arm64")

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
    # choices outline
    outline = ET.SubElement(root, "choices-outline")

    install_paths = [
        "/Library/Audio/Plug-Ins/VST3",
        "/Library/Audio/Plug-Ins/Components",
        "/Library/Audio/Plug-Ins/LV2",
        "/Library/Audio/Plug-Ins/CLAP",
        "/Library/Application Support/Avid/Audio/Plug-Ins",
        "/Applications"
        ]

    print("Create packages")
    for plugin_format, extension, install_path in zip(
            ["VST3", "AU", "LV2", "CLAP", "AAX", "Standalone"],
            ["vst3", "au", "lv2", "clap", "aax", "standalone"],
            install_paths):
        if plugin_format + "_PATH" in os.environ:
            plugin_path = os.environ[plugin_format + "_PATH"]
            if os.path.exists(plugin_path):
                if developer_id_app != "" and extension != "aax":
                    subprocess.run(
                        'codesign --force -s "{}" -v "{}" --deep --strict --options=runtime --timestamp'.format(
                            developer_id_app, plugin_path), shell=True)

                identifier = "{}.{}.{}.pkg".format(bundle_id, project_name, extension)
                pkg_path = "{}/{}.{}.pkg".format(build_dir, product_name, extension).replace(" ", "_")

                subprocess.run(
                    'pkgbuild --identifier "{}" --version {} --component "{}" --install-location "{}" "{}"'.format(
                        identifier, version, plugin_path, install_path, pkg_path), shell=True)
                ref = ET.SubElement(root, "pkg-ref",
                                    id=identifier, version=version, onConclusion="none")
                ref.text = pkg_path
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

    subprocess.run(["productbuild"] + command_list)

    if os.path.exists("packaging/icon.icns"):
        print("")
        print("Attach icns")
        subprocess.run(["mv", "packaging/icon.icns", "."])
        with open("tmpIcon.rsrc", "w") as outfile:
            subprocess.run(["echo", "read 'icns' (-16455) \"icon.icns\";"], stdout=outfile)
        print(subprocess.run(["Rez -append tmpIcon.rsrc -o \"{}_unsigned.pkg\"".format(artifact_name)], shell=True))
        print(subprocess.run(["SetFile -a C \"{}_unsigned.pkg\"".format(artifact_name)], shell=True))
    print("")
    return 0


if __name__ == '__main__':
    sys.exit(main())
