import ast
from enum import Enum
import io
import json
import sys
from typing import Any
import runpy
import os,shutil
import re as Regex
import glob
import importlib
import importlib.machinery
import importlib.util


from ..gnpkg import main as GNPKGMain

def resolve_resources(ls:"list[str]") -> "list[str]":
    for i in range(len(ls)):
        s = ls[i]
        ls[i] = os.path.abspath(s)
    return ls


def configure(file:str,output_file:str,__globals:"dict[str,Any]",atMode:bool):
    g = __globals

    stream = io.open(file,"r")
    data = stream.read()
    for __g in g:
        try:
            if atMode:
                p  = Regex.compile("@" + __g + "@",flags=Regex.MULTILINE)
                # print(p)
                data = p.sub(repl=Regex.sub(r"\\",r"\\\\",str(g[__g])),string=data)
                # print(data)
            else:
                p  = Regex.compile(rf"\$\({__g}\)",Regex.MULTILINE)
                data = Regex.sub(p,str(g[__g]),data)
        except TypeError and Regex.error as err:
            # print(f"Failed to use pattern: {err.pattern}")
            continue
    if not os.path.exists(os.path.dirname(output_file)):
        os.makedirs(os.path.dirname(output_file))
    write = io.open(output_file, "w")
    write.write(data)

class Namespace(object):
  def __init__(self, _dict):
    self.__dict__.update(_dict)

class TargetType(Enum):
    EXECUTABLE = 0
    LIBRARY = 1
    SCRIPT = 2
    COPY = 3
    IMPORTED_LIBRARY = 4
    APPLE_FRAMEWORK = 5
    APPLE_APP_BUNDLE = 6
    SOURCE_SET = 7
    GROUP = 8
    JAVA_ARCHIVE = 9

class TargetConfig:
    """
    Config Declaration
    """
    name : str
    deps:"list[str]"
    include_dirs: "list[str]"
    defines:"list[str]"
    libs:"list[str]"
    configs:"list[str]"
    def __init__(self,name:str,deps:"list[str]",include_dirs: "list[str]",defines:"list[str]"):
        self.name = name
        self.deps = deps 
        self.include_dirs = include_dirs
        self.defines = defines
        self.libs = []
        self.configs = None
        return

class Target :
    """
    Target Declaration
    """
    name: str
    __type__: TargetType
    source_files: "list[str]"
    dependencies: "list[str]"
    include_dirs: "list[str]"
    cflags: "list[str]"
    cxxflags: "list[str]"
    objcflags: "list[str]"
    objcxxflags: "list[str]"
    ldflags:"list[str]"
    output_dir:str
    defines:"list[str]"
    frameworks:"list[str]"
    framework_dirs:"list[str]"
    libs:"list[str]"
    lib_dirs:"list[str]"
    configs:"list[TargetConfig]"

    def __init__(self,name:str,_type:TargetType,source_files:"list[str]",deps:"list[str]"):
        self.name = name
        self.__type__ = _type
        self.source_files = source_files
        self.dependencies = deps
        self.include_dirs = []
        self.cflags: "list[str]" = []
        self.cxxflags: "list[str]" = []
        self.objcflags: "list[str]" = []
        self.objcxxflags: "list[str]" = []
        self.ldflags = []
        self.defines:"list[str]" = []
        self.frameworks:"list[str]" = []
        self.framework_dirs:"list[str]" = []
        self.libs:"list[str]" = []
        self.lib_dirs:"list[str]" = []
        self.configs = []

        self.set_current_dir()

    def set_current_dir(self):
        for i in range(len(self.source_files)):
           s = self.source_files[i]
           self.source_files[i] = os.path.abspath(s)

    def resolveConfig(self,conf:TargetConfig,confs:"dict[str,TargetConfig]"):
        # Check If Config has already been added!
        for c in self.configs:
            if c.name == conf.name:
                return 
        # Else resolve config as normal
        self.configs.append(conf)
        self.dependencies += conf.deps
        self.defines += conf.defines
        self.include_dirs += resolve_resources(conf.include_dirs)
        self.libs += conf.libs
        if conf.configs is not None:
            # print(confs)
            for c in conf.configs:
                _c = confs.get(c)
                if _c is None:
                    print(f"\x1b[31mERROR:\x1b[0m Config `{conf.name}` has an unresolved subconfig `{c}`")
                    exit(1)
                self.resolveConfig(_c,confs)
        return

