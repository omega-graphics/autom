import os,ast
from typing import Any
from .autom_types import *
from .bridge import *

import pathlib

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

    def evalPropChange(self,t_name:str,prop_name:str,data:Any,expr:ast.Expr,append:bool=False):
    
        def append_prop(prop,data):
            if append:
                prop += data 
            else:
                prop = data
            return prop
        
        # Config Props
        for c in self.p.__configs__:
            if c == t_name:
                _conf = self.p.__configs__[c]
                if prop_name == "libs":
                    _conf.libs = append_prop(_conf.libs,data)
                    return
                elif prop_name == "configs":
                    _conf.configs = append_prop(_conf.configs,data)
                elif prop_name == "include_dirs":
                    _conf.include_dirs = append_prop(_conf.include_dirs,data)

        # Target Properties
        for t in self.p.__targets__:
            if t.name == t_name:
                    if isinstance(t,AppleApplicationBundle):
                        if prop_name == "info_plist":
                            t.plist = append_prop(t.plist,data)
                            return
                    if isinstance(t,AppleFrameworkBundle) or isinstance(t,AppleApplicationBundle):
                        if prop_name == "resources":
                            t.resources = append_prop(t.resources,resolve_resources(data))
                            # print(t.resources)
                            return
                        elif prop_name == "embedded_frameworks":
                            t.embedded_frameworks = append_prop(t.embedded_frameworks,data)
                            return
                    
                    if isinstance(t,AppleFrameworkBundle):
                        if prop_name == "embedded_libs":
                            t.embedded_libs = append_prop(t.embedded_libs,data)
                            return

                    if prop_name == "cflags":
                        t.cflags = append_prop(t.cflags,data)
                    elif prop_name == "cxxflags":
                        t.cxxflags = append_prop(t.cxxflags,data)
                    elif prop_name == "objcflags":
                        t.objcflags = append_prop(t.objcflags,data) 
                    elif prop_name == "objcxxflags":
                        t.objcxxflags = append_prop(t.objcxxflags,data) 
                    elif prop_name == "ldflags":
                        t.ldflags = append_prop(t.ldflags,data) 
                    elif prop_name == "defines":
                        t.defines = append_prop(t.defines,data) 
                    elif prop_name == "include_dirs":
                        t.include_dirs = append_prop(t.include_dirs,resolve_resources(data))
                    elif prop_name == "libs":
                        t.libs = append_prop(t.libs,data) 
                    elif prop_name == "lib_dirs":
                        t.lib_dirs = append_prop(t.lib_dirs,resolve_resources(data))
                    elif prop_name == "frameworks":
                        t.frameworks = append_prop(t.frameworks,data) 
                    elif prop_name == "framework_dirs":
                        t.framework_dirs = append_prop(t.framework_dirs,resolve_resources(data))
                    elif prop_name == "configs":
                        confs:"list[str]" = data
                        for conf in confs:
                            t.resolveConfig(self.p.__configs__[conf],self.p.__configs__)
                    else:
                        self.error(expr.func,f"Cannot set property `{prop_name}` on target `{t.name}`")
        
        

        return    
        

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
                    self.p.add_targets(l=[Executable(name=t_name,source_files=srcs,deps=deps,output_dir=output_dir)])
                    return
                elif expr.func.id == "StaticLibrary" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(l=[Library(name=t_name,source_files=srcs,deps=deps,output_dir=output_dir,shared=False)])
                    return
                elif expr.func.id == "SharedLibrary" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(l=[Library(name=t_name,source_files=srcs,deps=deps,output_dir=output_dir,shared=True)])
                    return
                elif expr.func.id == "SourceSet" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    self.p.add_targets(l=[SourceSet(name=t_name,source_files=srcs,deps=deps)])
                    return
                elif expr.func.id == "Group" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    self.p.add_targets(l=[Group(name=t_name,deps=deps)])
                    return
                elif expr.func.id == "AppBundle" and not self.inInterfaceFileTop:
                    if not AUTOM_LANG_SYMBOLS["is_mac"]:
                        self.error(expr.func,"AppBundle target can only be declared if target os is macOS or iOS")
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(l=[AppleApplicationBundle(t_name,srcs,deps,output_dir)])
                    return
                elif expr.func.id == "FrameworkBundle" and not self.inInterfaceFileTop:
                    if not AUTOM_LANG_SYMBOLS["is_mac"]:
                        self.error(expr.func,"FrameworkBundle target can only be declared if target os is macOS or iOS")
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    version:str = self.evalExpr(expr.args[4],temp_scope)
                    self.p.add_targets(l=[AppleFrameworkBundle(t_name,srcs,deps,version,output_dir)])
                    return
                elif expr.func.id == "Script" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    script_n:str = self.evalExpr(expr.args[3],temp_scope)
                    script_args:"list[str]" = self.evalExpr(expr.args[4],temp_scope)
                    outputs:"list[str]" = self.evalExpr(expr.args[5],temp_scope)
                    self.p.add_targets(l=[Script(name=t_name,source_files=srcs,dependencies=deps,script=script_n,args=script_args,outputs=outputs)])
                    return 
                elif expr.func.id == "Copy" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(l=[Copy(t_name,srcs,deps,output_dir)])
                    return
                elif expr.func.id == "Config" and not self.inInterfaceFileTop:
                    _name:str = self.evalExpr(expr.args[0],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    include_dirs:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    defines:"list[str]" = self.evalExpr(expr.args[3],temp_scope)
                    self.p.__configs__[_name] = TargetConfig(_name,deps,include_dirs,defines)
                    return
                elif expr.func.id == "set_property" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    prop_name:str = self.evalExpr(expr.args[1],temp_scope)
                    data:Any = self.evalExpr(expr.args[2],temp_scope)

                    self.evalPropChange(t_name,prop_name,data,expr,False)
                    
                    return
                elif expr.func.id == "append_property" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    prop_name:str = self.evalExpr(expr.args[1],temp_scope)
                    data:Any = self.evalExpr(expr.args[2],temp_scope)

                    self.evalPropChange(t_name,prop_name,data,expr,True)
                    
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

                    os.chdir(os.path.dirname(os.path.abspath(file)))

                    __module = ast.parse(data,file)

                    self.interp(__module)

                    os.chdir(prior_dir)

                    self.inRootFile = prior_0
                    self.inInterfaceFileTop = prior_1
                    return
                elif expr.func.id == "configure":
                    file:str = os.path.abspath(self.evalExpr(expr.args[0],temp_scope))
                    output_file:str = self.evalExpr(expr.args[1],temp_scope)
                    configure(file,output_file,self.symTable,atMode=True)
                    return None
                # FS Functions
                elif expr.func.id == "glob":
                    pattern:str = self.evalExpr(expr.args[0],temp_scope)
                    return glob.glob(pattern)
                elif expr.func.id == "abspath":
                    path:str = self.evalExpr(expr.args[0],temp_scope)
                    return os.path.abspath(path)
                elif expr.func.id == "pathdir":
                    path:str = self.evalExpr(expr.args[0],temp_scope)
                    return os.path.dirname(path)
                elif expr.func.id == "pathresolve":
                    path:str = self.evalExpr(expr.args[0],temp_scope)
                    return os.path.normpath(path)
                elif expr.func.id == "pathjoin":
                    path:str = self.evalExpr(expr.args[0],temp_scope)
                    kwargs:str = self.evalExpr(expr.args[1],temp_scope)
                    return os.path.join(path,kwargs)
                elif expr.func.id == "pathext":
                    path:str = self.evalExpr(expr.args[0],temp_scope)
                    return os.path.splitext(path)[1]
                elif expr.func.id == "pathbase":
                    path:str = self.evalExpr(expr.args[0],temp_scope)
                    return os.path.basename(path)
                elif expr.func.id == "assert_expr":
                    _expr:Any = self.evalExpr(expr.args[0],temp_scope)
                    if not _expr:
                        print(f"\x1b[31mASSERT FAILED:\x1b[0m -> LOC {expr.lineno}:{expr.col_offset}")
                        exit(1)
                    return None
                elif expr.func.id == "print":
                    obj:Any = self.evalExpr(expr.args[0],temp_scope)
                    print(obj)
                    return None
                elif expr.func.id == "program_exists":
                    prog:str = self.evalExpr(expr.args[0],temp_scope)
                    print(f"Checking program exists `{prog}`")
                    v = shutil.which(prog) is not None
                    if v:
                        print(f"Checking program exists `{prog}` -- found")
                    else:
                        print(f"Checking program exists `{prog}` -- not found")
                    return v
                # Find Funcs
                elif expr.func.id == "find_prog":
                    prog:str = self.evalExpr(expr.args[0],temp_scope)
                    r = shutil.which(prog)
                    if r is None:
                        r = ""
                    return os.path.realpath(r)
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
            elif expr.id == "is_linux":
                return AUTOM_LANG_SYMBOLS['is_linux'] 

            # 2. - Eval Temp Scope Identifiers
            if temp_scope is not None:
                if temp_scope.get(expr.id) is not None:
                    return temp_scope[expr.id]

            # 3. - Eval Global Identifiers

            if self.symTable.get(expr.id) is not None:
                return self.symTable[expr.id]
            
            self.error(expr,f"Unknown Identifier `{expr.id}`")
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
        elif isinstance(expr,ast.Subscript):
            # Only Supports indexing of lists
            value_ = self.evalExpr(expr.value,temp_scope)
            # args:ast.Tuple = expr.slice
            idx_val = self.evalExpr(expr.slice,temp_scope)
            # print(idx_val)
            return value_[idx_val]
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
            elif len(stmt.orelse) > 0:
                if isinstance(stmt.orelse[0],ast.If):
                    _temp_scope = {}
                    if temp_scope is not None:
                        _temp_scope.update(temp_scope)
                    self.evalStmt(stmt.orelse[0],_temp_scope)
                else:
                    _temp_scope = {}
                    if temp_scope is not None:
                        _temp_scope.update(temp_scope)
                    for __stmt in stmt.orelse:
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