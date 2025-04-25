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
        # set-up windows developer prompt
        path2022 = 'C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Auxiliary/Build/vcvars64.bat'
        path2025 = 'C:/Program Files/Microsoft Visual Studio/2025/Enterprise/VC/Auxiliary/Build/vcvars64.bat'
        for msvc_path in [os.path.abspath(path2022), os.path.abspath(path2025)]:
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
