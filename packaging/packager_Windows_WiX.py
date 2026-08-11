import hashlib
import os
import uuid
from contextlib import contextmanager

NAMESPACE_GUID = uuid.UUID("12345678-1234-5678-1234-567812345678")
SUPPORTED_WIX_ARCHES = {"x64", "arm64"}


@contextmanager
def atomic_text_writer(path):
    temporary_path = f"{path}.tmp"
    try:
        with open(temporary_path, "w", encoding="utf-8") as output:
            yield output
        os.replace(temporary_path, path)
    except Exception:
        try:
            os.unlink(temporary_path)
        except FileNotFoundError:
            pass
        raise


def get_guid(string_input):
    return str(uuid.uuid5(NAMESPACE_GUID, string_input)).upper()


def get_wix_id(string_input):
    hash_object = hashlib.md5(string_input.encode("utf-8"))
    return "ID_" + hash_object.hexdigest().upper()


def escape_xml(value):
    return (
        str(value)
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace('"', "&quot;")
    )


def normalize_target_path(parts):
    """Return a stable Windows-path identity, independent of the build path."""
    return "/".join(str(part).replace("\\", "/").strip("/") for part in parts).casefold()


def make_component_identity(upgrade_code, wix_arch, fmt_name, parent_dir_id, target_parts):
    target_path = normalize_target_path(target_parts)
    return f"{upgrade_code}|{wix_arch}|{fmt_name}|{parent_dir_id}|{target_path}"


def write_file_component(
        file_handle,
        source_path,
        target_parts,
        component_list,
        upgrade_code,
        wix_arch,
        fmt_name,
        parent_dir_id,
        indent,
        target_name=None,
):
    identity = make_component_identity(
        upgrade_code, wix_arch, fmt_name, parent_dir_id, target_parts
    )
    comp_id = get_wix_id(f"COMPONENT|{identity}")
    file_id = get_wix_id(f"FILE|{identity}")
    component_guid = get_guid(f"COMPONENT|{identity}")
    component_list.append(comp_id)

    name_attribute = ""
    if target_name is not None:
        name_attribute = f' Name="{escape_xml(target_name)}"'

    file_handle.write(f'{indent}<Component Id="{comp_id}" Guid="{component_guid}">\n')
    file_handle.write(
        f'{indent}    <File Id="{file_id}" Source="{escape_xml(source_path)}"'
        f'{name_attribute} KeyPath="yes" />\n'
    )
    file_handle.write(f"{indent}</Component>\n")


def write_dir_recursive(
        file_handle,
        current_os_path,
        target_parts,
        component_list,
        upgrade_code,
        wix_arch,
        fmt_name,
        parent_dir_id,
        indent,
):
    items = sorted(os.listdir(current_os_path), key=str.casefold)
    files = [item for item in items if os.path.isfile(os.path.join(current_os_path, item))]
    dirs = [item for item in items if os.path.isdir(os.path.join(current_os_path, item))]
    files = [filename for filename in files if not filename.lower().endswith(".ilk")]

    for filename in files:
        full_path = os.path.join(current_os_path, filename)
        write_file_component(
            file_handle,
            full_path,
            (*target_parts, filename),
            component_list,
            upgrade_code,
            wix_arch,
            fmt_name,
            parent_dir_id,
            indent,
        )

    for dirname in dirs:
        full_path = os.path.join(current_os_path, dirname)
        child_target_parts = (*target_parts, dirname)
        directory_identity = make_component_identity(
            upgrade_code, wix_arch, fmt_name, parent_dir_id, child_target_parts
        )
        dir_id = get_wix_id(f"DIRECTORY|{directory_identity}")
        file_handle.write(
            f'{indent}<Directory Id="{dir_id}" Name="{escape_xml(dirname)}">\n'
        )
        write_dir_recursive(
            file_handle,
            full_path,
            child_target_parts,
            component_list,
            upgrade_code,
            wix_arch,
            fmt_name,
            parent_dir_id,
            indent + "    ",
        )
        file_handle.write(f"{indent}</Directory>\n")


