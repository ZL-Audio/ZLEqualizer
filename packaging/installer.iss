#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define ProjectName GetEnv('PROJECT_NAME')
#define ProductName GetEnv('PRODUCT_NAME')
#define Publisher GetEnv('COMPANY_NAME')
#define Year GetDateTimeString("yyyy","","")

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#ProductName}
OutputBaseFilename={#ProductName}-{#Version}-Windows
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#ProductName}.vst3"
DisableDirPage=yes
CreateAppDir=no
SetupIconFile=..\packaging\icon.ico
UninstallDisplayIcon={uninstallexe}

; MAKE SURE YOU READ THE FOLLOWING!
; LicenseFile="EULA"
InfoBeforeFile="Readme.rtf"
UninstallFilesDir="{commonappdata}\{#ProductName}\uninstall"

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3"; Description: {#ProductName} VST3; Types: full custom; Flags: checkablealone
Name: "lv2"; Description: {#ProductName} LV2; Types: full custom; Flags: checkablealone

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#ProductName}Data"
Type: filesandordirs; Name: "{commoncf64}\LV2\{#ProductName}Data"

; MSVC adds a .ilk when building the plugin. Let's not include that.
[Files]
Source: "..\Builds\{#ProjectName}_artefacts\Release\VST3\{#ProductName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#ProductName}.vst3\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: vst3

Source: "..\Builds\{#ProjectName}_artefacts\Release\LV2\{#ProductName}.lv2\*"; DestDir: "{commoncf64}\LV2\{#ProductName}.lv2\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: lv2

[Run]
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\VST3"; \
    Parameters: "/C mklink /D ""{commoncf64}\VST3\{#ProductName}Data"" ""{commonappdata}\{#ProductName}"""; \
    Flags: runascurrentuser; Components: vst3

Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\LV2"; \
    Parameters: "/C mklink /D ""{commoncf64}\LV2\{#ProductName}Data"" ""{commonappdata}\{#ProductName}"""; \
    Flags: runascurrentuser; Components: lv2