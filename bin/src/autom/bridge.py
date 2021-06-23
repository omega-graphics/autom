import os
from typing import Union
from .autom_types import *



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




name_regex_sub = r"[\w\-]+"
target_name_regex = Regex.compile(rf"\/\/(?:(?::({name_regex_sub}))|(?:[\w\/\-]+:({name_regex_sub})))",Regex.MULTILINE)


def gn_get_target_name(t_name:str) -> str:
    return target_name_regex.fullmatch(t_name).group(1)

regular_file_regex = Regex.compile(rf"\/\/([\w\-\/\.]+)",Regex.MULTILINE)
output_file_regex = Regex.compile(rf"\/\/out\/([\w\-\/\.]+)",Regex.MULTILINE)

def resolve_srcs(srcs:"list[str]",regex:Regex.Pattern):
    for i in range(len(srcs)):
        s = srcs[i]
        m = regex.fullmatch(s)
        srcs[i] = m.group(1)
    return srcs

def gn_bridge(dir:str,args:str,output_dir:str) -> "list[Target]":
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
    print(f"Importing GN Project ({dir})")
    os.system(f"gn gen out --args={args} --ide=json")
    j:"dict[str,dict[str,dict[str,Union[str,list[str]]]]]" = json.load(io.open("./out/project.json","r"))
    _targets = j["targets"]
    targets:"list[Target]" = []

    def push_target_to_queue(t:Target,json:"dict[str,Union[str,list[str]]]"):
        t.cflags = json["cflags"]
        t.cflags = json["cflags_cc"]
        t.cflags = json["cflags_objc"]
        t.cflags = json["cflags_objcc"]
        t.include_dirs = json["include_dirs"]
        t.defines = json["defines"]
        targets.append(t)

    for tname in _targets:
        _t = _targets[tname]
        deps = _t["deps"].copy()
        deps = resolve_srcs(deps,target_name_regex)
            
        if _t["type"] == "static_library":
            t = Library(gn_get_target_name(tname),
                source_files=resolve_srcs(_t["sources"],regular_file_regex),
                deps=deps,
                output_dir=output_file_regex.fullmatch(_t["output_dir"]).group(1),shared=False)
            push_target_to_queue(t)
        elif _t["type"] == "shared_library":
            t = Library(gn_get_target_name(tname),
                source_files=resolve_srcs(_t["sources"],regular_file_regex),
                deps=deps,
                output_dir=output_file_regex.fullmatch(_t["output_dir"]).group(1),shared=True)
            push_target_to_queue(t)
        elif _t["type"] == "executable":
            t = Executable(gn_get_target_name(tname),
                source_files=resolve_srcs(_t["sources"],regular_file_regex),
                deps=deps,
                output_dir=output_file_regex.fullmatch(_t["output_dir"]).group(1),shared=True)
            push_target_to_queue(t)
        elif _t["type"] == "action":
            t = Script(gn_get_target_name(tname),
                source_files=resolve_srcs(_t["sources"],regular_file_regex),
                dependencies=deps,
                output_dir=resolve_srcs(_t["outputs"],output_file_regex),script=regular_file_regex.fullmatch(_t["script"]).group(1),args=_t["args"])
            push_target_to_queue(t)
        elif _t["type"] == "source_set":
            t = SourceSet(gn_get_target_name(tname),
                source_files=resolve_srcs(_t["sources"],regular_file_regex),
                deps=deps)
            push_target_to_queue(t)
        # elif _t["type"] == "create_bundle":

    
        

    declared_targets = []
    sorted_targets:"list[Target]" = []

    def push_target_by_name(d:str):
        for t in targets:
            if t.name == d:
                for d in t.dependencies:
                    try:
                        declared_targets.index(d)
                    except ValueError:
                        push_target_by_name(d)
                sorted_targets.append(t)

    for t in targets:
        for d in t.dependencies:
            try:
                declared_targets.index(d)
            except ValueError:
                push_target_by_name(d)
        sorted_targets.append(t)
        t.output_dir = os.path.join("$root_out_dir",t.output_dir)
        break

    





    os.remove("./.gn")
    os.remove("./gn-utils")

    os.chdir(prev_dir)

    return sorted_targets