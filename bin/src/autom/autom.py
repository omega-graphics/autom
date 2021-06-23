"""AUTOM Build Tool

"""

import json,ast
from .cmake_gen import __CmakeGenerator__
from .gn_gen import __GNGenerator__
from .autom_types import *
from .interp import AUTOMInterp

def execForProject(f:str,t:ProjectFileType,output_dir:str):
    _in = io.open(f,"r")
    m = ast.parse(_in.read(),f)
    interp = AUTOMInterp()
    if t == ProjectFileType.GN:
        interp.symTable["gen_root_out_dir"] = "$root_out_dir"
    else:
        interp.symTable["gen_root_out_dir"] = "${CMAKE_BINARY_DIR}"
    interp.symTable["root_out_dir"] = output_dir
    return interp.interpForProject(m)

            
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





