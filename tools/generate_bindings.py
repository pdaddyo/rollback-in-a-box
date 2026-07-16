#!/usr/bin/env python3
"""Generate the complete Godot bridge from Box3D's public C headers."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
INCLUDE = ROOT / "gdext" / "extern" / "box3d" / "include"
MAIN_HEADER = INCLUDE / "box3d" / "box3d.h"
PUBLIC_HEADERS = [
    MAIN_HEADER,
    INCLUDE / "box3d" / "collision.h",
    INCLUDE / "box3d" / "math_functions.h",
    INCLUDE / "box3d" / "types.h",
    INCLUDE / "box3d" / "id.h",
    INCLUDE / "box3d" / "base.h",
]
GENERATED = ROOT / "gdext" / "src" / "generated"

MANUAL_VALUE_TYPES = {"b3Vec3", "b3Quat", "b3Transform"}

# Declared in the public headers but with no definition in the pinned revision.
# Binding these would fail at link time.
MISSING_UPSTREAM_DEFINITIONS = {
    # Left behind when the dump-file code was removed upstream (erincatto/box3d#53).
    "b3World_DumpShapeBounds",
}


def walk(node: object):
    if not isinstance(node, dict):
        return
    yield node
    for child in node.get("inner", []):
        yield from walk(child)


def clean_space(value: str) -> str:
    return re.sub(r"\s+", " ", value).strip()


def ascii_text(value: str) -> str:
    return value.encode("ascii", "replace").decode("ascii")


def cpp_string(value: str) -> str:
    return json.dumps(ascii_text(value))


def comment_text(node: dict) -> str:
    chunks: list[str] = []
    for child in walk(node):
        if child.get("kind") in {"TextComment", "VerbatimLineComment"}:
            text = clean_space(child.get("text", ""))
            if text:
                chunks.append(text)
    return clean_space(" ".join(chunks))


def parse_exported_functions() -> dict[str, str]:
    result: dict[str, str] = {}
    for header in PUBLIC_HEADERS:
        text = header.read_text()
        for match in re.finditer(r"\bB3_API\b(.*?);", text, re.DOTALL):
            prototype = clean_space(match.group(1))
            name_match = re.search(r"\b(b3[A-Za-z0-9_]+)\s*\(", prototype)
            if name_match:
                result[name_match.group(1)] = header.name
    return result


def load_ast(*, release: bool = False) -> dict:
    command = [
        "clang",
        "-std=c17",
        *( ["-DNDEBUG"] if release else [] ),
        "-I",
        str(INCLUDE),
        "-Xclang",
        "-ast-dump=json",
        "-fsyntax-only",
        str(MAIN_HEADER),
    ]
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    return json.loads(result.stdout)


def split_function_type(qual_type: str) -> str:
    marker = qual_type.find("(")
    if marker < 0:
        raise ValueError(f"cannot parse function type: {qual_type}")
    return clean_space(qual_type[:marker])


def collect_model(ast: dict, release_function_names: set[str]) -> tuple[list[dict], list[dict], list[str]]:
    exported = parse_exported_functions()
    function_nodes: dict[str, dict] = {}
    records: dict[str, dict] = {}
    constants: set[str] = set()

    for node in walk(ast):
        kind = node.get("kind")
        name = node.get("name", "")
        if (
            kind == "FunctionDecl"
            and name.startswith("b3")
            and name in release_function_names
            and name not in MISSING_UPSTREAM_DEFINITIONS
        ):
            function_nodes.setdefault(name, node)
        elif kind == "RecordDecl" and node.get("completeDefinition") and name.startswith("b3"):
            records.setdefault(name, node)
        elif kind == "EnumConstantDecl" and name.startswith("b3"):
            constants.add(name)

    functions: list[dict] = []
    for name, node in sorted(function_nodes.items()):
        parameters = []
        for child in node.get("inner", []):
            if child.get("kind") == "ParmVarDecl":
                parameters.append(
                    {
                        "name": child.get("name", f"arg{len(parameters)}"),
                        "type": clean_space(child["type"]["qualType"]),
                    }
                )
        return_type = split_function_type(node["type"]["qualType"])
        full_comment = next((c for c in node.get("inner", []) if c.get("kind") == "FullComment"), None)
        functions.append(
            {
                "name": name,
                "return_type": return_type,
                "parameters": parameters,
                "signature": f"{return_type} {name}({', '.join(p['type'] for p in parameters)})",
                "kind": "exported" if name in exported else "inline",
                "header": exported.get(name, "public inline header"),
                "description": comment_text(full_comment) if full_comment else "",
            }
        )

    defaults: dict[str, str] = {}
    for function in functions:
        if function["name"].startswith("b3Default") and not function["parameters"]:
            defaults.setdefault(function["return_type"], function["name"])

    structs: list[dict] = []
    for name, node in sorted(records.items()):
        fields = []
        codec = name not in MANUAL_VALUE_TYPES
        for child in node.get("inner", []):
            if child.get("kind") != "FieldDecl":
                continue
            field_name = child.get("name")
            field_type = clean_space(child["type"]["qualType"])
            if not field_name or field_type.startswith("union "):
                codec = False
                continue
            fields.append({"name": field_name, "type": field_type})
        structs.append(
            {
                "name": name,
                "fields": fields,
                "dictionary_codec": codec,
                "default_function": defaults.get(name),
            }
        )

    return functions, structs, sorted(constants)


def is_pointer(field_type: str) -> bool:
    return "*" in field_type


def array_type(field_type: str) -> tuple[str, int] | None:
    match = re.fullmatch(r"(.+?)\[(\d+)\]", field_type)
    return (clean_space(match.group(1)), int(match.group(2))) if match else None


def generate_structs(structs: list[dict]) -> str:
    lines = ["// Generated by tools/generate_bindings.py. Do not edit.", ""]
    for struct in structs:
        name = struct["name"]
        if not struct["dictionary_codec"] or name in MANUAL_VALUE_TYPES:
            continue
        initial = f"{struct['default_function']}()" if struct["default_function"] else "{}"
        error_return = initial if initial != "{}" else f"{name}{{}}"
        lines.extend(
            [
                f"template <> struct StructCodec<{name}> {{",
                f"\tstatic {name} decode(const Variant &p_value) {{",
                "\t\tif (p_value.get_type() == Variant::PACKED_BYTE_ARRAY) {",
                f"\t\t\treturn unpack_value<{name}>((PackedByteArray)p_value);",
                "\t\t}",
                f"\t\tERR_FAIL_COND_V_MSG(p_value.get_type() != Variant::DICTIONARY, {error_return},",
                f"\t\t\t\t\"Box3DRaw: {name} must be a Dictionary or exact PackedByteArray.\");",
                "\t\tDictionary p_fields = p_value;",
                f"\t\t{name} value = {initial};",
            ]
        )
        for field in struct["fields"]:
            member = field["name"]
            field_type = field["type"]
            arr = array_type(field_type)
            lines.append(f"\t\tif (p_fields.has({cpp_string(member)})) {{")
            if arr:
                base, count = arr
                lines.extend(
                    [
                        f"\t\t\tArray source = p_fields[{cpp_string(member)}];",
                        f"\t\t\tERR_FAIL_COND_V_MSG(source.size() != {count}, value, \"Box3DRaw: {name}.{member} requires {count} values.\");",
                        f"\t\t\tfor (int i = 0; i < {count}; ++i) value.{member}[i] = from_variant<{base}>(source[i]);",
                    ]
                )
            elif is_pointer(field_type):
                lines.append(
                    f"\t\t\tvalue.{member} = reinterpret_cast<decltype(value.{member})>((uintptr_t)(int64_t)p_fields[{cpp_string(member)}]);"
                )
            else:
                lines.append(f"\t\t\tvalue.{member} = from_variant<{field_type}>(p_fields[{cpp_string(member)}]);")
            lines.append("\t\t}")
        lines.extend(["\t\treturn value;", "\t}", f"\tstatic Variant encode(const {name} &value) {{", "\t\tDictionary fields;"])
        for field in struct["fields"]:
            member = field["name"]
            field_type = field["type"]
            arr = array_type(field_type)
            if arr:
                _, count = arr
                lines.extend(
                    [
                        "\t\t{",
                        "\t\t\tArray target;",
                        f"\t\t\ttarget.resize({count});",
                        f"\t\t\tfor (int i = 0; i < {count}; ++i) target[i] = to_variant(value.{member}[i]);",
                        f"\t\t\tfields[{cpp_string(member)}] = target;",
                        "\t\t}",
                    ]
                )
            elif is_pointer(field_type):
                lines.append(f"\t\tfields[{cpp_string(member)}] = (int64_t)(uintptr_t)value.{member};")
            else:
                lines.append(f"\t\tfields[{cpp_string(member)}] = to_variant(value.{member});")
        lines.extend(["\t\treturn fields;", "\t}", "};", ""])
    return "\n".join(lines)


def generate_functions(functions: list[dict]) -> str:
    lines = ["// Generated by tools/generate_bindings.py. Do not edit."]
    for function in functions:
        name = function["name"]
        params = function["parameters"]
        lines.extend(
            [
                f"if (function == StringName({cpp_string(name)})) {{",
                f"\tERR_FAIL_COND_V_MSG(arguments.size() != {len(params)}, Variant(), \"{name} expects {len(params)} arguments.\");",
            ]
        )
        args = []
        for index, param in enumerate(params):
            ptype = param["type"]
            if ptype == "const char *":
                lines.append(f"\tCharString string_{index} = String(arguments[{index}]).utf8();")
                lines.append(f"\tconst char *arg_{index} = string_{index}.get_data();")
            elif is_pointer(ptype):
                lines.append(f"\tauto arg_{index} = pointer_from_variant<{ptype}>(arguments[{index}]);")
            else:
                lines.append(f"\tauto arg_{index} = from_variant<{ptype}>(arguments[{index}]);")
            args.append(f"arg_{index}")
        call = f"{name}({', '.join(args)})"
        rtype = function["return_type"]
        if rtype == "void":
            lines.extend([f"\t{call};", "\treturn Variant();"])
        elif rtype == "const char *":
            lines.extend([f"\tconst char *result = {call};", "\treturn result == nullptr ? Variant(String()) : Variant(String::utf8(result));"])
        elif is_pointer(rtype):
            lines.extend([f"\tauto result = {call};", "\treturn (int64_t)(uintptr_t)result;"])
        else:
            lines.extend([f"\tauto result = {call};", "\treturn to_variant(result);"])
        lines.append("}")
    return "\n".join(lines) + "\n"


def generate_catalog(functions: list[dict], structs: list[dict], constants: list[str]) -> str:
    lines = [
        "// Generated by tools/generate_bindings.py. Do not edit.",
        "struct RawFunctionInfo { const char *name; const char *signature; const char *kind; const char *header; };",
        "static const RawFunctionInfo raw_functions[] = {",
    ]
    for function in functions:
        lines.append(
            f"\t{{ {cpp_string(function['name'])}, {cpp_string(function['signature'])}, {cpp_string(function['kind'])}, {cpp_string(function['header'])} }},"
        )
    lines.extend(
        [
            "};",
            "static int generated_function_count() { return (int)(sizeof(raw_functions) / sizeof(raw_functions[0])); }",
            "static PackedStringArray generated_function_names() {",
            "\tPackedStringArray names;",
            "\tnames.resize(generated_function_count());",
            "\tfor (int i = 0; i < generated_function_count(); ++i) names.set(i, raw_functions[i].name);",
            "\treturn names;",
            "}",
            "static Dictionary generated_function_info(const StringName &name) {",
            "\tfor (const RawFunctionInfo &info : raw_functions) {",
            "\t\tif (name == StringName(info.name)) {",
            "\t\t\tDictionary out; out[\"name\"] = info.name; out[\"signature\"] = info.signature;",
            "\t\t\tout[\"kind\"] = info.kind; out[\"header\"] = info.header; return out;",
            "\t\t}",
            "\t}",
            "\treturn Dictionary();",
            "}",
            "static Dictionary generated_constants() {",
            "\tDictionary constants;",
        ]
    )
    for constant in constants:
        lines.append(f"\tconstants[{cpp_string(constant)}] = (int64_t){constant};")
    lines.extend(["\treturn constants;", "}", "static PackedStringArray generated_struct_names() {", "\tPackedStringArray names;"])
    for struct in structs:
        lines.append(f"\tnames.push_back({cpp_string(struct['name'])});")
    lines.extend(["\treturn names;", "}", "static Dictionary generated_struct_info(const StringName &name) {"])
    for struct in structs:
        lines.extend(
            [
                f"\tif (name == StringName({cpp_string(struct['name'])})) {{",
                f"\t\tDictionary out; out[\"name\"] = {cpp_string(struct['name'])}; out[\"size\"] = (int64_t)sizeof({struct['name']});",
                f"\t\tout[\"dictionary_codec\"] = {'true' if struct['dictionary_codec'] else 'false'};",
                "\t\tArray fields;",
            ]
        )
        for field in struct["fields"]:
            lines.append(
                f"\t\t{{ Dictionary field; field[\"name\"] = {cpp_string(field['name'])}; field[\"type\"] = {cpp_string(field['type'])}; fields.push_back(field); }}"
            )
        lines.extend(["\t\tout[\"fields\"] = fields; return out;", "\t}"])
    lines.extend(["\treturn Dictionary();", "}", "static PackedByteArray generated_encode_struct(const StringName &name, const Variant &value) {"])
    for struct in structs:
        lines.append(f"\tif (name == StringName({cpp_string(struct['name'])})) return pack_value(from_variant<{struct['name']}>(value));")
    lines.extend(["\tERR_FAIL_V_MSG(PackedByteArray(), \"Box3DRaw: unknown struct type: \" + String(name));", "}", "static Variant generated_decode_struct(const StringName &name, const PackedByteArray &bytes) {"])
    for struct in structs:
        lines.append(f"\tif (name == StringName({cpp_string(struct['name'])})) return to_variant(unpack_value<{struct['name']}>(bytes));")
    lines.extend(["\tERR_FAIL_V_MSG(Variant(), \"Box3DRaw: unknown struct type: \" + String(name));", "}"])
    return "\n".join(lines) + "\n"


def generate_gdscript(functions: list[dict]) -> str:
    lines = [
        "# Generated by tools/generate_bindings.py. Do not edit.",
        "class_name Box3D",
        "extends RefCounted",
        "",
        "var raw: Box3DRaw",
        "",
        "func _init(p_raw: Box3DRaw = null) -> void:",
        "\traw = p_raw if p_raw != null else Box3DRaw.new()",
        "",
    ]
    for function in functions:
        args = [f"arg{i}" for i in range(len(function["parameters"]))]
        array = f"[{', '.join(args)}]"
        lines.extend(
            [
                f"func {function['name']}({', '.join(args)}):",
                f"\treturn raw.call_box3d(&\"{function['name']}\", {array})",
                "",
            ]
        )
    return "\n".join(lines)


def generate_docs(functions: list[dict], structs: list[dict]) -> str:
    exported = sum(1 for function in functions if function["kind"] == "exported")
    inline = len(functions) - exported
    lines = [
        "# Box3D API reference",
        "",
        "This file is generated from the pinned Box3D public headers. Do not edit it by hand.",
        "",
        f"Coverage: **{len(functions)} public functions** ({exported} exported `B3_API`, {inline} public inline) and **{len(structs)} concrete public structs**.",
        "",
        "Use the generated `Box3D` class for exact C function names or call `Box3DRaw.call_box3d()` directly. IDs are Godot integers. Value structs accept Dictionaries or exact packed bytes. Pointer parameters accept `Box3DBuffer`, a native address returned by Box3D, or `null`.",
        "",
        "## Functions",
        "",
        "| Function | Signature | Kind |",
        "| --- | --- | --- |",
    ]
    for function in functions:
        signature = function["signature"].replace("|", "\\|")
        lines.append(f"| `{function['name']}` | `{signature}` | {function['kind']} |")
    lines.extend(["", "## Structs", "", "| Struct | Dictionary fields |", "| --- | --- |"])
    for struct in structs:
        fields = ", ".join(f"`{f['name']}: {f['type']}`" for f in struct["fields"])
        if not struct["dictionary_codec"]:
            fields = "Exact packed bytes only (contains anonymous union fields)."
        lines.append(f"| `{struct['name']}` | {fields} |")
    return ascii_text("\n".join(lines) + "\n")


def generated_outputs(functions: list[dict], structs: list[dict], constants: list[str]) -> dict[Path, str]:
    model = {
        "box3d_commit": subprocess.run(
            ["git", "-C", str(ROOT / "gdext" / "extern" / "box3d"), "rev-parse", "HEAD"],
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip(),
        "function_count": len(functions),
        "exported_function_count": sum(1 for f in functions if f["kind"] == "exported"),
        "inline_function_count": sum(1 for f in functions if f["kind"] == "inline"),
        "struct_count": len(structs),
        "functions": functions,
        "structs": structs,
        "constants": constants,
    }
    return {
        GENERATED / "box3d_structs.inc": generate_structs(structs),
        GENERATED / "box3d_functions.inc": generate_functions(functions),
        GENERATED / "box3d_catalog.inc": generate_catalog(functions, structs, constants),
        ROOT / "addons" / "box3d_rollback" / "box3d.gd": generate_gdscript(functions),
        ROOT / "docs" / "api-reference.md": generate_docs(functions, structs),
        ROOT / "api" / "box3d-api.json": json.dumps(model, indent=2, sort_keys=True, ensure_ascii=True) + "\n",
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="fail if committed generated files are stale")
    args = parser.parse_args()

    ast = load_ast()
    release_ast = load_ast(release=True)
    release_function_names = {
        node.get("name", "")
        for node in walk(release_ast)
        if node.get("kind") == "FunctionDecl" and node.get("name", "").startswith("b3")
    }
    functions, structs, constants = collect_model(ast, release_function_names)
    outputs = generated_outputs(functions, structs, constants)
    stale: list[Path] = []
    for path, content in outputs.items():
        if path.exists() and path.read_text() == content:
            continue
        stale.append(path)
        if not args.check:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)
    if stale and args.check:
        for path in stale:
            print(f"stale generated file: {path.relative_to(ROOT)}", file=sys.stderr)
        return 1
    print(f"Box3D API: {len(functions)} functions, {len(structs)} structs, {len(constants)} enum constants")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
