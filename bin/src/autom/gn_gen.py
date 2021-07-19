from .autom_types import *

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
        # print(self.targets)
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
            elif t.__type__.value == TargetType.SOURCE_SET.value:
                stream.write(f"source_set(\"{t.name}\")" + "{\n")
                self.writeStandardTargetProps(t,stream)
                stream.write("\n}")
            elif t.__type__.value == TargetType.GROUP.value:
                stream.write(f"group(\"{t.name}\")" + "{\n")
                stream.write("  public_deps = {}\n".format(json.dumps(self.__formatDeps(t.dependencies))))
                stream.write("\n}")
            elif t.__type__.value == TargetType.APPLE_APP_BUNDLE.value:
                stream.write(f"mac_app_bundle(\"{t.name}\")" + "{\n")
                stream.write(f"  plist = \"{t.plist}\"\n")
                stream.write("  resources = {}\n".format(json.dumps(t.resources)))
                if len(t.embedded_frameworks) > 0:
                    stream.write("  embedded_frameworks = {}\n".format(json.dumps(t.embedded_frameworks)))
                self.writeStandardTargetProps(t,stream)
                stream.write("\n}")
            elif t.__type__.value == TargetType.APPLE_FRAMEWORK.value:
                stream.write(f"mac_framework_bundle(\"{t.name}\")" + "{\n")
                stream.write(f"  version = \"{t.version}\"\n")
                stream.write("  resources = {}\n".format(json.dumps(t.resources)))
                if len(t.embedded_frameworks) > 0:
                    stream.write("  embedded_frameworks = {}\n".format(json.dumps(t.embedded_frameworks)))
                if len(t.embedded_libs) > 0:
                    stream.write("  embedded_libs = {}\n".format(json.dumps(t.embedded_libs)))
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
            elif t.__type__.value == TargetType.COPY.value:
                stream.write(f"copy(\"{t.name}\")" + "{\n")
                stream.write("  public_deps = {}\n".format(json.dumps(self.__formatDeps(t.dependencies))))
                stream.write("  sources = {}\n".format(json.dumps(t.source_files)))
                stream.write("  outputs = [{}]".format(json.dumps(f"{t.output_dir}" + "/{{source_file_part}}")))
                stream.write("\n}")
            stream.write("\n\n")
        stream.close()