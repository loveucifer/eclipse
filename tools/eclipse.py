import sys
import subprocess
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PREMAKE = os.path.join(ROOT, "premake5")
PREMAKE_FILE = os.path.join(ROOT, "premake5.lua")


def gen():
    subprocess.run([PREMAKE, "--file=" + PREMAKE_FILE, "gmake"], cwd=ROOT, check=True)


def build():
    subprocess.run(["make", "-C", ROOT], cwd=ROOT, check=True)


def run():
    subprocess.run(["./bin/Debug/eclipse"], cwd=ROOT, check=True)


def version():
    ver = {}
    exec(open(os.path.join(ROOT, "tools", "version.py")).read(), ver)
    print("eclipse tools -v{}.{}".format(ver["MAJOR"], ver["MINOR"]))


def help():
    print("Usage: eclipse <commands...>")
    print()
    print("Commands:")
    print("  gen           generate project files (premake)")
    print("  build         build the project")
    print("  run           run the executable")
    print("  version       print version")
    print()
    print("Examples:")
    print("  eclipse gen")
    print("  eclipse gen build")
    print("  eclipse build run")
    print("  eclipse gen build run")


TABLE = {
    "gen": gen,
    "build": build,
    "run": run,
    "version": version,
    "help": help,
}


if __name__ == "__main__":
    if len(sys.argv) < 2:
        help()
        sys.exit(1)

    for cmd in sys.argv[1:]:
        fn = TABLE.get(cmd)
        if fn is None:
            print("unknown command: " + cmd)
            help()
            sys.exit(1)
        fn()
