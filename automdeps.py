import io 
import json
import os,sys
import re as Regex
import runpy
from urllib.request import *
import tarfile,zipfile

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

current_idx:int = 0

_counter:Counter

def parseAutomDepsFile(stream:io.TextIOWrapper,root:bool = True,count = 0):
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
        if c.get("type") == "clone":
            assert(c.get("url"))
            assert(c.get("dest"))
            assert(c.get("branch"))
            url = c.get("url")
            dest = c.get("dest")
            branch = None
            if(c.get("branch") != "default"):
                branch = c.get("branch")

            prior_dir = os.getcwd()
            if branch == None:
                os.system(f"git clone " + url + f" {dest}")
            else:
                os.system(f"git clone " + url + f" --branch {branch} {dest}")
            
        elif c.get("type") == "script":
            assert(c.get("path"))
            assert(c.get("args"))

            runpy.run_path(c.get("path") + " {}".format(c.get("args")))
        elif c.get("type") == "download":
            assert(c.get("url"))
            assert(c.get("dest"))
            res = urlretrieve(c.get("url"),c.get("dest"))
            print(res)
        elif c.get("type") == "tar":
            assert(c.get("tarfile"))
            assert(c.get("dest"))
            t_file:str = c.get("tarfile")
            tar_file_type = tar_file_regex.match(t_file)
            if tar_file_type == None:
                raise f"Tar File Format Invalid: \"{t_file}\""
            tar = tarfile.open(t_file,"r:*")
            tar.extractall(c.get("dest"))
            tar.close()
        elif c.get("type") == "unzip":
            assert(c.get("zipfile"))
            assert(c.get("dest"))
            z_file:str  = c.get("zipfile")
            _zip = zipfile.ZipFile(z_file,"r")
            _zip.extractall(c.get("dest"))
            _zip.close()
        _counter.increment()
    stream.close()

    if root:
        _counter.finish()

if __name__ == "__main__":
    # print(os.getcwd())
    if os.path.exists("./AUTOMDEPS"):
        c = countAutomDepsFileCommandsRecurse(io.open("./AUTOMDEPS","r"))
        parseAutomDepsFile(io.open("./AUTOMDEPS","r"),True,c)
    else:
        raise "AUTOMDEPS File Not Found in Current Directory. Exiting..."