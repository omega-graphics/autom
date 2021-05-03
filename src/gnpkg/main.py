import argparse
import io
import json,os,shutil

class Package:
    name:str
    url:str 
    gitClone:bool 
    gnBindingDest:str
    def __init__(self, name:str,url:str ,gitClone:bool, gnBindingDest:str):
        self.name = name
        self.gitClone = gitClone
        self.url = url
        self.gnBindingDest = gnBindingDest
        return

def get_current_packages() -> "list[Package]":
    l:"list[Package]" = []
    j:dict = json.load(io.open(f"{os.path.dirname(os.path.abspath(__file__))}/pkgs.json","r"))
    assert(j.get("packages"))
    pkgs:"list[dict[str,str]]" = j.get("packages")
    for package in pkgs:
        l.append(Package(package.get("name"),package.get("url"),True,package.get("gn_binding")))
    return l

__all__ = [
    "get_package",
    "main",
    "link_utils"
]
def get_package(name:str,home:str,homeConfig:dict):
    if homeConfig.get("packages") == None:
        homeConfig["packages"] = []
    cfg:list = homeConfig["packages"]
    pkgs = get_current_packages()
    success:bool = False
    for p in pkgs:
        if p.name == name:
            success = True
            if p.gitClone:
                os.system("git clone " + p.url + f" {home}/{p.name}/code")
                original_dir =  os.path.abspath(os.getcwd())
                os.chdir(os.path.dirname(os.path.abspath(__file__)))
                
                for iter in os.walk(p.gnBindingDest):
                    for f in iter[2]:
                        shutil.copy2(os.path.join(iter[0],f),f"{home}/{p.name}/{os.path.basename(f)}")

                os.chdir(original_dir)
                # homeConfig["packages"].append({
                #     "name":p.name,
                #     "installedPath":f"{homeConfig.get('installDest')}/{p.name}"
                # })

    if success == False:
        raise f"Could not find package {name}.\nExiting..."

    # json.dump(homeConfig,io.open("./GNPKG","w"))

    return

def link_utils():
    dest = os.path.abspath(os.getcwd())
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    os.symlink(os.path.abspath("../../gn-utils"),dest + "/gn-utils",target_is_directory=True)
    os.chdir(dest)
def main():
    parser = argparse.ArgumentParser(prog="gnpkg")
    subparsers = parser.add_subparsers(dest="command")
    utils_parser = subparsers.add_parser("utils")
    utils_parser.add_argument("--get",action="store_const",const=True,default=False,help="Gets the Latest GN Utils and installs it in a `gn-utils` dir")
    get_parser = subparsers.add_parser("get")
    # parser.add_argument("--update",action="store_const",const=True,default=False)
    get_parser.add_argument("package",type=str)
    args = parser.parse_args()
    home:str
    homeConfig:dict
    if args.command != "utils":
        if os.path.exists("./GNPKG"):
                homeConfig = json.load(io.open("./GNPKG","r"))
                assert(homeConfig.get("installDest"))

                home = os.path.abspath(homeConfig.get("installDest"))
        else:
            raise FileNotFoundError("./GNPKG")

    if args.command == "utils":
        if args.get:
            link_utils()

    elif args.command == "get":
        get_package(args.package,home=home,homeConfig=homeConfig)
        
       
    