project = None



class Project:
    name: str
    version: str
    __targets__: "list[Target]"
    __configs__:"dict[str,TargetConfig]"
    install_rules:"list[dict]"
    
    def __init__(self, name : str, version :str):
        self.name = name
        self.version = version 
        self.__targets__ = []
        self.__configs__ = {}
        self.install_rules = []
        global project
        project = self

    # def add_dir(self, name: str):
    #     n = os.path.abspath(name) + "/AUTOM.build"
    #     t = runpy.run_path(os.path.abspath(name) + "/AUTOM.build",init_globals={"project":self} + AUTOM_LANG_SYMBOLS)
    #     try:
    #         __n_t = t['targets']
    #         for t in __n_t:
    #             t.set_current_dir(name)
    #         self.__targets__ += __n_t
    #     except KeyError:
    #         print(f"\u001b[31mERROR:\u001b[0m The Variable targets is not defined in file scope:{n}")
    #         exit(1)
    #     return
    def get_target_by_name(self,name:str):
        for t in self.__targets__:
            if t.name == name:
                return t 
        
    def add_targets(self,l:"list[Target]"):
        for t in l:
            for dep in t.dependencies:
                _dep_t = self.get_target_by_name(dep)
                if _dep_t is None:
                    print(f"\x1b[31mERROR:\x1b[0m Target `{t.name}` has an unresolved dependency {dep}")
                    exit(1)
                for c in _dep_t.configs:
                    t.resolveConfig(c,self.__configs__)


        self.__targets__ += l
        return

    def install_targets(self,entry:str, targets:"list[Target]",loc:str):
        global target_os
        files:list = []
        for t in targets:
            if t.__type__.value == TargetType.LIBRARY.value:
                ext:str
                if t.shared:
                    if target_os == "mac":
                        ext = "dylib"
                    elif target_os == "win":
                        ext = "dll"
                        files.append(f"{t.output_dir}/{t.name}.lib")
                    else:
                        ext = "so"
                else:
                    if target_os == "win":
                        ext = "lib"
                    else:
                        ext = "a"
                files.append(f"{t.output_dir}/{t.name}.{ext}")

        e = {
            "name":entry,
            "dest":f"$(INSTALL_PREFIX)/{loc}",
            "files":files
        }

        self.install_rules.append(e)

        return 

    def install_files(self,entry:str, files:"list[str]",loc:str):
        e = {
            "name":entry,
            "dest":f"$(INSTALL_PREFIX)/{loc}",
            "files":files
        }
        self.install_rules.append(e)
        return 

    # def export_targets():


class SourceSet(Target):
    """
    Defines a source set
    """
    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]"):
        super(SourceSet,self).__init__(name,TargetType.SOURCE_SET,source_files,deps)
        self.output_dir = ""

class Executable(Target):
    """
    Defines an executable target
    """

    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]",output_dir:str):
        super(Executable,self).__init__(name,TargetType.EXECUTABLE,source_files,deps)
        self.output_dir = output_dir


class AppleApplicationBundle(Executable):
    """
    Defines a Apple App Bundle
    """
    embedded_frameworks:"list[str]"
    resources:"list[str]"
    plist:str
    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]",output_dir:str):
        global target_os
        if target_os != "mac":
            raise RuntimeError("AppBundles can only be declared if the target platform is macOS")
        super(AppleApplicationBundle,self).__init__(name,source_files,deps,output_dir=output_dir)
        self.__type__ = TargetType.APPLE_APP_BUNDLE
        self.output_dir = output_dir
        self.embedded_frameworks = []
        self.resources = []
        self.plist = ""

