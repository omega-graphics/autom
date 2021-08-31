import textmate
import os

if __name__ == "__main__":
    textmate.TextMateGrammarProcessor().build_grammar("autom.yaml",True,"./vscode/syntaxes/Autom.tmLanguage.json")

    os.chdir("./vscode")
    os.system("npm install")
    os.system("npx vsce package")
    os.chdir("../")