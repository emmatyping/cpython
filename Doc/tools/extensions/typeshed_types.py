"""Override Python Domain types to pull signature information from typeshed."""
from __future__ import annotations
import ast
from typing import TYPE_CHECKING
import pathlib

import typeshed_client
from sphinx.domains.python import (
    PyFunction,
    PyDecoratorFunction,
    PyClasslike,
    PyMethod,
    PyClassMethod,
    PyStaticMethod,
    PyDecoratorMethod,
    PyAttribute,
    PyProperty,
    PythonDomain,
)

if TYPE_CHECKING:
    from sphinx.application import Sphinx
    from sphinx.util.typing import ExtensionMetadata
    from sphinx.addnodes import desc_signature

ignored_modules = (
    "test",
)

search_context = typeshed_client.get_search_context(
    typeshed=pathlib.Path().home() / "typeshed" / "stdlib",
    version=(3, 14),
    platform="linux",
)

def ast_from_resolved_name(resolved_name: typeshed_client.resolver.ResolvedName) -> ast.AST:
    if isinstance(resolved_name, typeshed_client.NameInfo):
        return resolved_name.ast
    elif isinstance(resolved_name, typeshed_client.ImportedInfo):
        return resolved_name.info.ast
    else:
        return None

class TypedPyFunction(PyFunction):
    
    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get('module', self.env.ref_context.get('py:module'))
        resolver = typeshed_client.Resolver(search_context)
        if modname and not any((modname.startswith(name) for name in ignored_modules)):
            try:
                if modname is not None:
                    qualname = f"{modname}.{fullname}"
                else:
                    qualname = fullname
                resolved_name = resolver.get_fully_qualified_name(qualname)
                resolved_ast = ast_from_resolved_name(resolved_name)
                if resolved_ast:
                    # update signode with the proper signature from resolved_ast
                    pass
            except Exception:
                print(f"Exception with: {modname}.{fullname}")
                raise
            
        return fullname, clsname

overriden_directives = {
    "function": TypedPyFunction,
    "class": PyClasslike,
    "method": PyMethod,
    "classmethod": PyClassMethod,
    "staticmethod": PyStaticMethod,
    "attribute": PyAttribute,
    "property": PyProperty,
    "decorator": PyDecoratorFunction,
    "decoratormethod": PyDecoratorMethod,
}


def setup(app: Sphinx) -> ExtensionMetadata:
    for name, type_ in overriden_directives.items():
        app.add_directive_to_domain("py", name, type_, override=True)
    PythonDomain.directives.update(**overriden_directives)
    app.add_domain(PythonDomain, override=True)
    return {
        "version": "0.1.0",
        "env_version": 4,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }