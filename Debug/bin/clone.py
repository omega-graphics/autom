import os
import argparse
import re as Regex
from typing import Text
import io
from autom_deps import *

git_regex = Regex.compile(r"\/(.+)\.git$", flags=Regex.MULTILINE | Regex.DOTALL)

parser = argparse.ArgumentParser(prog="autom-clone")
parser.add_argument("--runhooks", dest="runhooks", action="store_const", const=True, default=False)
parser.add_argument("--branch", type=str)
parser.add_argument("git_repo_url", type=Text)
parser.add_argument("dest", type=str)
args = parser.parse_args()

if args.dest is None:
    args.dest = ""

if args.git_repo_url:
    print(f"1. Cloning \"{args.git_repo_url}\"")
    if args.branch is None:
        os.system(f"git clone " + args.git_repo_url + f" {args.dest}")
    else:
        os.system(f"git clone " + args.git_repo_url + f" --branch {args.branch} {args.dest}")
    if args.dest == "":
        m = git_regex.match(args.git_repo_url)     
        os.chdir(m.group(0))
    else:
        os.chdir(args.dest)
    
    if os.path.exists("./AUTOMDEPS"):
        print(f"2. Running AUTOMDEPS commands")
        main(args=[])


