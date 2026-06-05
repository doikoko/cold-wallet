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
        if "linux" in current_os:
            subprocess.run(["sudo", "rm", "-rf", wallet_core_dir], check=True)
        else:
            shutil.rmtree(wallet_core_dir)

    os.mkdir(wallet_core_dir)
    os.mkdir(wallet_core_dir + "/build")
    os.mkdir(wallet_core_dir + "/include")

    files_to_copy = {
        "/wallet-core/build/libprotobuf.a": f"{wallet_core_dir}/build/",
        "/wallet-core/build/trezor-crypto/libTrezorCrypto.a": f"{wallet_core_dir}/build/",
        "/wallet-core/build/libTrustWalletCore.a": f"{wallet_core_dir}/build/",
        "/wallet-core/include/TrustWalletCore": f"{wallet_core_dir}/",
        "/wallet-core/src/proto/Binance.pb.h": f"{wallet_core_dir}/include/"
    }

    container_name = "tw_temp"

    commands = [
        ["docker", "pull", "trustwallet/wallet-core"],
        ["docker", "run", "-d", "--name", container_name, "trustwallet/wallet-core", "tail", "-f", "/dev/null"],
        *[
            ["docker", "cp", container_name + f":{doc_file}", f"{local_file}"]
            for doc_file, local_file in files_to_copy.items()
        ],
        ["docker", "rm", "-f", container_name],
        ["docker", "rmi", "trustwallet/wallet-core"],
    ]

    is_linux = "linux" in platform.platform().lower()

    if is_linux:
        commands = [["sudo"] + command for command in commands]

    for command in commands:
        print(f"running: {' '.join(command)}")
        subprocess.run(command, check=True)

    if is_linux:
        username = os.getlogin()

        print("fixing permissions and ownership...")
        subprocess.run(["sudo", "chown", "-R", f"{username}:{username}", wallet_core_dir], check=True)
        subprocess.run(["sudo", "chmod", "-R", "755", wallet_core_dir], check=True)

    trust_wallet_core_dir = f"{wallet_core_dir}/TrustWalletCore"
    if os.path.exists(trust_wallet_core_dir):
        for item_name in os.listdir(trust_wallet_core_dir):
            source_path = os.path.join(trust_wallet_core_dir, item_name)
            target_path = os.path.join(f"{wallet_core_dir}/include", item_name)
            shutil.move(source_path, target_path)

        shutil.rmtree(trust_wallet_core_dir)
    print("Done successfully!")

if __name__ == "__main__":
    main()
