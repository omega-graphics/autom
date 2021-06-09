"""AUTOM Build Tool

"""

import ast
from enum import Enum
import io
import json
from typing import Any
import runpy
import os
import re as Regex
import glob


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
    "generateProjectFiles"
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


class Project:
    name: str
    version: str
    __targets__: "list[Target]"
    
    def __init__(self, name : str, version :str):
        self.name = name
        self.version = version 
        self.__targets__ = []

    def add_dir(self, name: str):
        n = os.path.abspath(name) + "/AUTOM"
        t = runpy.run_path(os.path.abspath(name) + "/AUTOM")
        try:
            __n_t = t['targets']
            for t in __n_t:
                t.set_current_dir(name)
            self.__targets__ += __n_t
        except KeyError:
            print(f"\u001b[31mERROR:\u001b[0m The Variable targets is not defined in file scope:{n}")
            exit(1)
        return

    def add_targets(self,list:"list[Target]"):
        self.__targets__ += list
        return


class TargetType(Enum):
    EXECUTABLE = 0,
    LIBRARY = 1,
    SCRIPT = 2,
    COPY = 3,
    IMPORTED_LIBRARY = 4
    APPLE_FRAMEWORK = 5,
    APPLE_APP_BUNDLE = 6,


class Target :
    """
    Target Declaration
    """
    name: str
    __type__: TargetType
    source_files: "list[str]"
    dependencies: "list[str]"
    include_dirs: "list[str]"

    def __init__(self,name:str,_type:TargetType,source_files:"list[str]",dependencies:"list[str]"):
        self.name = name
        self.__type__ = _type
        self.source_files = source_files
        self.dependencies = dependencies
        self.include_dirs = []

    def set_current_dir(self,newCurrentDir:str):
        for s in self.source_files:
           i =  self.source_files.index(s)
           n = newCurrentDir +  "/" + os.path.basename(s)
           self.source_files[i] = n

    def add_include_dirs(self,includeDirs:"list[str]"):
        self.include_dirs += includeDirs


class Executable(Target):
    """
    Defines an executable target
    """
    output_dir:str

    def __init__(self,name:str,source_files:"list[str]",dependencies:"list[str]",output_dir:str):
        super(Executable,self).__init__(name,TargetType.EXECUTABLE,source_files,dependencies)
        self.output_dir = output_dir


class AppleApplicationBundle(Executable):
    """
    Defines a Apple App Bundle
    """
    def __init__(self,name:str,source_files:"list[str]",dependencies:"list[str]",output_dir:str):
        super(AppleApplicationBundle,self).__init__(name,source_files,dependencies,output_dir=output_dir)
        self.__type__ = TargetType.APPLE_APP_BUNDLE
        self.output_dir = output_dir

class Library(Target):
    """
    Defines a static or shared library target
    """
    shared:bool

    def __init__(self,name:str,source_files:"list[str]",dependencies:"list[str]",output_dir:str,shared:bool):
        super(Library,self).__init__(name,TargetType.LIBRARY,source_files,dependencies)
        self.output_dir = output_dir
        self.shared = shared

class AppleFrameworkBundle(Library):
    """
    Defines a Apple Framework Bundle
    """
    def __init__(self,name:str,source_files:"list[str]",dependencies:"list[str]",output_dir:str):
        super(AppleFrameworkBundle,self).__init__(name,source_files,dependencies,True)
        self.__type__ = TargetType.APPLE_FRAMEWORK
        self.output_dir = output_dir



class Script(Target):
    script:str

    def __init__(self,name:str,source_files:"list[str]",dependencies:"list[str]",script:str):
        super(Script,self).__init__(name,TargetType.SCRIPT,source_files,dependencies)
        self.script = script


class Copy(Target):
    output_dir:str

    def __init__(self,name:str,source_files:"list[str]",dependencies:"list[str]",output_dir:str):
        super(Copy,self).__init__(name,TargetType.COPY,source_files,dependencies)
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


def cmake_bridge():
    return


def gn_bridge():
    return


AUTOM_LANG_SYMBOLS = {
    "Project": Project,
    "Executable": Executable,
    "Library": Library,
    "path": os.path,
    "ProjectFileType": ProjectFileType,
    "gn_bridge": gn_bridge,
    "cmake_bridge": cmake_bridge,
    "glob": glob.glob
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

    def generate(self,out_file:str):
        out_dir = os.path.dirname(out_file)
        if os.path.exists(out_dir) == False: 
            os.makedirs(out_dir)
        stream  = open(out_file,"w")
        stream.write("# This File Was Generated by AUTOM Build Tool. Do NOT EDIT!!!\n")
        stream.write('import("//gn-utils/Utils.gni")\n')
        print(self.targets)
        for t in self.targets:
            if t.__type__.value == TargetType.EXECUTABLE.value:
                stream.write(f"executable(\"{t.name}\")" + "{\n")
                stream.write("  include_dirs = {}\n".format(json.dumps(t.include_dirs)))
                stream.write("  sources = {}\n".format(json.dumps(t.source_files)))
                stream.write("  deps = {}".format(json.dumps(self.__formatDeps(t.dependencies))))

                # stream.write("deps = ")
                stream.write("\n}")
            elif t.__type__.value == TargetType.LIBRARY.value:
                if t.shared:
                    stream.write(f"shared_library(\"{t.name}\")" + "{\n")
                else: 
                    stream.write(f"static_library(\"{t.name}\")" + "{\n")
                stream.write("  include_dirs = {}\n".format(json.dumps(t.include_dirs)))
                stream.write("  sources = {}\n".format(json.dumps(t.source_files)))
                stream.write("  deps = {}".format(json.dumps(self.__formatDeps(t.dependencies))))
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

# def __unwrap_sources(_ast:ast.List) -> list[str]:
    
#     return

            
    


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
        __CmakeGenerator__(targets, project).generate(output_dir + "/CMakeLists.txt")
    elif mode == ProjectFileType.GN:
        __GNGenerator__(targets).generate(output_dir + "/BUILD.gn")
    return



