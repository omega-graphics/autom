"""AUTOM Build Tool

"""

import ast
from enum import Enum
import io
import json
import sys
from typing import Any
import runpy
import os
import re as Regex
import glob
import importlib
import importlib.machinery
import importlib.util


from ..gnpkg import main as GNPKGMain


__all__ = [
    "Project",
    "Executable",
    "Library",
    "ProjectFileType",
    "Script",
    "Copy",
    "configure",
    "ImportedLibrary",
    "AUTOM_LANG_SYMBOLS",
    # "interp",
    "generateProjectFiles",
    "AUTOMInterp"
]


def configure(file:str,output_file:str,__globals):
    g = __globals
    cwd = os.getcwd()

    file = g["__project_dir__"] + "/" + os.path.basename(file)
    print(file)
    stream = open(file,"r")
    data = stream.read()
    for __g in g:
        # print(__g)
        try:
            p  = Regex.compile(r"\$\({}\)".format(__g),Regex.MULTILINE | Regex.DOTALL)
            data = Regex.sub(p,str(g[__g]),data)
        except TypeError:
            continue
    write = open(output_file, "w")
    write.write(data)

class Namespace(object):
  def __init__(self, _dict):
    self.__dict__.update(_dict)

class TargetType(Enum):
    EXECUTABLE = 0,
    LIBRARY = 1,
    SCRIPT = 2,
    COPY = 3,
    IMPORTED_LIBRARY = 4
    APPLE_FRAMEWORK = 5,
    APPLE_APP_BUNDLE = 6,
    SOURCE_SET = 7


class Target :
    """
    Target Declaration
    """
    name: str
    __type__: TargetType
    source_files: "list[str]"
    dependencies: "list[str]"
    include_dirs: "list[str]"
    cflags: "list[str]" = []
    cxxflags: "list[str]" = []
    objcflags: "list[str]" = []
    objcxxflags: "list[str]" = []
    output_dir:str
    defines:"list[str]" = []
    frameworks:"list[str]" = []
    framework_dirs:"list[str]" = []
    libs:"list[str]" = []
    lib_dirs:"list[str]" = []

    def __init__(self,name:str,_type:TargetType,source_files:"list[str]",deps:"list[str]"):
        self.name = name
        self.__type__ = _type
        self.source_files = source_files
        self.dependencies = deps
        self.include_dirs = []

        self.set_current_dir()

    def set_current_dir(self):
        for i in range(len(self.source_files)):
           s = self.source_files[i]
           self.source_files[i] = os.path.abspath(s)


project = None

class Project:
    name: str
    version: str
    __targets__: "list[Target]"
    install_rules:"list[dict]"
    
    def __init__(self, name : str, version :str):
        self.name = name
        self.version = version 
        self.__targets__ = []
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

    def add_targets(self,list:"list[Target]"):
        self.__targets__ += list
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
        super(Executable,self).__init__(name,TargetType.SOURCE_SET,source_files,deps)
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
    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]",output_dir:str):
        global target_os
        if target_os != "mac":
            raise RuntimeError("AppBundles can only be declared if the target platform is macOS")
        super(AppleApplicationBundle,self).__init__(name,source_files,deps,output_dir=output_dir)
        self.__type__ = TargetType.APPLE_APP_BUNDLE
        self.output_dir = output_dir
        self.embedded_frameworks = []
        self.resources = []

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
    def __init__(self,name:str,source_files:"list[str]",deps:"list[str]",version:str,output_dir:str):
        global target_os
        if target_os != "mac":
            raise RuntimeError("FrameworkBundles can only be declared if the target platform is macOS")
        super(AppleFrameworkBundle,self).__init__(name,source_files,deps,shared=True,output_dir=output_dir)
        self.__type__ = TargetType.APPLE_FRAMEWORK
        self.output_dir = output_dir
        self.version = version
        self.embedded_frameworks = []
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




def cmake_bridge(dir:str,args:str) -> "list[Target]":
    prev_dir = os.path.abspath(os.getcwd())
    os.chdir(dir)

    if not os.path.exists("./CMakeLists.txt"):
        raise f"\x1b[31mCMakeLists.txt does not exist in this folder: {dir}\x1b[0m"

    if not os.path.exists("./build/.cmake/v1/query/client-autom"):
        os.mkdir("./build/.cmake/v1/query/client-autom")

    os.system(f"cmake -S . -B ./build -G\"Ninja\" {args}")

    os.chdir(prev_dir)

    return


