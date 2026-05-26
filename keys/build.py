import subprocess
import os
import shutil
import platform


def check_dependencies() -> bool:
    dependencies = ["docker", "clang", "cmake", "ninja"]
    is_installed = True

    for dependency in dependencies:
        try:
            subprocess.run(
                [dependency, "--version"],
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
        except:
            is_installed = False
            print(f"you have no required dependency: {dependency}")

    return is_installed


def main():
    is_installed = check_dependencies()
    if not is_installed:
        exit(1)
    
    current_os = platform.system().lower()
    current_dir = os.getcwd()

    wallet_core_dir = current_dir + "/wallet-core"
    if os.path.isdir(wallet_core_dir):
        shutil.rmtree(wallet_core_dir)
    
    os.mkdir(wallet_core_dir)
    os.mkdir(wallet_core_dir + "/build")
    os.mkdir(wallet_core_dir + "/include")
    
    files_to_copy = {
        "/wallet-core/build/libprotobuf.a": f"{wallet_core_dir}/build/",
        "/wallet-core/build/trezor-crypto/libTrezorCrypto.a": f"{wallet_core_dir}/build/",
        "/wallet-core/build/libTrustWalletCore.a": f"{wallet_core_dir}/build/",
        "/wallet-core/include/TrustWalletCore": f"{wallet_core_dir}/",
    }

    container_name = "tw_temp"
    commands = [
        ["docker", "pull", "trustwallet/wallet-core"],
        ["docker", "run", "-d", "--name", container_name, "trustwallet/wallet-core"],
        *[
            ["docker", "cp", container_name + f":{doc_file}", f"{local_file}"]
            for doc_file, local_file in files_to_copy.items()
        ],
        ["docker", "rm", container_name],
        ["docker", "rmi", "trustwallet/wallet-core"],
    ]

    if "linux" in platform.platform().lower(): 
        commands = [["sudo"] + command for command in commands]
        subprocess.run(
            ["sudo", "find", current_dir, "-type", "d", "-exec", "chmod", "+755", "{}", "+"]
        )

    for command in commands:
        print(f"running: {' '.join(command)}")
        subprocess.run(command, check=True)

    os.rename(f"{wallet_core_dir}/TrustWalletCore", f"{wallet_core_dir}/include")    
    subprocess.run(["cmake", "-Bbuild", "-GNinja"])

if __name__ == "__main__": main()