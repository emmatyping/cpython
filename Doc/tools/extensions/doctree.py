
from __future__ import annotations

from typing import TYPE_CHECKING

from docutils import nodes
from sphinx import addnodes

if TYPE_CHECKING:
    from sphinx.application import Sphinx
    from sphinx.util.typing import ExtensionMetadata

class Visitor:
    def __init__(self):
        self.indent = 0

    def visit(self, node):
        print(' '*self.indent + node.__class__)
        self.indent += 4
        for child in node.findall(include_self=False):
            self.visit(child)
        self.indent -= 4


def get_doctree_ast(app: Sphinx, doctree: nodes.document, doc_name: str) -> None:
    visitor = Visitor()
    if doc_name == "library/sys.monitoring":
        '''
        Notes: prefix is desc_addname for the prefix (e.g. 'sys.monitoring.') then 
        desc_name with the final part, e.g. 'use_tool_id'.

        structure is:
        desc_addname
            <prefix>
        desc_name
            <name>
        paramlist
            desc_parameter
                desc_sig_name
                    <name>
                desc_sig_punctuation
                    ':'
                desc_sig_space
                    ' '
                desc_sig_name
                    reference
                        <name>
        desc_returns
            reference
                <name>
        '''
        breakpoint()

def setup(app: Sphinx) -> ExtensionMetadata:
    app.connect("doctree-resolved", get_doctree_ast)

    return {
        "version": "1.0",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
