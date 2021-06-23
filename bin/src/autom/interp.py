import os,ast
from typing import Any
from .autom_types import *
from .bridge import *

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
                elif expr.func.id == "Copy" and not self.inInterfaceFileTop:
                    t_name:str = self.evalExpr(expr.args[0],temp_scope)
                    srcs:"list[str]" = self.evalExpr(expr.args[1],temp_scope)
                    deps:"list[str]" = self.evalExpr(expr.args[2],temp_scope)
                    output_dir:str = self.evalExpr(expr.args[3],temp_scope)
                    self.p.add_targets(list=[Copy(t_name,srcs,deps,output_dir)])
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
                                elif prop_name == "libs":
                                    t.libs = data 
                                elif prop_name == "lib_dirs":
                                    t.lib_dirs = resolve_resources(data)
                                elif prop_name == "frameworks":
                                    t.frameworks = data 
                                elif prop_name == "framework_dirs":
                                    t.framework_dirs = resolve_resources(data)
                                elif prop_name == "configs":
                                    confs:"list[str]" = data
                                    for conf in confs:
                                        t.resolveConfig(self.p.__configs__[conf])
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
            elif len(stmt.orelse) > 0:
                if isinstance(stmt.orelse[0],ast.If):
                    _if = stmt.orelse[0]
                    if self.evalExpr(_if.test,temp_scope):
                        _temp_scope = {}
                        if temp_scope is not None:
                            _temp_scope.update(temp_scope)
                        for __stmt in _if.body:
                            if self.willReturn:
                                break
                            self.evalStmt(__stmt,_temp_scope)
                        return
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