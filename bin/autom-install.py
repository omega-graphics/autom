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
    def process(self):
        j = json.load(io.open("./AUTOMINSTALL","r"))
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
    args = parser.parse_args()

    if args.install_prefix is None:
        raise "Install Prefix Not Provided.\nExiting..."

    if os.path.exists("./AUTOMINSTALL"):
       AUTOMInstall(args.install_prefix).process()
    else: 
        raise FileNotFoundError("./AUTOMINSTALL")

if __name__ == "__main__":
    main()