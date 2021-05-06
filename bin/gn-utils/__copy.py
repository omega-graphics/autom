import sys
import os,glob,argparse,json

import shutil





parser = argparse.ArgumentParser()
parser.add_argument("--src",type=str)
parser.add_argument("--dest",type=str)
parser.add_argument("--calc-glob-outputs",action="store_const",const=True,default=False)
parser.add_argument("--glob",dest="glob",action="store_const",const=True,default=False)
args = parser.parse_args()


src = args.src
dest = args.dest
_glob:bool = args.glob

if args.calc_glob_outputs:
    if os.path.isdir(dest) == False:
        print(f"This is NOT a dir:{dest}")
        exit(1)
    results = glob.glob(src)
    out_results:"list[str]" = []
    for res in results:
        _file_dest = f"{dest}/{os.path.basename(res)}"
        out_results.append(_file_dest)
    sys.stdout.write(json.dumps(out_results))
elif _glob:
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
