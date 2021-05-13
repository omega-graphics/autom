import os,json,io
import argparse
import re as Regex
import shutil


var_regex = Regex.compile(r"\$\(INSTALL_PREFIX\)",Regex.MULTILINE | Regex.DOTALL)

class AUTOMInstall:
    installPrefix:str
    def __init__(self,installPrefix:str):
        self.installPrefix = installPrefix
    def processString(self,string:str) -> str:
        m = var_regex.match(string)
        if m is None:
            return string 
        else:
            return var_regex.sub(self.installPrefix,string)
    def process(self,stream:io.TextIOWrapper):
        j = json.load(stream)
        assert(j.get("entries"))
        entries:"list[dict]" = j.get('entries')
        for e in entries:
            assert(e.get("files"))
            assert(e.get("dest"))
            dest = self.processString(e.get("dest"))
            for f in e.get("files"):
                if not os.path.exists(dest):
                    os.makedirs(dest)
                shutil.copy2(f,os.path.join(dest,os.path.basename(f)))

                
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--install-prefix",type=str,dest="install_prefix")
    parser.add_argument("--config",type=str,required=False)
    args = parser.parse_args()

    if args.install_prefix is None:
        raise "Install Prefix Not Provided.\nExiting..."

    autom_install_file:str

    if args.config is not None:
       autom_install_file = os.path.join(args.config,"AUTOMINSTALL")
    else:
        autom_install_file ="./AUTOMINSTALL"


    if os.path.exists(autom_install_file):
       AUTOMInstall(args.install_prefix).process(io.open(autom_install_file,"r"))
    else: 
        raise FileNotFoundError(autom_install_file)

if __name__ == "__main__":
    main()