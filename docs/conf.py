import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__),"_ext"))


project = "AUTOM Build System"

author = "Omega Graphics"

copyright = "2021, Omega Graphics"

html_theme_path = ["_themes"]

html_theme = "autom"

extensions = ["autom-sphinx"]

highlight_language = "autom"

html_codeblock_linenos_style = 'inline'