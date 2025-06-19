"""Override Python Domain types to pull signature information from typeshed."""

from __future__ import annotations
import ast
import functools
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
from sphinx.addnodes import (
    desc_signature,
    desc_signature_line,
    desc_content,
    desc_name,
    desc_parameterlist,
    desc_parameter,
    desc_annotation,
)
from docutils import nodes

if TYPE_CHECKING:
    from sphinx.application import Sphinx
    from sphinx.util.typing import ExtensionMetadata

ignored_modules = ("test",)

search_context = typeshed_client.get_search_context(
    typeshed=pathlib.Path().home() / "typeshed" / "stdlib",
    version=(3, 14),
    platform="linux",
)


def ast_from_resolved_name(
    resolved_name: typeshed_client.resolver.ResolvedName,
) -> typeshed_client.NameInfo:
    """Extract ast.AST node from resolved typeshed_client name."""
    if isinstance(resolved_name, typeshed_client.NameInfo):
        return resolved_name
    elif isinstance(resolved_name, typeshed_client.ImportedInfo):
        return resolved_name.info
    else:
        return None


@functools.cache
def get_typeshed_nameinfo(fullname: str, modname: str) -> typeshed_client.NameInfo:
    """Get ast.AST from typeshed."""
    if not modname:
        return None

    try:
        resolver = typeshed_client.Resolver(search_context)
        qualname = f"{modname}.{fullname}" if modname else fullname
        resolved_name = resolver.get_fully_qualified_name(qualname)
        resolved_ast = ast_from_resolved_name(resolved_name)
        return resolved_ast
    except Exception as e:
        print(f"Exception getting typeshed signature for {modname}.{fullname}: {e}")

    return None


def ast_to_sig(func_ast: ast.AST, fullname: str) -> desc_signature_line:
    signode = desc_signature_line()
    name_node = desc_name(text=fullname)
    signode += name_node

    if func_ast.args:
        # Add parameter list
        paramlist = desc_parameterlist()

        # Process positional only arguments
        for posarg in func_ast.args.posonlyargs:
            param = desc_parameter()
            param += nodes.Text(posarg.arg)

            if posarg.annotation:
                param += nodes.Text(": ")
                annotation_node = desc_annotation()
                annotation_node += nodes.Text(ast.unparse(posarg.annotation))
                param += annotation_node

            paramlist += param

        # Process arguments
        for arg in func_ast.args.args:
            param = desc_parameter()
            param += nodes.Text(arg.arg)

            if arg.annotation:
                param += nodes.Text(": ")
                annotation_node = desc_annotation()
                annotation_node += nodes.Text(ast.unparse(arg.annotation))
                param += annotation_node

            paramlist += param

        # Handle *args
        if func_ast.args.vararg:
            param = desc_parameter()
            param += nodes.Text(f"*{func_ast.args.vararg.arg}")
            if func_ast.args.vararg.annotation:
                param += nodes.Text(": ")
                annotation_node = desc_annotation()
                annotation_node += nodes.Text(
                    ast.unparse(func_ast.args.vararg.annotation)
                )
                param += annotation_node
            paramlist += param

        # Handle keyword-only arguments
        for arg in func_ast.args.kwonlyargs:
            param = desc_parameter()
            param += nodes.Text(arg.arg)

            if arg.annotation:
                param += nodes.Text(": ")
                annotation_node = desc_annotation()
                annotation_node += nodes.Text(ast.unparse(arg.annotation))
                param += annotation_node

            paramlist += param

        # Handle **kwargs
        if func_ast.args.kwarg:
            param = desc_parameter()
            param += nodes.Text(f"**{func_ast.args.kwarg.arg}")
            if func_ast.args.kwarg.annotation:
                param += nodes.Text(": ")
                annotation_node = desc_annotation()
                annotation_node += nodes.Text(
                    ast.unparse(func_ast.args.kwarg.annotation)
                )
                param += annotation_node
            paramlist += param

        signode += paramlist

    # Add return annotation
    if func_ast.returns:
        signode += nodes.Text(" -> ")
        ret_annotation = desc_annotation()
        ret_annotation += nodes.Text(ast.unparse(func_ast.returns))
        signode += ret_annotation
    return signode


def update_signature_node(
    signode: desc_signature, name_info: typeshed_client.NameInfo, fullname: str
) -> None:
    if not isinstance(signode, desc_signature):
        raise TypeError(f"Invalid signature item, got {type(signode)}")

    if name_info is None:
        return

    if isinstance(name_info.ast, (ast.FunctionDef, ast.AsyncFunctionDef)):
        # it's an AST node, easy enough!
        typeshed_sigs = [name_info.ast]
    elif isinstance(name_info.ast, ast.ClassDef):
        # a class def for something documented as a function. We should look up
        # __new__, which may be overloaded
        typeshed_sigs = []
        for child in name_info.ast.body:
            if isinstance(child, ast.FunctionDef) and child.name == "__new__":
                typeshed_sigs.append(child)
    elif isinstance(name_info.ast, ast.Assign):
        # assignments are tricky, defer these for now.
        # the correct way to deal with them is probably to look up the name and
        # replace with whatever that gets back (maybe recursive call?)
        return
    elif isinstance(name_info.ast, ast.AnnAssign):
        # this handles cases such as
        # class TypeDef:
        #     __call__(...)
        # foo: TypeDef
        # TODO: look up the type annotation relative to the current context
        return
    elif isinstance(name_info.ast, typeshed_client.OverloadedName):
        typeshed_sigs = name_info.ast.definitions
    else:
        raise RuntimeError(
            f"Don't know how to handle {signode['module']}.{fullname} ast type {type(name_info.ast)}"
        )

    content = signode.next_node(desc_content)
    signode.clear()
    if len(typeshed_sigs) > 1:
        signode.is_multiline = True
    for entry in typeshed_sigs:
        signode += ast_to_sig(entry, fullname)
    if content:
        signode += content.deepcopy()


