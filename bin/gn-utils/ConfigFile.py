
import os
import sys
import re as Regex

define_regex = Regex.compile(r"^-D([\w|-|_|\+|\-|\"]+)=",Regex.MULTILINE)
# import out
import shutil

at_mode = False
class ConfigFileProcessor:
    
    code:str
    defines:"dict[str,str]"
    def __init__(self,code:str,defines:"dict[str,str]"):
        self.code = code
        self.defines = defines
    def process(self):
        for d in self.defines:
            self.code = Regex.sub("@" + d+ "@",self.defines[d],self.code,flags=Regex.MULTILINE)
            



def main():

    args = sys.argv
    args.pop(0)
    # print(args)

    defines:"dict[str,str]" = {}
    input = None 
    output = None

    for arg in args:
        if arg.startswith("-D"):
            res = define_regex.match(arg)
            eqs_idx = arg.index("=")
            val:str = ""
            for i in range(eqs_idx + 1,len(arg)):
                val = f"{val}{arg[i]}"
            if res != None:
                defines[res.group(1)] = val
            # else:
            #     defines[res[0]] = ""
        elif arg == "--input" or arg == "-I":
            idx = args.index(arg)
            input = args[idx + 1]
        elif arg == "--output" or arg == "-O":
            idx = args.index(arg)
            output = args[idx + 1]
        elif arg == "--at":
            at_mode = True

    cwd = os.getcwd()
    if input != None and output != None:
        fstream = open(input,"r")
        config = ConfigFileProcessor(code=fstream.read(),defines=defines)
        config.process()
        f = open(output,"w")
        f.write(config.code)
        print("Sucessfully Configured File")

if __name__ == "__main__":
    main()