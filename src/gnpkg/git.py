# namespace git

import os 
import shutil

if shutil.which("git") == None:
    raise "git not found in PATH.\nExiting..."

git_cmd = "git"

def clone(url:str,branch:str = "default",cloneSubmodules:bool = False):
    cmd_str = f"{git_cmd} {url}"
    if branch != "default":
        cmd_str += f" --branch {branch}"
    
    if cloneSubmodules:
        cmd_str += "f --recurse-submodules"
    os.system(cmd_str)
    
def pull():
    os.system(f"{git_cmd} pull")