def gn_bridge(dir:str,args:str) -> "list[Target]":
    prev_dir = os.path.abspath(os.getcwd())
    os.chdir(dir)

    if not os.path.exists("./BUILD.gn"):
        raise f"\x1b[31mBUILD.gn does not exist in this folder: {dir}\x1b[0m"

    if not os.path.exists("./.gn"):
        GNPKGMain.main(args=["utils","--get"])
        stream = io.open("./.gn","w")
        if sys.platform == "win32":
            python3 = "py -3"
        else:
            python3 = "python3"
        stream.write(f"buildconfig = \"//gn-utils/BUILDCONFIG.gn\"\n\nscript_executable = \"{python3}\"")
        stream.close()
    
    # Parse JSON File

    os.system(f"gn gen out --args={args} --ide=json")
    j = json.load(io.open("./out/project.json","r"))





    os.remove("./.gn")
    os.remove("./gn-utils")

    os.chdir(prev_dir)

    return

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
    "gn_bridge": gn_bridge,
    "cmake_bridge": cmake_bridge,
    "glob": glob.glob,
    "include":include,
    "TargetRoutine":target_routine,
    # Target OS
    "target_os": target_os,
    "is_win":sys.platform == "win32",
    "is_mac":sys.platform == "darwin",
    "AppBundle":AppleApplicationBundle,
    "FrameworkBundle":AppleFrameworkBundle
}

class __GNGenerator__:
    """
    Private class for generating GN
    """
    targets:"list[Target]"
    def __init__(self,targets:"list[str]"):
        self.targets = targets
        return
    def __formatDeps(self,deps:"list[str]") -> list:
        for d in deps:
            n = ":{}".format(d)
            deps[deps.index(d)] = n
        return deps

    def writeStandardTargetProps(self,t:Target,stream:io.TextIOWrapper):
        stream.write("  include_dirs = {}\n".format(json.dumps(t.include_dirs)))
        stream.write("  sources = {}\n".format(json.dumps(t.source_files,indent=2,sort_keys=True)))
        stream.write("  public_deps = {}\n".format(json.dumps(self.__formatDeps(t.dependencies))))
        stream.write("  defines = {}\n".format(json.dumps(t.defines)))
        stream.write("  libs = {}\n".format(json.dumps(t.libs)))
        if len(t.lib_dirs) > 0:
            stream.write("  lib_dirs = {}\n".format(json.dumps(t.lib_dirs)))
        stream.write("  frameworks = {}\n".format(json.dumps(t.frameworks)))
        if len(t.framework_dirs) > 0:
            stream.write("  framework_dirs = {}\n".format(json.dumps(t.framework_dirs)))
        if len(t.cflags) > 0:
            stream.write("  cflags = {}\n".format(json.dumps(t.cflags)))
        stream.write(f"  output_dir = \"$root_out_dir/{t.output_dir}\"\n")

    def generate(self,out_file:str):
        out_dir = os.path.dirname(out_file)
        if os.path.exists(out_dir) == False: 
            os.makedirs(out_dir)
        stream  = open(out_file,"w")
        stream.write("# This File Was Generated by AUTOM Build Tool. Do NOT EDIT!!!\n")
        stream.write('import("//gn-utils/Utils.gni")\n\n')
        print(self.targets)
        for t in self.targets:
            if t.__type__.value == TargetType.EXECUTABLE.value:
                stream.write(f"executable(\"{t.name}\")" + "{\n")
                self.writeStandardTargetProps(t,stream)
                # stream.write("deps = ")
                stream.write("\n}")
            elif t.__type__.value == TargetType.LIBRARY.value:
                if t.shared:
                    stream.write(f"shared_library(\"{t.name}\")" + "{\n")
                else: 
                    stream.write(f"static_library(\"{t.name}\")" + "{\n")
                self.writeStandardTargetProps(t,stream)
                stream.write("\n}")
            elif t.__type__.value == TargetType.APPLE_FRAMEWORK.value:
                stream.write(f"mac_framework_bundle(\"{t.name}\")" + "{\n")
                stream.write(f"  version = \"{t.version}\"\n")
                stream.write("  resources = {}\n".format(json.dumps(t.resources)))
                if len(t.embedded_frameworks) > 0:
                    stream.write("  embedded_frameworks = {}\n".format(json.dumps(t.embedded_frameworks)))
                self.writeStandardTargetProps(t,stream)
                stream.write("\n}")
            elif t.__type__.value == TargetType.SCRIPT.value:
                stream.write(f"action(\"{t.name}\")" + "{\n")
                stream.write("  sources = {}\n".format(json.dumps(t.source_files)))
                stream.write(f" script = \"{t.script}\"\n")
                stream.write("  args = {}\n".format(json.dumps(t.args)))
                stream.write("  public_deps = {}\n".format(json.dumps(self.__formatDeps(t.dependencies))))
                stream.write("  outputs = {}".format(json.dumps(t.outputs)))
                stream.write("\n}")
            stream.write("\n\n")
        stream.close()



