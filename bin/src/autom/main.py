import argparse
from .autom import *
import runpy
import os
import ast,io

def main():
    parser = argparse.ArgumentParser(prog="autom",description="AUTOM Build Tool.. Automates the building of Native Projects utlizing CMake or GN")
    parser.add_argument("--mode",type=str)
    parser.add_argument("--out",type=str)
    parser.add_argument("-p",type=str,dest="p")
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
        # sym = {"build_mode":t} + AUTOM_LANG_SYMBOLS
        filename = os.path.abspath("./AUTOM")
        # stream = io.open(filename,"r")
        
        n = runpy.run_path(filename,init_globals=AUTOM_LANG_SYMBOLS)
            
    except FileNotFoundError:
        print("\u001b[31mERROR:\u001b[0m AUTOM file not found in current dir")
        exit(1)

    try:
        generateProjectFiles(n["project"],t,args.out)
    except KeyError:
        print(f"\u001b[31mERROR:\u001b[0m Exported project not found in AUTOM file")

