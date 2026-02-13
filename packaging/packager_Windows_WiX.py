import os
import sys
import uuid
import hashlib

NAMESPACE_GUID = uuid.UUID('12345678-1234-5678-1234-567812345678')

def get_guid(string_input):
    return str(uuid.uuid5(NAMESPACE_GUID, string_input)).upper()

def get_wix_id(string_input):
    hash_object = hashlib.md5(string_input.encode('utf-8'))
    return "ID_" + hash_object.hexdigest().upper()

def escape_xml(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")

def main():
    temp_dir = "./windowstmp"
    os.makedirs(temp_dir, exist_ok=True)
    
    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    publisher = os.getenv("COMPANY_NAME", "MyCompany")
    
    outfile_path = "packaging/installer.wxs"
    overrides_path = "packaging/overrides.wxl"
    
    os.makedirs(os.path.dirname(outfile_path), exist_ok=True)
    f = open(outfile_path, "w", encoding="utf-8")

    f.write('<Wix xmlns="http://wixtoolset.org/schemas/v4/wxs" xmlns:ui="http://wixtoolset.org/schemas/v4/wxs/ui">\n')
    
    upgrade_code = get_guid(f"{project_name}_UpgradeCode") 

    f.write(f'    <Package Name="{escape_xml(product_name)}" Manufacturer="{escape_xml(publisher)}" Version="{version}" UpgradeCode="{upgrade_code}" Scope="perMachine" Compressed="yes">\n')
    
    # --- Downgrade Logic ---
    # AllowDowngrades="yes" permits the installer to overwrite a newer version with this older one without erroring.
    f.write(f'        <MajorUpgrade AllowDowngrades="yes" Schedule="afterInstallInitialize" />\n')

    f.write(f'        <MediaTemplate EmbedCab="yes" />\n')

    icon_path = "packaging/icon.ico"
    if os.path.exists(icon_path):
        f.write(f'        <Icon Id="AppIcon.ico" SourceFile="{icon_path}" />\n')
        f.write('        <Property Id="ARPPRODUCTICON" Value="AppIcon.ico" />\n')

    # --- Directory Structure ---
    # Public IDs (uppercase) allow them to be modified by the UI
    company_dir_id = "COMPANYDIR"
    
    f.write('        <StandardDirectory Id="CommonFiles64Folder">\n')
    f.write('            <Directory Id="VST3DIR" Name="VST3" />\n')
    f.write('            <Directory Id="CLAPDIR" Name="CLAP" />\n')
    f.write('            <Directory Id="LV2DIR" Name="LV2" />\n')
    f.write('            <Directory Id="AvidDir" Name="Avid">\n')
    f.write('                <Directory Id="AudioDir" Name="Audio">\n')
    f.write('                    <Directory Id="AAXDIR" Name="Plug-Ins" />\n')
    f.write('                </Directory>\n')
    f.write('            </Directory>\n')
    f.write('        </StandardDirectory>\n')
    f.write('        <StandardDirectory Id="ProgramFiles64Folder">\n')
    f.write(f'            <Directory Id="{company_dir_id}" Name="ZLAudio" />\n') 
    f.write('        </StandardDirectory>\n') 

    # --- Harvest Logic ---
    features = {} 
    # Tuple: (FormatName, Extension, DirectoryID, IsBundle)
    formats = [("VST3", "vst3", "VST3DIR", True), ("CLAP", "clap", "CLAPDIR", True), ("AAX", "aaxplugin", "AAXDIR", True), ("LV2", "lv2", "LV2DIR", True), ("Standalone", "exe", company_dir_id, False)]

    for fmt_name, ext, parent_dir_id, is_bundle_config in formats:
        env_var = f"{fmt_name}_PATH"
        if env_var not in os.environ: continue
        source_path = os.environ[env_var]
        if not os.path.exists(source_path): continue

        feature_id = f"Feature_{fmt_name}"
        # Store the parent_dir_id (e.g., VST3DIR) to use as the ConfigurableDirectory
        features[feature_id] = {"title": f"{fmt_name}", "components": [], "config_dir": parent_dir_id}

        f.write(f'        <DirectoryRef Id="{parent_dir_id}">\n')
        is_actually_file = os.path.isfile(source_path)
        use_bundle_logic = is_bundle_config and not is_actually_file

        if use_bundle_logic:
            bundle_dir_name = f"{product_name}.{ext}"
            bundle_dir_id = get_wix_id(f"DIR_{fmt_name}_BUNDLE")
            f.write(f'            <Directory Id="{bundle_dir_id}" Name="{bundle_dir_name}">\n')
            write_dir_recursive(f, source_path, bundle_dir_id, features[feature_id]["components"], fmt_name)
            f.write('            </Directory>\n')
        else:
            if is_actually_file:
                comp_id = get_wix_id(f"COMP_{fmt_name}_FILE")
                file_id = get_wix_id(f"FILE_{fmt_name}_FILE")
                features[feature_id]["components"].append(comp_id)
                target_name = f"{product_name}.{ext}"
                f.write(f'            <Component Id="{comp_id}" Guid="{get_guid(comp_id)}">\n')
                f.write(f'                <File Id="{file_id}" Source="{source_path}" Name="{target_name}" KeyPath="yes" />\n')
                f.write('            </Component>\n')
            else:
                write_dir_recursive(f, source_path, parent_dir_id, features[feature_id]["components"], fmt_name)
        f.write('        </DirectoryRef>\n')

    # --- Feature Definition ---
    # The Root feature does not have a ConfigurableDirectory. 
    # This forces the user to select the sub-features to change their specific paths.
    f.write(f'        <Feature Id="Complete" Title="Complete Installation" Display="expand" Level="1">\n')
    
    for feat_id, data in features.items():
        if not data["components"]: continue 
        
        # ConfigurableDirectory:
        # When this feature is highlighted in the Feature Tree, the "Browse" button controls this directory.
        f.write(f'            <Feature Id="{feat_id}" Title="{data["title"]}" Level="1" ConfigurableDirectory="{data["config_dir"]}">\n')
        for comp_id in data["components"]:
            f.write(f'                <ComponentRef Id="{comp_id}" />\n')
        f.write(f'            </Feature>\n')
    f.write('        </Feature>\n')

    # --- UI Setup ---
    eula_path = "packaging/EULA.rtf"
    readme_path = "packaging/Readme.rtf"
    license_file = None
    if os.path.exists(eula_path): license_file = eula_path
    elif os.path.exists(readme_path): license_file = readme_path
    else:
        license_file = os.path.join(temp_dir, "GenericLicense.rtf")
        with open(license_file, "w") as lf:
            lf.write(r"{\rtf1\ansi No EULA provided.\par}")
    f.write(f'        <WixVariable Id="WixUILicenseRtf" Value="{license_file}" />\n')

    banner_bmp = "packaging/banner.bmp"
    if os.path.exists(banner_bmp):
         f.write(f'        <WixVariable Id="WixUIBannerBmp" Value="{banner_bmp}" />\n')
    dialog_bmp = "packaging/dialog.bmp"
    if os.path.exists(dialog_bmp):
         f.write(f'        <WixVariable Id="WixUIDialogBmp" Value="{dialog_bmp}" />\n')

    # WixUI_FeatureTree is the native UI set that enables the "Customize" dialog (Feature Tree).
    f.write('        <UI>\n')
    f.write('            <ui:WixUI Id="WixUI_FeatureTree" />\n')
    f.write('            <UIRef Id="WixUI_ErrorProgressText" />\n')
    f.write('        </UI>\n')

    f.write('    </Package>\n')
    f.write('</Wix>\n')
    f.close()

    with open(overrides_path, "w", encoding="utf-8") as wxl:
        wxl.write('<?xml version="1.0" encoding="utf-8"?>\n')
        wxl.write('<WixLocalization Culture="en-us" xmlns="http://wixtoolset.org/schemas/v4/wxl">\n')
        wxl.write(f'    <String Id="WelcomeDlgTitle" Value="{{\\WixUI_Font_Title}}Welcome to the {escape_xml(product_name)} Installer" />\n')
        wxl.write(f'    <String Id="WelcomeDlgDescription" Value="The installer will guide you through the steps required to install {escape_xml(product_name)} on your computer." />\n')
        wxl.write('</WixLocalization>\n')
        
    print(f"Generated {outfile_path}")

def write_dir_recursive(file_handle, current_os_path, parent_wix_id, component_list, prefix):
    try:
        items = os.listdir(current_os_path)
    except OSError: return
    files = [i for i in items if os.path.isfile(os.path.join(current_os_path, i))]
    dirs = [i for i in items if os.path.isdir(os.path.join(current_os_path, i))]
    files = [f for f in files if not f.endswith(".ilk")]

    for filename in files:
        full_path = os.path.join(current_os_path, filename)
        file_id = get_wix_id(f"FILE_{prefix}_{full_path}")
        comp_id = get_wix_id(f"COMP_{prefix}_{full_path}")
        component_list.append(comp_id)
        file_handle.write(f'                <Component Id="{comp_id}" Guid="{get_guid(comp_id)}">\n')
        file_handle.write(f'                    <File Id="{file_id}" Source="{full_path}" KeyPath="yes" />\n')
        file_handle.write(f'                </Component>\n')

    for dirname in dirs:
        full_path = os.path.join(current_os_path, dirname)
        dir_id = get_wix_id(f"DIR_{prefix}_{full_path}")
        clean_dirname = escape_xml(dirname)
        file_handle.write(f'                <Directory Id="{dir_id}" Name="{clean_dirname}">\n')
        write_dir_recursive(file_handle, full_path, dir_id, component_list, prefix)
        file_handle.write(f'                </Directory>\n')

if __name__ == '__main__':
    main()