class __CmakeGenerator__:
    """
    Private class for generating CMake
    """
    targets: "list[Target]"
    proj_info: Project
    def __init__(self,targets:"list[Target]",project:Project):
        self.targets = targets
        self.proj_info = project
    def format_source_files(self,srcs:"list[str]"):
        for i in range(0,len(srcs)):
            s = srcs[i]
            n = "\"{}\"".format(s)
            srcs[i] = n 
        return " ".join(srcs)
    def generate(self,out_file:str):
        out_dir = os.path.dirname(out_file)
        if os.path.exists(out_dir) == False: 
            os.makedirs(out_dir)
        stream  = io.open(out_file,"w")
        stream.write("# This File Was Generated by AUTOM Build Tool. Do NOT EDIT!!!\n")
        print(self.targets)
        stream.write(f"cmake_minimum_required(VERSION 3.13)")
        stream.write("\n")
        stream.write(f"project(\"{self.proj_info.name}\" VERSION \"{self.proj_info.version}\" LANGUAGES C CXX)")
        stream.write("\n\n")
        for t in self.targets:
            if t.__type__.value == TargetType.EXECUTABLE.value:
                stream.write(f"add_executable(\"{t.name}\" " + self.format_source_files(srcs=t.source_files) + ")\n")
                stream.write(f"set_target_properties(\"{t.name}\" PROPERTIES RUNTIME_OUTPUT_DIRECTORY " + "\"${CMAKE_BINARY_DIR}/" + f"{t.output_dir}\")\n")
                stream.write("\n")
            elif t.__type__.value == TargetType.LIBRARY.value:
                if t.shared:
                    stream.write(f"add_library(\"{t.name}\" SHARED {self.format_source_files(srcs=t.source_files)})\n")
                    stream.write(f"set_target_properties(\"{t.name}\" PROPERTIES RUNTIME_OUTPUT_DIRECTORY " + "\"${CMAKE_BINARY_DIR}/" + f"{t.output_dir}\")\n")
                else:
                    stream.write(f"add_library(\"{t.name}\" STATIC {self.format_source_files(srcs=t.source_files)})\n")
                stream.write(f"set_target_properties(\"{t.name}\" PROPERTIES ARCHIVE_OUTPUT_DIRECTORY " + "\"${CMAKE_BINARY_DIR}/" + f"{t.output_dir}\")\n")
                
        stream.close()

def resolve_resources(ls:"list[str]") -> "list[str]":
    for i in range(len(ls)):
        s = ls[i]
        ls[i] = os.path.abspath(s)
    return ls

