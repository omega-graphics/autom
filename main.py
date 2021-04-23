import argparse
from src.autom import *
import runpy
import os

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode",type=str)
    parser.add_argument("--out",type=str)
    args = parser.parse_args()

    t:ProjectFileType
    if args.mode == "gn":
        t = ProjectFileType.GN
    elif args.mode == "cmake":
        t = ProjectFileType.CMAKE
    else:
        print(f"\u001b[31mERROR:\u001b[0m Unknown project file mode:{args.mode}")
        exit(1)

    try:
        n = runpy.run_path(os.path.abspath("./AUTOMPROJ"),{"build_mode":t})
    except FileNotFoundError:
        print("\u001b[31mERROR:\u001b[0m AUTOMPROJ file not found in current dir")
        exit(1)

    try:
        gen = ProjectFileGen()
        gen.generateProjectFiles(n["export"],t,args.out)
    except KeyError:
        print(f"\u001b[31mERROR:\u001b[0m Exported project not found in AUTOMPROJ file")

if __name__ == "__main__":
    main()