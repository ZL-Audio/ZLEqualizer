import os
import sys
import subprocess
from pathlib import PureWindowsPath


def main():
    temp_dir = "./windowstmp"
    subprocess.run(["mkdir", temp_dir])

    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    bundle_id = os.getenv("BUNDLE_ID", "")
    build_dir = os.getenv("BUILD_DIR", "")
    artifact_name = os.getenv("ARTIFACT_NAME", "")

    outfile = open("packaging/installer.iss", "w")

    outfile.write(r'''
#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define ProjectName GetEnv('PROJECT_NAME')
#define ProductName GetEnv('PRODUCT_NAME')
#define Publisher GetEnv('COMPANY_NAME')
#define ArtifactName GetEnv('ARTIFACT_NAME')
#define Year GetDateTimeString("yyyy","","")

[Setup]
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
AppName={#ProductName}
OutputBaseFilename={#ArtifactName}
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#ProductName}.vst3"
DisableDirPage=yes
CreateAppDir=no
''')

    if os.path.isfile("/packaging/icon.ico"):
        outfile.write(r'SetupIconFile=..\packaging\icon.ico')
    outfile.write("UninstallDisplayIcon={uninstallexe}")
    if os.path.isfile("/packaging/EULA"):
        outfile.write(r'LicenseFile="EULA"')
    if os.path.isfile("/packaging/Readme.rtf"):
        outfile.write(r'InfoBeforeFile="Readme.rtf"')

    outfile.write('''
[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom
''')

    all_install_paths = [
        r'{commoncf64}\VST3',
        r'{commoncf64}\LV2',
        r'{commoncf64}\Avid\Audio\Plug-Ins',
        r'{commonpf64}\ZLAudio']

    plugin_formats, extensions, install_paths = [], [], []
    for plugin_format, extension, install_path in zip(
            ["VST3", "LV2", "AAX", "Standalone"],
            ["vst3", "lv2", "aaxplugin", "standalone"],
            all_install_paths):
        if plugin_format + "_PATH" in os.environ:
            plugin_path = os.environ[plugin_format + "_PATH"]
            if os.path.exists(plugin_path):
                plugin_formats.append(plugin_format)
                extensions.append(extension)
                install_paths.append(install_path)

    outfile.write("[Components]\n")
    for plugin_format, extension, install_path in zip(plugin_formats, extensions, install_paths):
        outfile.write(
            'Name: "{}"; Description: {} {}; Types: full custom; Flags: checkablealone'.format(
                extension, product_name, plugin_format
            ))
        outfile.write("\n")

    outfile.write("[UninstallDelete]\n")
    for plugin_format, extension, install_path in zip(plugin_formats, extensions, install_paths):
        outfile.write(
            'Type: filesandordirs; Name: "{}"'.format(
                install_path + '\\' + product_name + 'Data'
            ))
        outfile.write("\n")

    outfile.write("[Files]\n")
    for plugin_format, extension, install_path in zip(plugin_formats, extensions, install_paths):
        if plugin_format == "Standalone":
            outfile.write(
                'Source: "..\\{}"; DestDir: "{}"; Excludes: *.ilk; Flags: ignoreversion; Components: {}'.format(
                    str(PureWindowsPath(os.environ[plugin_format + "_PATH"])),
                    install_path + '\\' + product_name,
                    extension)
            )
        else:
            outfile.write(
                'Source: "..\\{}\\*"; DestDir: "{}"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: {}'.format(
                    str(PureWindowsPath(os.environ[plugin_format + "_PATH"])),
                    install_path + '\\' + product_name + "." + extension + "\\",
                    extension)
                )
        outfile.write("\n")

    outfile.write("[Run]\n")
    for plugin_format, extension, install_path in zip(plugin_formats, extensions, install_paths):
        outfile.write('Filename: "{cmd}"; ')
        outfile.write('WorkingDir:"{}"; '.format(install_path))
        outfile.write('Parameters: "/C mklink /D ""{}"" ""{}"""; '
                      .format(install_path + '\\' + product_name + 'Data',
                              '{commonappdata}\\{#ProductName}'))
        outfile.write("Flags: runascurrentuser; ")
        outfile.write("Components: " + extension)
        outfile.write("\n")

    outfile.close()

if __name__ == '__main__':
    sys.exit(main())