class TypedPyFunction(PyFunction):
    """Function directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        name_info = get_typeshed_nameinfo(fullname, modname)
        if name_info:
            update_signature_node(signode, name_info, fullname)

        return fullname, clsname


class TypedPyMethod(PyMethod):
    """Method directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        # For methods, we need to get the class context
        if clsname:
            method_qualname = f"{clsname}.{fullname}"
        else:
            method_qualname = fullname

        name_info = get_typeshed_nameinfo(method_qualname, modname)
        if name_info:
            update_signature_node(signode, name_info, fullname)

        return fullname, clsname


class TypedPyClassMethod(PyClassMethod):
    """Class method directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        if clsname:
            method_qualname = f"{clsname}.{fullname}"
        else:
            method_qualname = fullname

        name_info = get_typeshed_nameinfo(method_qualname, modname)
        if name_info:
            update_signature_node(signode, name_info, fullname)

        return fullname, clsname


class TypedPyStaticMethod(PyStaticMethod):
    """Static method directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        if clsname:
            method_qualname = f"{clsname}.{fullname}"
        else:
            method_qualname = fullname

        name_info = get_typeshed_nameinfo(method_qualname, modname)
        if name_info:
            update_signature_node(signode, name_info, fullname)

        return fullname, clsname


class TypedPyClasslike(PyClasslike):
    """Class directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        # For classes, we want to get the __init__ method signature
        init_qualname = f"{fullname}.__init__"
        name_info = get_typeshed_nameinfo(init_qualname, modname)
        if name_info:
            # Modify the signature to show class construction rather than __init__
            init_ast = name_info
            if (
                init_ast.args
                and init_ast.args.args
                and init_ast.args.args[0].arg == "self"
            ):
                # Remove 'self' parameter for class constructor display
                class_ast = ast.FunctionDef(
                    name=fullname,
                    args=ast.arguments(
                        args=init_ast.args.args[1:],  # Skip 'self'
                        defaults=init_ast.args.defaults,
                        kwonlyargs=init_ast.args.kwonlyargs,
                        kw_defaults=init_ast.args.kw_defaults,
                        vararg=init_ast.args.vararg,
                        kwarg=init_ast.args.kwarg,
                        posonlyargs=getattr(init_ast.args, "posonlyargs", []),
                    ),
                    decorator_list=[],
                    returns=None,  # Classes don't show return type
                    lineno=init_ast.lineno,
                )
                update_signature_node(signode, class_ast, fullname)

        return fullname, clsname


class TypedPyAttribute(PyAttribute):
    """Attribute directive with typeshed type information support."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        if clsname:
            qualname = f"{clsname}.{fullname}"
        else:
            qualname = f"{fullname}"

        name_info = get_typeshed_nameinfo(qualname, modname)
        if name_info:
            signode.clear()
            name_node = desc_name(text=fullname)
            signode += name_node
            signode += nodes.Text(": ")

            annotation_node = desc_annotation()
            annotation_node += nodes.Text(ast.unparse(name_info.ast.annotation))
            signode += annotation_node
        return fullname, clsname


class TypedPyProperty(PyProperty):
    """Property directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        if clsname:
            prop_qualname = f"{clsname}.{fullname}"
        else:
            prop_qualname = fullname

        name_info = get_typeshed_nameinfo(prop_qualname, modname)
        if name_info:
            # For properties, we typically want to show the return type
            # TODO(emmatyping): how to handle setters etc.
            prop_ast = name_info
            if prop_ast.returns:
                signode.clear()
                name_node = desc_name(text=fullname)
                signode += name_node
                signode += nodes.Text(": ")

                annotation_node = desc_annotation()
                annotation_node += nodes.Text(ast.unparse(prop_ast.returns))
                signode += annotation_node

        return fullname, clsname


# Decorator classes inherit the same functionality
class TypedPyDecoratorFunction(PyDecoratorFunction):
    """Decorator function directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        name_info = get_typeshed_nameinfo(fullname, modname)
        if name_info:
            update_signature_node(signode, name_info, fullname)

        return fullname, clsname


class TypedPyDecoratorMethod(PyDecoratorMethod):
    """Decorator method directive using typeshed type signatures."""

    def handle_signature(self, sig: str, signode: desc_signature) -> tuple[str, str]:
        fullname, clsname = super().handle_signature(sig, signode)
        modname = self.options.get("module", self.env.ref_context.get("py:module"))

        if clsname:
            method_qualname = f"{clsname}.{fullname}"
        else:
            method_qualname = fullname

        name_info = get_typeshed_nameinfo(method_qualname, modname)
        if name_info:
            update_signature_node(signode, name_info, fullname)

        return fullname, clsname


overriden_directives = {
    "function": TypedPyFunction,
    "class": TypedPyClasslike,
    "method": TypedPyMethod,
    "classmethod": TypedPyClassMethod,
    "staticmethod": TypedPyStaticMethod,
    "attribute": TypedPyAttribute,
    "property": TypedPyProperty,
    "decorator": TypedPyDecoratorFunction,
    "decoratormethod": TypedPyDecoratorMethod,
}


def setup(app: Sphinx) -> ExtensionMetadata:
    """Setup the Sphinx extension."""
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
