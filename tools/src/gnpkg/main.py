import argparse
import io
import json,os,shutil
import re as Regex

__all__ = [
    "main",
]

error_prefix = "\x1b[31mERROR:\x1b[0m"
warn_prefix = "\x1b[32mWARNING:\x1b[0m"

class Package:
    name:str
    url:str 
    recSubModules:bool
    gitClone:bool 
    gnBindingDest:str
    deps:"list[str]"
    def __init__(self, name:str,deps:"list[str]",url:str ,gitClone:bool, gnBindingDest:str,recSubModules:bool):
        self.name = name
        self.deps = deps
        self.gitClone = gitClone
        self.url = url
        self.gnBindingDest = gnBindingDest
        self.recSubModules = recSubModules
        return


def get_current_packages() -> "list[Package]":
    l:"list[Package]" = []
    j:dict = json.load(io.open(f"{os.path.dirname(os.path.abspath(__file__))}/pkgs.json","r"))
    assert(j.get("packages"))
    pkgs:"list[dict[str,str]]" = j.get("packages")
    for package in pkgs:
        clSubModules = False
        if package.get("cloneSubmodules") is None:
            clSubModules = False 
        else:
            clSubModules = package.get("cloneSubmodules")
        l.append(Package(package.get("name"),package.get("deps"),package.get("url"),True,package.get("gn_binding"),recSubModules=clSubModules))
    return l

PkgList = "list[dict[str,str]]"

def package_is_installed(name:str,installedPkgList:PkgList) -> bool:
    for i in installedPkgList:
        if i.get("name") == name and os.path.exists(i.get("installedPath") + f"/BUILD.gn"):
            return True 
    return False

pkgs:"list[Package]" = get_current_packages()

def get_package(name:str,home:str,homeConfig:dict,root:bool = True,install_if_root:bool = True,depContext:bool = False):
    if os.path.exists(f"{home}/Gnpkg.gni") == False:
        prev_dir = os.path.abspath(os.getcwd())
        os.chdir(os.path.abspath(os.path.dirname(__file__)))
        if os.path.exists(home) == False:
            os.makedirs(home)
        shutil.copy2("./Gnpkg.gni",f"{home}/Gnpkg.gni")
        os.chdir(prev_dir)
    
    if homeConfig.get("packages") == None:
        homeConfig["packages"] = []
    cfg:PkgList = homeConfig["packages"]

    global pkgs
    # Check if Package is Already Installed.
    if package_is_installed(name=name,installedPkgList=cfg):
        if not depContext:
            print(f"{error_prefix} Package {name} is already installed.",flush=True)
        return

    success:bool = False
    for p in pkgs:
        if p.name == name:
            success = True
            print(f"Installing Package {name}")
            if p.deps is not None:
                print("Dependencies:")
                for d in p.deps:
                    print(f"\t-> {d}")
                for d in p.deps:
                    get_package(d,home=home,homeConfig=homeConfig,root = False,depContext=True)
            if p.gitClone:
                
                if p.recSubModules:
                    git_cmd_line = "git clone --recurse-submodules " + p.url + f" {home}/{p.name}/code"
                else: 
                    git_cmd_line = "git clone " + p.url + f" {home}/{p.name}/code"
                os.system(git_cmd_line)
                original_dir =  os.path.abspath(os.getcwd())
                os.chdir(os.path.dirname(os.path.abspath(__file__)))
                
                for iter in os.walk(p.gnBindingDest):
                    for f in iter[2]:
                        shutil.copy2(os.path.join(iter[0],f),f"{home}/{p.name}/{os.path.basename(f)}")

                os.chdir(original_dir)
                homeConfig["packages"].append({
                    "name":p.name,
                    "installedPath":f"{homeConfig.get('installDest')}/{p.name}"
                })

    if success == False:
        raise f"Could not find package {name}.\nExiting..."

    if root and install_if_root:
        json.dump(homeConfig,io.open("./GNPKG","w"),sort_keys=True,indent=2)

    return

def package_get_dependents(name:str) -> "list[str]":
    dependents:"list[str]" = []
    global pkgs
    for p in pkgs:
        if p.deps is not None:
            for d in p.deps:
                if d == name:
                    dependents.append(p.name)
                    break
    return dependents

def package_is_git_cloned(name:str):
    global pkgs
    for p in pkgs:
        if p.name == name:
            return p.gitClone
    return None

# For Removing .git folder in git cloned packages
def onerror(func, path, exc_info):
    
    import stat
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        func(path)
    else:
        raise ""

def remove_package(name:str,homeConfig:dict):
    pkgs:PkgList = homeConfig.get("packages")
    success:bool = False
    for i in range(len(pkgs)):
        p = pkgs[i]
        assert(p.get('name'))
        if p.get("name") == name:
            success = True
            parents = package_get_dependents(name)
            if len(parents) > 0:
                cont = input(f"{warn_prefix} This package is a dependency of the following packages {parents}\nRemoving this package could result in build errors. Continue?[Y/N]:")
                if Regex.match(r"^N|n",cont):
                    exit(0)
                elif Regex.match(r"^Y|y",cont) == None:
                    print(f"Unknown Option:{cont}")
                    exit(1)
            # if package_is_git_cloned(name):
            #     shutil.rmtree(p.get("installedPath") + "/code/.git",onerror=onerror)
            shutil.rmtree(p.get("installedPath"))
            pkgs.pop(i)
            break 

    if not success:   
        print(f"{error_prefix} Package not previously installed.\nExiting...")
    else:
        json.dump(homeConfig,io.open("./GNPKG","w"),sort_keys=True,indent=2)


def link_utils():
    dest = os.path.abspath(os.getcwd())
    if os.path.exists(dest + "/gn-utils"):
        return
    else:
        os.chdir(os.path.dirname(os.path.abspath(__file__)))
        os.symlink(os.path.abspath("../../gn-utils"),dest + "/gn-utils",target_is_directory=True)
        os.chdir(dest)
def get_chromium_build_utils():
    os.system("git clone https://chromium.googlesource.com/chromium/src/build ./build")

def main(args):
    parser = argparse.ArgumentParser(prog="gnpkg")
    subparsers = parser.add_subparsers(dest="command")
    utils_parser = subparsers.add_parser("utils")
    utils_parser.add_argument("--get",action="store_const",const=True,default=False,help="Gets the Latest GN Utils and installs it in a `gn-utils` dir")
    utils_parser.add_argument("--getchrome",action="store_const",const=True,default=False,help="Fetches the Chromium GN Build Utils. (Used for chromium project repos)")
    get_parser = subparsers.add_parser("get")
    # parser.add_argument("--update",action="store_const",const=True,default=False)
    get_parser.add_argument("package",type=str,nargs="?")

    remove_parser = subparsers.add_parser("remove")
    remove_parser.add_argument("package",type=str)

    args = parser.parse_args(args=args)
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
        elif args.getchrome:
            get_chromium_build_utils()

    elif args.command == "get":
        if args.package is None:
           instPkgList:PkgList = homeConfig.get("packages")
           if instPkgList is None or len(instPkgList) == 0:
               print(f"{error_prefix} No Packages have been added to this config.Exiting...")
               exit(1)
           else:
               for p in instPkgList:
                   get_package(p.get("name"),home=home,homeConfig=homeConfig,install_if_root=False)
        else:
            get_package(args.package,home=home,homeConfig=homeConfig)
    elif args.command == "remove":
        remove_package(args.package,homeConfig=homeConfig)
        
       
    
