import io 
import json
import os,sys
import re as Regex
import runpy
from urllib.request import *
import tarfile,zipfile,shutil
import src.autom.autom
import src.gnpkg.main
import argparse

tar_file_regex = Regex.compile(r"(?:\.tar\.(\w{2}))|\.t(\w{2})$",Regex.DOTALL | Regex.MULTILINE)


class Counter:
    _len:int
    current_idx:int = 0
    def __init__(self,_len:int):
        self._len = _len
        s = f"{_len}"
        sys.stdout.write(f"[0/{_len}]\r")
        sys.stdout.flush() # return to start of line, after '['
        sys.stdout.write(f"\n")
    def increment(self):
        self.current_idx+=1
        sys.stdout.write(f"[{self.current_idx}/{self._len}]\r")
        sys.stdout.flush()
        sys.stdout.write(f"\n")
    def finish(self):
       sys.stdout.write(f"\n")


def countAutomDepsFileCommandsRecurse(stream:io.TextIOWrapper) -> int:
    j:dict = json.load(stream)
    assert(j.get("commands"))
    current_len = len(j.get("commands"))

    if j.get("subdirs") != None:
        subdirs = j.get("subdirs")
        for s in subdirs:
            current_len += countAutomDepsFileCommandsRecurse(io.open(s + "/AUTOMDEPS","r"))
    return current_len

#Global Vars
current_idx:int = 0

_counter:Counter
# Stores the abspaths of all the cloned repos using the clone.py tool waiting to synced with automdeps
clone_automdeps_queue:"list[str]" = []

updateOnly:bool = False

def parseAutomDepsFile(stream:io.TextIOWrapper,root:bool = True,count = 0):
    global updateOnly
    j:dict = json.load(stream)

    if root:
        _counter = Counter(count)

    assert(j.get("commands"))
    commands:"list[dict]" = j.get("commands")

    if j.get("subdirs") != None:
        subdirs = j.get("subdirs")
        for s in subdirs:
            parseAutomDepsFile(io.open(s + "/AUTOMDEPS","r"),root=False)

    for c in commands:
        assert(c.get("type"))
        if c.get("type") == "gnpkg":
            assert(c.get("command"))
            com = c.get('command')

            if com == "utils":
                print("GNPKG Get:gn-utils")
                src.gnpkg.main.link_utils()
            elif com == "get":
                assert(c.get('package'))
                print(f"GNPKG Get:{c.get('package')}")
                homeConfig:dict = json.load(io.open("./GNPKG"))
                assert(homeConfig.get("installDest"))
                src.gnpkg.main.get_package(c.get('package'),home=homeConfig.get("installDest"),homeConfig=homeConfig)

        elif c.get("type") == "git_clone":
            assert(c.get("url"))
            assert(c.get("dest"))
            assert(c.get("branch"))
            url = c.get("url")
            dest = c.get("dest")
            branch = None
            if(c.get("branch") != "default"):
                branch = c.get("branch")

            prior_dir = os.getcwd()
            if updateOnly:
                print(f"Git Pull {c.get('url')}\nBranch:{c.get('branch')}")
                os.system(f"git pull")
            else:
                print(f"Git Clone {c.get('url')}\nBranch:{c.get('branch')}")
                if branch == None:
                    os.system(f"git clone" + url + f" {dest}")
                else:
                    os.system(f"git clone " + url + f" --branch {branch} {dest}")

            
        elif c.get("type") == "clone":
            # Same implementation as clone.py
            assert(c.get("url"))
            assert(c.get("dest"))
            assert(c.get("branch"))
            url = c.get("url")
            dest = c.get("dest")
            branch = None
            if(c.get("branch") != "default"):
                branch = c.get("branch")

            prior_dir = os.getcwd()
            if updateOnly:
                print(f"AUTOM Sync {c.get('url')}\nBranch:{c.get('branch')}")
                os.system(f"git pull")
            else:
                print(f"AUTOM Clone {c.get('url')}\nBranch:{c.get('branch')}")
                if branch == None:
                    os.system(f"git clone " + url + f" {dest}")
                else:
                    os.system(f"git clone " + url + f" --branch {branch} {dest}")
            
            clone_automdeps_queue.append(os.path.abspath(dest))
        else:
            if updateOnly == True:
                print("SKIP")  
        if updateOnly == False:
            if c.get("type") == "autom":
                assert(c.get("dir"))
                assert(c.get("dest"))
                exports = runpy.run_path(f'{c.get("dir")}/AUTOM')
                assert(exports.get("export"))
                p = src.autom.autom.Project("","")
                p.add_targets(exports.get("export"))
                print(f"AUTOM {c.get('dir')}")
                src.autom.autom.generateProjectFiles(project=p,mode=src.autom.autom.ProjectFileType.GN,output_dir=c.get("dest"))
            
            elif c.get("type") == "script":
                assert(c.get("path"))
                assert(c.get("args"))
                print(f"Script {c.get('path')}")
                runpy.run_path(c.get("path") + " {}".format(c.get("args")))
            elif c.get("type") == "download":
                assert(c.get("url"))
                assert(c.get("dest"))
                os.makedirs(os.path.dirname(c.get("dest")))
                print(f"Download {c.get('url')}")
                res = urlretrieve(c.get("url"),c.get("dest"))
                print(res)
            elif c.get("type") == "tar":
                assert(c.get("tarfile"))
                assert(c.get("dest"))
                t_file:str = c.get("tarfile")
                tar_file_type = tar_file_regex.match(t_file)
                if tar_file_type == None:
                    raise f"Tar File Format Invalid: \"{t_file}\""
                print(f"Tar {t_file}")
                tar = tarfile.open(t_file,"r:*")
                tar.extractall(c.get("dest"))
                tar.close()
            elif c.get("type") == "unzip":
                assert(c.get("zipfile"))
                assert(c.get("dest"))
                z_file:str  = c.get("zipfile")
                _zip = zipfile.ZipFile(z_file,"r")
                print(f"Unzipping {z_file}")
                _zip.extractall(c.get("dest"))
                _zip.close()
                os.remove(z_file)
                shutil.rmtree(os.path.dirname(z_file))
         
        _counter.increment()
    stream.close()

    if root:
        _counter.finish()

def main():
    parser = argparse.ArgumentParser(prog="automdeps")
    parser.add_argument("--exec",action="store_const",const=True,default=True)
    parser.add_argument("--update",dest="update",action="store_const",const=True,default=False)
    args = parser.parse_args()


    global updateOnly
    updateOnly = args.update
       

    absroot = os.path.abspath(os.getcwd())
    if os.path.exists("./AUTOMDEPS"):
        print("Invoking root ./AUTOMDEPS")
        c = countAutomDepsFileCommandsRecurse(io.open("./AUTOMDEPS","r"))
        parseAutomDepsFile(io.open("./AUTOMDEPS","r"),True,c)

        for t in clone_automdeps_queue:
            print(f"Invoking sub-component {os.path.relpath(t,start=absroot)}")
            os.chdir(t)
            c = countAutomDepsFileCommandsRecurse(io.open("./AUTOMDEPS","r"))
            parseAutomDepsFile(io.open("./AUTOMDEPS","r"),True,c)
            os.chdir(absroot)
            
    else:
        raise "AUTOMDEPS File Not Found in Current Directory. Exiting..."
    return
if __name__ == "__main__":
    main()
    