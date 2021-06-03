import os,sys,argparse
import subprocess,json

p = argparse.ArgumentParser()
p.add_argument("pkg",type=str)
p.add_argument("-F",action="store_const",const=True,default=False)
p.add_argument("-L",action="store_const",const=True,default=False)

args = p.parse_args()

flags = []
if args.F:
    flags.append("--cflags")
elif args.L:
    flags.append("--libs")

stream = os.popen(f" pkg-config {' '.join(flags)} {args.pkg}")
content = stream.read()
stream.close()
rc = content.split(" ")
rc.pop()
sys.stdout.write(json.dumps(rc))

