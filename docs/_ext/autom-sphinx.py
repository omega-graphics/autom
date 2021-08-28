import docutils
import pygments
from pygments.lexer import RegexLexer, bygroups, words
from pygments.formatters import NullFormatter
import sphinx.application
import sphinx.domains
import pygments.lexers
import pygments.lexers._mapping

import autom


import io

from pygments import highlight, lex, token


from docutils import nodes
from docutils.parsers.rst import Directive

import sys 
import os 

sys.path.append(os.path.dirname(__file__))

pygments.lexers._mapping.LEXERS["AutomLexer"] = ('autom', 'AUTOM Language', ('autom',), ('AUTOM.build', '*.build'), ('text/autom',))

# for (name, aliases,filenames,mimetypes) in pygments.lexers.get_all_lexers():
#     print(aliases)




# print(lex(io.open("../AUTOM.build","r").read(),AutomLexer()))
# print(highlight(io.open("../AUTOM.build","r").read(),AutomLexer(),NullFormatter()))
from docutils.nodes import *

class AutomBlock(Directive):

    def run(self):
        
        paragraph_node = Paragraph(text="Hello World")
        return [paragraph_node]


def setup(app:sphinx.application.Sphinx):
    app.add_directive("autom-block", AutomBlock)

    return {
        'version': '0.1',

        'parallel_read_safe': True,

        'parallel_write_safe': True,
    }