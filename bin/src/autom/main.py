import argparse
import sys
from .autom import *
import runpy
import os
import ast,io,json
from ..gnpkg.main import *

def __main():
    parser = argparse.ArgumentParser(prog="autom",description="AUTOM Build Tool.. Automates the building of Native Projects utlizing CMake or GN")
    parser.add_argument("--apple-codesig",dest="apple_codesig",type=str)
    parser.add_argument("--mode",type=str,choices=["gn","cmake","gradle"])
    parser.add_argument("--out",type=str)
    parser.add_argument("--export-comp-db",dest="export_comp_db",action="store_const",const=True,default=False,help=
    """
    Export compile_commands.json
    """)
    parser.add_argument("-p",type=str,dest="p")
    args = parser.parse_args()

    t:ProjectFileType
    if args.mode == "gn":
        t = ProjectFileType.GN
    elif args.mode == "cmake":
        t = ProjectFileType.CMAKE
    elif args.mode == "gradle":
        t = ProjectFileType.GRADLE
    else:
        print(f"\u001b[31mERROR:\u001b[0m Unknown project file mode:{args.mode}")
        exit(1)
    
    try:
        # sym = {"build_mode":t} + AUTOM_LANG_SYMBOLS
        filename = os.path.abspath("./AUTOM.build")
        # stream = io.open(filename,"r")


    except FileNotFoundError:
        print("\u001b[31mERROR:\u001b[0m AUTOM file not found in current dir")
        exit(1)
            
    project = execForProject(filename,t,args.out)

    generateProjectFiles(project,t,args.out)


    if t == ProjectFileType.CMAKE:
        os.system(f"cmake -S . -B {args.out}")
    elif t == ProjectFileType.GN:
        if not os.path.exists("./.gn"):
            main(args=["utils","--get"])
            stream = io.open("./.gn","w")
            if sys.platform == "win32":
                python3 = "py"
            else:
                python3 = "python3"
            stream.write(f"buildconfig = \"//gn-utils/BUILDCONFIG.gn\"\n\nscript_executable = \"{python3}\"")
            stream.close()
        _args = ""
        if args.apple_codesig:
            sig = args.apple_codesig
            _args = '--args="code_signature = \\"' + sig + '\\""'
            print(_args)
        if args.export_comp_db:
            os.system(f'gn gen {args.out} --export-compile-commands ' + _args)
        else:
            os.system(f'gn gen {args.out} ' + _args)