class Library(Target):
    """
    Defines a static or shared library target
    """
    shared:bool

    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]",output_dir:str,shared:bool):
        super(Library,self).__init__(name,TargetType.LIBRARY,source_files,deps)
        self.output_dir = output_dir
        self.shared = shared

class AppleFrameworkBundle(Library):
    """
    Defines a Apple Framework Bundle
    """
    version:str
    embedded_frameworks:"list[str]"
    resources:"list[str]"
    embedded_libs:"list[str]"
    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]",version:str,output_dir:str):
        global target_os
        if target_os != "mac":
            raise RuntimeError("FrameworkBundles can only be declared if the target platform is macOS")
        super(AppleFrameworkBundle,self).__init__(name,source_files,deps,shared=True,output_dir=output_dir)
        self.__type__ = TargetType.APPLE_FRAMEWORK
        self.output_dir = output_dir
        self.version = version
        self.embedded_frameworks = []
        self.embedded_libs = []
        self.resources = []



class Script(Target):
    script:str
    outputs:"list[str]"
    args:"list[str]"

    def __init__(self,name:str,source_files:"list[str]",dependencies:"list[str]",script:str,args:"list[str]",outputs:"list[str]"):
        super(Script,self).__init__(name,TargetType.SCRIPT,source_files,dependencies)
        self.script = script
        self.outputs = outputs
        self.args = args


class Copy(Target):
    output_dir:str

    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]",output_dir:str):
        super(Copy,self).__init__(name,TargetType.COPY,source_files,deps)
        self.output_dir = output_dir

class Group(Target):
    def __init__(self,name:str,deps:"list[str]"):
        super(Group,self).__init__(name,TargetType.GROUP,[],deps)

class ImportedLibrary(Target):
    lib:str 
    include_dirs:str

    def __init__(self,name:str,lib:str,include_dirs:"list[str]" = []):
        super(ImportedLibrary,self).__init__(name,TargetType.IMPORTED_LIBRARY,[],[])
        self.lib = lib
        self.include_dirs = include_dirs


class ProjectFileType(Enum):
    GN = 0,
    CMAKE = 1
    GRADLE = 2

# Gradle Only Functions/Classes
class GradleRepo:
    name:str 
    def __init__(self,name:str):
        self.name = name 

class GradleDepType(Enum):
    IMPLEMENATION = 0
    API = 1

class GradleDep:
    name:str 
    __type__:GradleDepType
    def __init__(self,name:str,_type:GradleDepType):
        self.name = name 
        self.__type__ = _type





target_os:str
if sys.platform == "win32":
    target_os:str = "win"
elif sys.platform == "darwin":
    target_os = "mac"
else:
    target_os = "linux"

def target_routine(project:Project):
    def dec(func):
        def wrapper(**args):
            # print(AUTOM_LANG_SYMBOLS)
            func.__dict__.update(AUTOM_LANG_SYMBOLS)
            targets = func(**args)
            project.add_targets(targets)
        return wrapper
    return dec

def include(interface:str):
    global project
    global AUTOM_LANG_SYMBOLS
    prev_dir = os.path.abspath(os.getcwd())
    n_globals = AUTOM_LANG_SYMBOLS.copy()
    n_globals["project"] = project
    file,ext = os.path.splitext(os.path.basename(interface))
    m = runpy.run_path(interface,n_globals,file)

    
    
    # AUTOM_LANG_SYMBOLS.update(nn_globals)
    os.chdir(prev_dir)
    return Namespace(m)
    # return nn_globals

AUTOM_LANG_SYMBOLS = {
    "Project": Project,
    "Executable": Executable,
    "Library": Library,
    "path": os.path,
    "ProjectFileType": ProjectFileType,
    "glob": glob.glob,
    "include":include,
    "TargetRoutine":target_routine,
    # Target OS
    "target_os": target_os,
    "is_win":sys.platform == "win32",
    "is_mac":sys.platform == "darwin",
    "is_linux":sys.platform == "linux",
    "AppBundle":AppleApplicationBundle,
    "FrameworkBundle":AppleFrameworkBundle
}