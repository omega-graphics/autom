import argparse
import os 
import io,json

def addEntry(entries:"list[dict]",dest:str,sources:"list[str]") -> "list[dict]":
    e = {
        "dest":dest,
        "files":sources
    }
    entries.append(e)
    return entries

parser = argparse.ArgumentParser()
parser.add_argument("--file",type=str)
parser.add_argument('--dest',type=str)
parser.add_argument("sources",type=str,nargs="+")
args = parser.parse_args()

if os.path.exists(args.file):
    j = json.load(io.open(args.file,"r"))
    entries = j.get("entries")
    j["entries"] = addEntry(entries,dest=args.dest,sources=args.sources)
    json.dump(j,io.open(args.file,"w"),sort_keys=True,indent=2)
else:
    j = {
        "entries":[]
    }
    j["entries"] = addEntry(j.get("entries"),dest=args.dest,sources=args.sources)
    json.dump(j,io.open(args.file,"w"),sort_keys=True,indent=2)
