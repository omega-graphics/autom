import os,argparse

parser = argparse.ArgumentParser()
parser.add_argument("--jar",type=str)
parser.add_argument("--class-path",type=str,required=False)
parser.add_argument("--entry-point",type=str,required=False)
parser.add_argument("-g",action="store_const",const=True,default=False)
parser.add_argument("-d",type=str)
parser.add_argument("sources",type=str,nargs="+")
args = parser.parse_args()

args:str

if args.class_path is not None:
    args = f"{args} -cp {args.class_path}"

if args.g:
    args = f"{args} -g"

if args.d is not None:
    args = f"{args} -d {args.d}"


if os.system(f"javac {args} {' '.join(args.sources)}") != 0:
    exit(1)

class_files:"list[str]" = []

_srcs:"list[str]" = args.sources

for s in _srcs:
    file,ext = os.path.splitext(os.path.basename(s))
    class_files.append(os.path.join(args.d,f"{file}.class"))

if os.system(f"jar -cf {args.jar} {' '.join(class_files)}") != 0:
    exit(1)

