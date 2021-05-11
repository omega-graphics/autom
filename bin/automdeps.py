import io 
import json
import os,sys
import re as Regex
import runpy
from enum import Enum
from typing import Any
from urllib.request import *
import tarfile,zipfile,shutil
import src.autom.autom
import src.gnpkg.main
import argparse
from queue import Queue

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

    if j.get("postCommands") is not None:
        current_len += len(j.get('postCommands'))

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
absroot:str
isAbsRoot:bool

Command = dict

variables:dict[str,Any] = {}

# Conditional Parser

def processStringWithVariables(string:str) -> str:
    s = string
    for k in variables:
        # print(k)
        regexExp = Regex.compile(rf"\$\({k}\)",Regex.DOTALL | Regex.MULTILINE)
        if isinstance(variables.get(k),str):
            s = regexExp.sub(variables.get(k),s)
        else:
            s = regexExp.sub(json.dumps(variables.get(k)),s)
    # print(s)
    return s
    
def processCommand(c:Command):
    assert(c.get("type"))
    if c.get("type") == "gnpkg":
        assert(c.get("command"))
        com:str = c.get('command')
        src.gnpkg.main.main(args=com.split(" "))

    elif c.get("type") == "git_clone":
        assert(c.get("url"))
        assert(c.get("dest"))
        assert(c.get("branch"))
        url = c.get("url")
        url = processStringWithVariables(url)
        dest = c.get("dest")
        dest = processStringWithVariables(dest)
        branch = None
        if(c.get("branch") != "default"):
            branch = c.get("branch")
            branch = processStringWithVariables(branch)

        prior_dir = os.getcwd()
        if updateOnly:
            print(f"Git Pull {c.get('url')}\nBranch:{c.get('branch')}")
            os.system(f"git pull")
        else:
            print(f"Git Clone {c.get('url')}\nBranch:{c.get('branch')}")
            if branch == None:
                os.system(f"git clone " + url + f" {dest}")
            else:
                os.system(f"git clone " + url + f" --branch {branch} {dest}")

        
    elif c.get("type") == "clone":
        # Same implementation as clone.py
        assert(c.get("url"))
        assert(c.get("dest"))
        assert(c.get("branch"))
        url = c.get("url")
        url = processStringWithVariables(url)
        dest = c.get("dest")
        dest = processStringWithVariables(dest)
        branch = None
        if(c.get("branch") != "default"):
            branch = c.get("branch")
            branch = processStringWithVariables(branch)

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
        elif c.get('type') == "chdir":
            assert(c.get("dir"))
            dir = processStringWithVariables(c.get('dir'))
            os.chdir(dir)
        elif c.get('type') == "system":
           assert(c.get("path"))
           path = processStringWithVariables(c.get("path"))
           os.system(path)
        elif c.get("type") == "script":
            assert(c.get("path"))
            assert(c.get("args"))
            path = c.get('path')
            path = processStringWithVariables(path)
            args = c.get("args")
            for arg in args:
                arg = processStringWithVariables(arg)
            print(f"Script {path}")
            sys.argv[1:] = args
            prev = os.path.abspath(os.getcwd())
            os.chdir(os.path.dirname(path))
            root_m,ext_m = os.path.splitext(path)
            print(root_m)
            runpy.run_module(root_m,run_name="__main__",alter_sys=True)
            os.chdir(prev)
        elif c.get("type") == "download":
            assert(c.get("url"))
            assert(c.get("dest"))
            url = c.get('url')
            url = processStringWithVariables(url)
            dest = c.get("dest")
            dest = processStringWithVariables(dest)
            os.makedirs(os.path.dirname(dest))
            print(f"Download {url}")
            res = urlretrieve(url,dest)
            print(res)
        elif c.get("type") == "tar":
            assert(c.get("tarfile"))
            assert(c.get("dest"))
            dest = c.get("dest")
            dest = processStringWithVariables(dest)
            t_file:str = c.get("tarfile")
            t_file = processStringWithVariables(t_file)
            tar_file_type = tar_file_regex.match(t_file)
            if tar_file_type == None:
                raise f"Tar File Format Invalid: \"{t_file}\""
            print(f"Tar {t_file}")
            tar = tarfile.open(t_file,"r:*")
            tar.extractall(dest)
            tar.close()
        elif c.get("type") == "unzip":
            assert(c.get("zipfile"))
            assert(c.get("dest"))
            dest = c.get("dest")
            dest = processStringWithVariables(dest)
            z_file:str  = c.get("zipfile")
            z_file = processStringWithVariables(z_file)
            _zip = zipfile.ZipFile(z_file,"r")
            print(f"Unzipping {z_file}")
            _zip.extractall(dest)
            _zip.close()
            os.remove(z_file)
            shutil.rmtree(os.path.dirname(z_file))
    return

