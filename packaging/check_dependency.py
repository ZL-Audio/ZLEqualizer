import os
import sys
import platform
import subprocess


def main():
    # project_name = os.getenv("PROJECT_NAME", "Pamplejuce")
    product_name = os.getenv("PRODUCT_NAME", "")
    standalone_path = os.getenv("Standalone_PATH", "")

    if platform.system() == 'Linux':
        linux_path = os.path.join(standalone_path, product_name)
        result = subprocess.run(['ldd', standalone_path], capture_output=True, text=True)
        print(result.stdout)
        print(result.stderr)
    elif platform.system() == 'Windows':
        machine = platform.machine().lower()
        if machine == 'arm64':
            vcvars_script = 'vcvarsarm64.bat'
        else:
            vcvars_script = 'vcvars64.bat'

        program_files_x86 = os.environ.get('ProgramFiles(x86)', 'C:\\Program Files (x86)')
        vswhere_path = os.path.join(program_files_x86, 'Microsoft Visual Studio', 'Installer', 'vswhere.exe')

        vs_path = None
        if os.path.exists(vswhere_path):
            vswhere_cmd = f'"{vswhere_path}" -latest -property installationPath'
            vswhere_result = subprocess.run(vswhere_cmd, shell=True, capture_output=True, text=True)
            if vswhere_result.returncode == 0 and vswhere_result.stdout.strip():
                vs_path = vswhere_result.stdout.strip()

        if not vs_path:
            print("Error: Could not locate Visual Studio via vswhere.")
            return False
        msvc_path = os.path.join(vs_path, 'VC', 'Auxiliary', 'Build', vcvars_script)

        if os.path.exists(msvc_path):
            print(f"Found MSVC at: {msvc_path}")
            cmd = f'cmd.exe /c ""{msvc_path}" && set"'
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True)

            if result.returncode != 0:
                print(f"Error running {msvc_path}: {result.stderr}")
                return False

            for line in result.stdout.splitlines():
                if '=' in line:
                    key, value = line.split('=', 1)
                    os.environ[key] = value
        else:
            print(f"Error: Could not find {vcvars_script} at: {msvc_path}")
            return False

        result = subprocess.run(['dumpbin', '/dependents', standalone_path], capture_output=True, text=True)
        print(result.stdout)
        print(result.stderr)
    elif platform.system() == 'Darwin':
        macos_path = os.path.join(standalone_path, 'Contents', 'MacOS', product_name)
        result = subprocess.run(['otool', '-L', macos_path], capture_output=True, text=True)
        print(result.stdout)
        print(result.stderr)

if __name__ == '__main__':
    sys.exit(main())