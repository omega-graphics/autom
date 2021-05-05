import sys
import os,glob

import shutil

args = sys.argv
args.pop(0)

src = None 
dest = None
_glob:bool = False

for arg in args:

    if arg == "--src":
        idx = args.index(arg)
        src = args[idx + 1]
    elif arg == "--dest":
        idx = args.index(arg)
        dest = args[idx + 1]
    elif args == "--glob":
        _glob = True 

if _glob:
    if os.path.isdir(dest) == False:
        print(f"This is NOT a dir:{dest}")
        exit(1)
    
    results = glob.glob(src)
    for res in results:
        _file_dest = f"{dest}/{os.path.basename(res)}"
        if os.path.exists(_file_dest) == False:
            shutil.copy2(res,dest)

if os.path.isfile(src):
    if os.path.exists(dest):
        os.remove(dest)
    shutil.copy2(src,dest)
elif os.path.isdir(src):
    if os.path.exists(dest):
        shutil.rmtree(dest)
    shutil.copytree(src,dest,True)