def main():
    temp_dir = "./windowstmp"
    os.makedirs(temp_dir, exist_ok=True)

    project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "Pamplejuce Demo")
    version = os.getenv("VERSION", "0.0.0")
    publisher = os.getenv("COMPANY_NAME", "MyCompany")
    wix_arch = os.getenv("WIX_ARCH", "x64").strip().lower()
    if wix_arch not in SUPPORTED_WIX_ARCHES:
        raise ValueError(
            f"WIX_ARCH must be one of {sorted(SUPPORTED_WIX_ARCHES)}, got {wix_arch!r}"
        )

    outfile_path = "packaging/installer.wxs"
    os.makedirs(os.path.dirname(outfile_path), exist_ok=True)

    upgrade_code = get_guid(f"{project_name}_UpgradeCode")
    features = {}

    with atomic_text_writer(outfile_path) as output:
        output.write(
            '<Wix xmlns="http://wixtoolset.org/schemas/v4/wxs" '
            'xmlns:ui="http://wixtoolset.org/schemas/v4/wxs/ui">\n'
        )
        output.write(
            f'    <Package Name="{escape_xml(product_name)}" '
            f'Manufacturer="{escape_xml(publisher)}" Version="{escape_xml(version)}" '
            f'UpgradeCode="{upgrade_code}" Scope="perMachine" Compressed="yes">\n'
        )

        output.write(
            '        <MajorUpgrade AllowDowngrades="yes" '
            'Schedule="afterInstallInitialize" />\n'
        )

        output.write('        <MediaTemplate EmbedCab="yes" />\n')

        icon_path = "packaging/icon.ico"
        if os.path.isfile(icon_path):
            output.write(
                f'        <Icon Id="AppIcon.ico" SourceFile="{escape_xml(icon_path)}" />\n'
            )
            output.write('        <Property Id="ARPPRODUCTICON" Value="AppIcon.ico" />\n')

        # Public IDs (uppercase) allow WixUI's feature tree to configure the paths.
        company_dir_id = "COMPANYDIR"
        output.write('        <StandardDirectory Id="CommonFiles64Folder">\n')
        output.write('            <Directory Id="VST3DIR" Name="VST3" />\n')
        output.write('            <Directory Id="CLAPDIR" Name="CLAP" />\n')
        output.write('            <Directory Id="LV2DIR" Name="LV2" />\n')
        output.write('            <Directory Id="AvidDir" Name="Avid">\n')
        output.write('                <Directory Id="AudioDir" Name="Audio">\n')
        output.write('                    <Directory Id="AAXDIR" Name="Plug-Ins" />\n')
        output.write("                </Directory>\n")
        output.write("            </Directory>\n")
        output.write("        </StandardDirectory>\n")
        output.write('        <StandardDirectory Id="ProgramFiles64Folder">\n')
        output.write(
            f'            <Directory Id="{company_dir_id}" Name="ZLAudio" />\n'
        )
        output.write("        </StandardDirectory>\n")

        # Tuple: (FormatName, Extension, DirectoryID, IsBundle)
        formats = [
            ("VST3", "vst3", "VST3DIR", True),
            ("CLAP", "clap", "CLAPDIR", True),
            ("AAX", "aaxplugin", "AAXDIR", True),
            ("LV2", "lv2", "LV2DIR", True),
            ("Standalone", "exe", company_dir_id, False),
        ]

        for fmt_name, extension, parent_dir_id, is_bundle_config in formats:
            env_var = f"{fmt_name}_PATH"
            if env_var not in os.environ:
                continue

            source_path = os.environ[env_var].strip()
            if not source_path:
                raise ValueError(f"{env_var} is set but empty")
            if not os.path.exists(source_path):
                raise FileNotFoundError(f"{env_var} does not exist: {source_path}")
            if not os.path.isfile(source_path) and not os.path.isdir(source_path):
                raise ValueError(f"{env_var} is not a file or directory: {source_path}")

            feature_id = f"Feature_{fmt_name}"
            features[feature_id] = {
                "title": fmt_name,
                "components": [],
                "config_dir": parent_dir_id,
            }
            components = features[feature_id]["components"]

            output.write(f'        <DirectoryRef Id="{parent_dir_id}">\n')
            is_source_file = os.path.isfile(source_path)
            use_bundle_logic = is_bundle_config and not is_source_file

            if use_bundle_logic:
                bundle_dir_name = f"{product_name}.{extension}"
                bundle_identity = make_component_identity(
                    upgrade_code,
                    wix_arch,
                    fmt_name,
                    parent_dir_id,
                    (bundle_dir_name,),
                )
                bundle_dir_id = get_wix_id(f"DIRECTORY|{bundle_identity}")
                output.write(
                    f'            <Directory Id="{bundle_dir_id}" '
                    f'Name="{escape_xml(bundle_dir_name)}">\n'
                )
                write_dir_recursive(
                    output,
                    source_path,
                    (bundle_dir_name,),
                    components,
                    upgrade_code,
                    wix_arch,
                    fmt_name,
                    parent_dir_id,
                    "                ",
                )
                output.write("            </Directory>\n")
            elif is_source_file:
                target_name = f"{product_name}.{extension}"
                write_file_component(
                    output,
                    source_path,
                    (target_name,),
                    components,
                    upgrade_code,
                    wix_arch,
                    fmt_name,
                    parent_dir_id,
                    "            ",
                    target_name=target_name,
                )
            else:
                write_dir_recursive(
                    output,
                    source_path,
                    (),
                    components,
                    upgrade_code,
                    wix_arch,
                    fmt_name,
                    parent_dir_id,
                    "            ",
                )
            output.write("        </DirectoryRef>\n")

            if not components:
                raise RuntimeError(
                    f"{env_var} did not contain any packageable files: {source_path}"
                )

        component_count = sum(len(data["components"]) for data in features.values())
        if component_count == 0:
            raise RuntimeError(
                "No plugin artifacts were provided. Set at least one supported *_PATH variable."
            )

        output.write(
            '        <Feature Id="Complete" Title="Complete Installation" '
            'Display="expand" Level="1">\n'
        )
        for feature_id, data in features.items():
            output.write(
                f'            <Feature Id="{feature_id}" '
                f'Title="{escape_xml(data["title"])}" Level="1" '
                f'ConfigurableDirectory="{data["config_dir"]}">\n'
            )
            for comp_id in data["components"]:
                output.write(f'                <ComponentRef Id="{comp_id}" />\n')
            output.write("            </Feature>\n")
        output.write("        </Feature>\n")

        eula_path = "packaging/EULA.rtf"
        readme_path = "packaging/Readme.rtf"
        if os.path.exists(eula_path):
            license_file = eula_path
        elif os.path.exists(readme_path):
            license_file = readme_path
        else:
            license_file = os.path.join(temp_dir, "GenericLicense.rtf")
            with open(license_file, "w", encoding="utf-8") as license_output:
                license_output.write(r"{\rtf1\ansi No EULA provided.\par}")
        output.write(
            f'        <WixVariable Id="WixUILicenseRtf" '
            f'Value="{escape_xml(license_file)}" />\n'
        )

        banner_bmp = "packaging/banner.bmp"
        if os.path.exists(banner_bmp):
            output.write(
                f'        <WixVariable Id="WixUIBannerBmp" '
                f'Value="{escape_xml(banner_bmp)}" />\n'
            )
        dialog_bmp = "packaging/dialog.bmp"
        if os.path.exists(dialog_bmp):
            output.write(
                f'        <WixVariable Id="WixUIDialogBmp" '
                f'Value="{escape_xml(dialog_bmp)}" />\n'
            )

        output.write("        <UI>\n")
        output.write('            <ui:WixUI Id="WixUI_FeatureTree" />\n')
        output.write('            <UIRef Id="WixUI_ErrorProgressText" />\n')
        output.write("        </UI>\n")
        output.write("    </Package>\n")
        output.write("</Wix>\n")

    print(f"Generated {outfile_path} with {component_count} components for {wix_arch}")


if __name__ == "__main__":
    main()
