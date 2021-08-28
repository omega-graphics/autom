

__all__ = ["AutomLexer"]

import pygments
from pygments.formatters import TerminalFormatter
from pygments.formatters.terminal256 import Terminal256Formatter, TerminalTrueColorFormatter
from pygments.lexers.python import Python3Lexer
from pygments.lexer import ExtendedRegexLexer, words
from pygments import highlight, lex, token

class AutomLexer(ExtendedRegexLexer):
    name = 'AUTOM Language'
    aliases = ['autom']
    filenames = ['AUTOM.build','*.autom']

    tokens = {
        'root': [
            (r'\s+', token.Whitespace),
            (r'\n',token.Whitespace),
            (r'#.*$', token.Comment),
            (r'"[^"]*"',token.String),
            (r'\[|\]|{|}|,|\.|\(|\)|:',token.Punctuation),
            (words(('var', 'if','elif','else','func','import','load'),suffix=r'\b'), token.Keyword),
            (words(('true','false')),token.Keyword.Type),
            (r'\w+(?=\([^A-Z])', token.Name.Function),
            (r'\w+(?=\([^a-z])', token.Name.Class),
            (r'\w+(?=:)',token.Name.Other),
            (r'(?<=var\s)\w+',token.Name.Variable),
            (r'(?<!\.)\w+(?!:)',token.Name.Variable),
            (r'(?<=\.)\w+',token.Name.Property),
            (r'\w+', token.Text),
            (words(('=','+=','+','==')),token.Operator)
        ]
    }

# print(highlight(open("../AUTOM.build","r").read(),AutomLexer(),TerminalTrueColorFormatter()))