postCommands:"Queue[list[Command]]" = Queue()
postRootCommands:"list[list[Command]]" = []
priorPostCommandsLen:int

def parseAutomDepsFile(stream:io.TextIOWrapper,root:bool = True,count = 0):
    global updateOnly
    global _counter
    global isAbsRoot
    global variables
    
    j:dict = json.load(stream)

    if isAbsRoot and j.get("rootCommands"):
        count += len(j.get("rootCommands"))
    if root:
        _counter = Counter(count)

    assert(j.get("commands"))
    commands:"list[dict]" = j.get("commands")
    _local_vars = j.get("variables")
    if _local_vars is not None:
        variables.update(_local_vars)

    if isAbsRoot:
        if j.get("rootCommands") is not None:
            rootCommands = j.get("rootCommands")
            for c in rootCommands:
                processCommand(c)
                _counter.increment()

        isAbsRoot = False

    global postCommands
    global postRootCommands
    global priorPostCommandsLen

    if j.get('postCommands') is not None:
        priorPostCommandsLen = len(postCommands)
        postCommands.put(j.get('postCommands'))
    
    if j.get('postRootCommands') is not None:
        postRootCommands.put(j.get('postRootCommands'))

    for c in commands:
        processCommand(c)
        _counter.increment()

    if j.get("subdirs") is not None:
        subdirs = j.get("subdirs")
        for s in subdirs:
            parent_dir = os.path.abspath(os.getcwd())
            t = os.path.abspath(s)
            os.chdir(t)
            print(f"Invoking sub-directory {os.path.relpath(t,start=absroot)}")
            parseAutomDepsFile(io.open(s + "/AUTOMDEPS","r"),root=False)
            os.chdir(parent_dir)
            if len(postCommands) > priorPostCommandsLen:
                p_cmd_list = postCommands.get()
                for c in p_cmd_list:
                    processCommand(c)
                    _counter.increment()
        
    stream.close()


    if root:
        _counter.finish()


    global postRootCommands
    if isAbsRoot and len(postRootCommands) > 0:
        print("Post Commands:")
        counter_len = 0 
        for cmd_list in postRootCommands:
            counter_len += len(cmd_list)
        _counter = Counter(counter_len)
        for cmd_list in postRootCommands:
            for c in cmd_list:
                processCommand(c)
                _counter.increment()
        _counter.finish()

def main():
    parser = argparse.ArgumentParser(prog="automdeps",description=
    "AUTOM Project Dependency Manager (Automates 3rd party library installation/fetching as well project configuration)")
    parser.add_argument("--exec",action="store_const",const=True,default=True)
    parser.add_argument("--update",dest="update",action="store_const",const=True,default=False)
    args = parser.parse_args()


    global updateOnly
    updateOnly = args.update
       
    global absroot
    absroot = os.path.abspath(os.getcwd())
    if os.path.exists("./AUTOMDEPS"):
        global isAbsRoot
        isAbsRoot = True

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
    