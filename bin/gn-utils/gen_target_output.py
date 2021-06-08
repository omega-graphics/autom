import argparse,json,io,os,sys

if sys.platform == "win32":
    ext = "dll"
elif sys.platform == "darwin":
    ext = "dylib"
else :
    ext = "so"

parser = argparse.ArgumentParser()
parser.add_argument("--group",type=str)
parser.add_argument("--targets",type=str,nargs="+")
parser.add_argument("--output-dir",type=str)
parser.add_argument("--export-file",type=str)
args = parser.parse_args()


if not os.path.exists(args.export_file):
    obj:"dict[str,list[str]]" = {}
    _n_targets = []
    for t in args.targets:
        _n_targets.append(os.path.join(args.output_dir,f"{t}.{ext}"))
    obj[args.group] = _n_targets
    json.dump(obj,io.open(args.export_file,mode="w"),sort_keys=True,indent=2)
else:
    obj = json.load(io.open(args.export_file,mode="r"))
    
    if obj.get(args.group) is None:
        _n_targets = []
        for t in args.targets:
            _n_targets.append(os.path.join(args.output_dir,f"{t}.{ext}"))
        obj[args.group] = _n_targets
        json.dump(obj,io.open(args.export_file,mode="w"),sort_keys=True,indent=2)
