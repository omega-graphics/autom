import argparse
import os 
import io,json

def addEntry(entries:"list[dict]",dest:str,sources:"list[str]",n:str) -> "list[dict]":
    e = {
        "name":n,
        "dest":dest,
        "files":sources
    }
    hasEntry:bool = False
    for _e in entries:
        if _e["name"] == n:
            _e["dest"] = e["dest"]
            _e["files"] = e["files"]
            hasEntry = True 
            break
    
    if not hasEntry:
        entries.append(e)
    
    return entries

parser = argparse.ArgumentParser()
parser.add_argument("name",type=str)
parser.add_argument("--file",type=str)
parser.add_argument('--dest',type=str)
parser.add_argument("sources",type=str,nargs="+")
args = parser.parse_args()

if os.path.exists(args.file):
    j = json.load(io.open(args.file,"r"))
    entries = j.get("entries")
    j["entries"] = addEntry(entries,dest=args.dest,sources=args.sources,n=args.name)
    json.dump(j,io.open(args.file,"w"),sort_keys=True,indent=2)
else:
    j = {
        "entries":[]
    }
    j["entries"] = addEntry(j.get("entries"),dest=args.dest,sources=args.sources,n=args.name)
    json.dump(j,io.open(args.file,"w"),sort_keys=True,indent=2)