class AUTOMInterp(object):
    symTable:"dict[str,Any]"

    p:Project

    inRootFile:bool

    inInterfaceFileTop:bool

    inFuncContext:bool 

    willReturn:bool 

    returnVal:Any

    def __init__(self):
        self.inRootFile = True 
        self.inInterfaceFileTop = False
        self.symTable = {}
        self.inFuncContext = False 
        self.willReturn = False
        self.returnVal = None

    def error(self,node:ast.AST,message:str):
        print(f"\x1b[31mERROR:\x1b[0m {message} -> LOC {node.lineno}:{node.col_offset}")
        exit(1)
        

    def evalExpr(self,expr:ast.expr,temp_scope = None) -> Any:
        if isinstance(expr,ast.Call):
            if isinstance(expr.func,ast.Name):
                # Eval standard lib functions if name matches
                if expr.func.id == "Project" and not self.inInterfaceFileTop:
                    if not self.inRootFile:
                        return
                    project_id:str = self.evalExpr(expr.args[0],temp_scope)
                    project_version_str:str = self.evalExpr(expr.args[1],temp_scope)
                    self.p = Project(project_id,project_version_str)
                    return 
                elif expr.func.id == "Executable" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(list=[Executable(name=t_name,source_files=srcs,deps=deps,output_dir=output_dir)])
                    return
                elif expr.func.id == "StaticLibrary" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(list=[Library(name=t_name,source_files=srcs,deps=deps,output_dir=output_dir,shared=False)])
                    return
                elif expr.func.id == "SharedLibrary" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(list=[Library(name=t_name,source_files=srcs,deps=deps,output_dir=output_dir,shared=True)])
                    return
                elif expr.func.id == "AppBundle" and not self.inInterfaceFileTop:
                    if not AUTOM_LANG_SYMBOLS["is_mac"]:
                        self.error(expr.func,"AppBundle target can only be declared if target os is macOS or iOS")
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(list=[AppleApplicationBundle(t_name,srcs,deps,output_dir)])
                    return
                elif expr.func.id == "FrameworkBundle" and not self.inInterfaceFileTop:
                    if not AUTOM_LANG_SYMBOLS["is_mac"]:
                        self.error(expr.func,"FrameworkBundle target can only be declared if target os is macOS or iOS")
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    version:str = self.evalExpr(expr.args[4],temp_scope)
                    self.p.add_targets(list=[AppleFrameworkBundle(t_name,srcs,deps,version,output_dir)])
                    return
                elif expr.func.id == "Script" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    script_n:str = self.evalExpr(expr.args[3],temp_scope)
                    script_args:"list[str]" = self.evalExpr(expr.args[4],temp_scope)
                    outputs:"list[str]" = self.evalExpr(expr.args[5],temp_scope)
                    self.p.add_targets(list=[Script(name=t_name,source_files=srcs,dependencies=deps,script=script_n,args=script_args,outputs=outputs)])
                    return
                elif expr.func.id == "set_property" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    prop_name:str = self.evalExpr(expr.args[1],temp_scope)
                    data:Any = self.evalExpr(expr.args[2],temp_scope)
                    
                    for t in self.p.__targets__:
                        if t.name == t_name:
                                if isinstance(t,AppleFrameworkBundle):
                                    if prop_name == "resources":
                                        t.resources = resolve_resources(data)
                                        return
                                    elif prop_name == "embedded_frameworks":
                                        t.embedded_frameworks = data
                                        return

                                if prop_name == "cflags":
                                    t.cflags = data 
                                elif prop_name == "cxxflags":
                                    t.cxxflags = data
                                elif prop_name == "objcflags":
                                    t.objcflags = data 
                                elif prop_name == "objcxxflags":
                                    t.objcxxflags = data 
                                elif prop_name == "defines":
                                    t.defines = data 
                                elif prop_name == "include_dirs":
                                    t.include_dirs = resolve_resources(data) 
                                elif prop_name == "frameworks":
                                    t.frameworks = data 
                                elif prop_name == "framework_dirs":
                                    t.framework_dirs = resolve_resources(data)
                                else:
                                    self.error(expr.func,f"Cannot set property `{prop_name}` on target `{t.name}`")

                    return
                elif expr.func.id == "include":
                    file:str = self.evalExpr(expr.args[0],temp_scope)

                    prior_0 = self.inRootFile 
                    prior_1 = self.inInterfaceFileTop

                    self.inRootFile = False
                    self.inInterfaceFileTop = True

                    prior_dir = os.path.abspath(os.getcwd())
                    data = io.open(file,"r").read()

                    os.chdir(os.path.dirname(file))

                    __module = ast.parse(data,file)

                    self.interp(__module)

                    os.chdir(prior_dir)

                    self.inRootFile = prior_0
                    self.inInterfaceFileTop = prior_1
                    return
                elif expr.func.id == "subdir":
                    file:str = os.path.join(self.evalExpr(expr.args[0],temp_scope),"AUTOM.build")

                    prior_0 = self.inRootFile 
                    prior_1 = self.inInterfaceFileTop

                    self.inRootFile = False
                    self.inInterfaceFileTop = False

                    prior_dir = os.path.abspath(os.getcwd())
                    data = io.open(file,"r").read()

                    os.chdir(os.path.dirname(file))

                    __module = ast.parse(data,file)

                    self.interp(__module)

                    os.chdir(prior_dir)

                    self.inRootFile = prior_0
                    self.inInterfaceFileTop = prior_1
                    return
                # FS Functions
                elif expr.func.id == "glob":
                    pattern:str = self.evalExpr(expr.args[0],temp_scope)
                    return glob.glob(pattern)
                elif expr.func.id == "abspath":
                    path:str = self.evalExpr(expr.args[0],temp_scope)
                    return os.path.abspath(path)
                elif expr.func.id == "print":
                    obj:Any = self.evalExpr(expr.args[0],temp_scope)
                    print(obj)
                    return None

            if not isinstance(expr.func,ast.Name):
                self.error(expr.func,"Expected a Function Name")
            
            
            obj:ast.FunctionDef = self.symTable[expr.func.id]


            _temp_scope:"dict[str,Any]" = {}

            if len(expr.args) > 0:
                self.error(expr.args,"Positional args are not supported in AUTOM.. Closing..")

            for kw in expr.keywords:
                _temp_scope[kw.arg] = self.evalExpr(kw.value,temp_scope)

            if temp_scope is not None:
                _temp_scope.update(temp_scope)

            prior:bool
            if self.inInterfaceFileTop:
                prior = True
                self.inInterfaceFileTop = False
            else:
                prior = False

            self.inFuncContext = True
            for stmt in obj.body:
                if self.willReturn:
                    break
                self.evalStmt(stmt,_temp_scope)
            self.inFuncContext = False

            
            if self.willReturn:
                self.willReturn = False
                return self.returnVal

            if prior:
                self.inInterfaceFileTop = prior
            return
        elif isinstance(expr,ast.Name):
            # 1. -  Eval Builtin Identifers 
            if expr.id == "is_mac":
                return AUTOM_LANG_SYMBOLS['is_mac']
            elif expr.id == "is_win":
                return AUTOM_LANG_SYMBOLS['is_win'] 

            # 2. - Eval Temp Scope Identifiers
            if temp_scope is not None:
                if temp_scope.get(expr.id) is not None:
                    return temp_scope[expr.id]

            # 3. - Eval Global Identifiers

            if self.symTable.get(expr.id) is not None:
                return self.symTable[expr.id]
            
            self.error(expr,"Unknown Identifier")
        elif isinstance(expr,ast.BoolOp):
            left_val = self.evalExpr(expr.values[0],temp_scope)
            right_val = self.evalExpr(expr.values[1],temp_scope)
            if isinstance(expr.op,ast.And):
                return left_val and right_val
            elif isinstance(expr.op,ast.Or):
                return left_val or right_val
        elif isinstance(expr,ast.UnaryOp):
            val = self.evalExpr(expr.operand)
            if isinstance(expr.op,ast.Not):
                return not val
            elif isinstance(expr.op,ast.UAdd):
                return +val
            elif isinstance(expr.op,ast.USub):
                return -val
            else:
                return ~val
        elif isinstance(expr,ast.BinOp):
            left_val = self.evalExpr(expr.left,temp_scope)
            right_val = self.evalExpr(expr.right,temp_scope)
            if isinstance(expr.op,ast.Add):
                return left_val + right_val
            elif isinstance(expr.op,ast.Sub):
                return left_val - right_val
            elif isinstance(expr.op,ast.Mult):
                return left_val * right_val 
            elif isinstance(expr.op,ast.Div):
                return left_val / right_val
            elif isinstance(expr.op,ast.Pow):
                return left_val ** right_val
            elif isinstance(expr.op,ast.Mod):
                return left_val % right_val
        
        elif isinstance(expr,ast.Constant):
            return expr.value
        # Make Standard Types
        elif isinstance(expr,ast.List):
            rc = []
            for val in expr.elts:
                rc.append(self.evalExpr(val,temp_scope))
            return rc 
        elif isinstance(expr,ast.Dict):
            rc = {}
            for i in range(len(expr.keys)):
                k = expr.keys[i]
                v = expr.values[i]
                rc[self.evalExpr(k,temp_scope)] = self.evalExpr(v,temp_scope)
            return rc
        elif isinstance(expr,ast.JoinedStr):
            rc = ""
            for v in expr.values:
                if isinstance(v,ast.Constant):
                    rc += v.value
                elif isinstance(v,ast.FormattedValue):
                    rc += self.evalExpr(v.value,temp_scope)
            return rc
        return
    def evalStmt(self,stmt:ast.stmt,temp_scope = None):
        if isinstance(stmt,ast.Return):
            self.willReturn = True
            if stmt.value is not None:
                self.returnVal = self.evalExpr(stmt.value,temp_scope)
            else:
                self.returnVal =  None
        elif isinstance(stmt,ast.FunctionDef):
            self.symTable[stmt.name] = stmt
        elif isinstance(stmt,ast.ClassDef):
            self.error(stmt,"Class defs are not supported in AUTOM.")
        elif isinstance(stmt,ast.AnnAssign):

            name:ast.Name = stmt.target
            if not isinstance(name,ast.Name):
                self.error(name,"Variable EXPR must be an identifier")
                
            if stmt.value is not None:
                self.symTable[name.id] = self.evalExpr(stmt.value,temp_scope)
            else: 
                self.symTable[name.id] = None
        elif isinstance(stmt,ast.Assign):
            if len(stmt.targets) > 1:
                self.error(stmt.targets,"Variable EXPR must be an identifier")
            
            name:ast.Name = stmt.targets[0]
            if not isinstance(name,ast.Name):
                self.error(name,"Variable EXPR must be an identifier")

            self.symTable[name.id] = self.evalExpr(stmt.value,temp_scope)
        elif isinstance(stmt,ast.AugAssign):
            name:ast.Name = stmt.target
            if not isinstance(name,ast.Name):
                self.error(name,"Variable EXPR must be an identifier")
            
            if isinstance(stmt.op,ast.Add):
                self.symTable[name.id] += self.evalExpr(stmt.value,temp_scope)
            elif isinstance(stmt.op,ast.Sub):
                self.symTable[name.id] += self.evalExpr(stmt.value,temp_scope)
            elif isinstance(stmt.op,ast.Mult):
                self.symTable[name.id] *= self.evalExpr(stmt.value,temp_scope)
            elif isinstance(stmt.op,ast.Div):
                self.symTable[name.id] /= self.evalExpr(stmt.value,temp_scope)
            
        elif isinstance(stmt,ast.If):
            if self.evalExpr(stmt.test,temp_scope):
                _temp_scope = {}
                if temp_scope is not None:
                    _temp_scope.update(temp_scope)
                for __stmt in stmt.body:
                    self.evalStmt(__stmt,_temp_scope)
                return
            elif isinstance(stmt.orelse[0],ast.If):
                _if = stmt.orelse[0]
                if self.evalExpr(_if.test,temp_scope):
                    _temp_scope = {}
                    _temp_scope.update(temp_scope)
                    for __stmt in _if.body:
                        if self.willReturn:
                            break
                        self.evalStmt(__stmt,_temp_scope)
                    return
            
        elif isinstance(stmt,ast.Expr):
            self.evalExpr(stmt.value,temp_scope)
        return

    def interp(self,m:ast.Module):
        
        for stmt in m.body:
            self.evalStmt(stmt)
        return
    def interpForProject(self,m:ast.Module):
        self.interp(m)
        return self.p
            
def generateInstallFile(project:Project,output_dir:str):
    stream = io.open(f"{output_dir}/AUTOMINSTALL","w")
    out = {
        "entries":project.install_rules
    }
    json.dump(out,stream,indent=2,sort_keys=True)

def generateProjectFiles(project:Project,mode:ProjectFileType,output_dir:str,options:"dict[str,Any]" = {}):
    targets = project.__targets__
    target_names = []
    for i in range(0, len(targets)-1):
        t = targets[i]
        for d in t.dependencies:
            try: 
                target_names.index(d)
            except ValueError:
                print(f"\u001b[31mERROR:\u001b[0m \"{d}\" is a dependency of target \"{t.name}\" but does not exist.")
                exit(1)
        target_names.append(t.name)
    """
    Do a quick dependency check
    """

    if mode == ProjectFileType.CMAKE:
        __CmakeGenerator__(targets, project).generate("./CMakeLists.txt")
    elif mode == ProjectFileType.GN:
        __GNGenerator__(targets).generate("./BUILD.gn")

    if len(project.install_rules) > 0:
        generateInstallFile(project,output_dir=output_dir)

    return





