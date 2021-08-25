import sphinx.application
import sphinx.domains

from docutils import nodes
from docutils.parsers.rst import Directive


class AutomBlock(Directive):

    def run(self):
        paragraph_node = nodes.paragraph(text="Hello World")
        return [paragraph_node]


def setup(app:sphinx.application.Sphinx):
    app.add_directive("autom-block", AutomBlock)

    return {
        'version': '0.1',

        'parallel_read_safe': True,

        'parallel_write_safe': True,
    }