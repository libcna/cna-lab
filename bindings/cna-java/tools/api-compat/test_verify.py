#!/usr/bin/env python3
"""Focused regression tests for contract mapping and compiled metadata reading."""

from __future__ import annotations

import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location("cna_java_api_verify", Path(__file__).with_name("verify.py"))
if SPEC is None or SPEC.loader is None:
    raise RuntimeError("cannot load verifier module")
VERIFY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERIFY)


class VerifyTests(unittest.TestCase):

    @classmethod
    def setUpClass(cls) -> None:
        cls.rules = json.loads(
            (ROOT / "tools/api-compat/mapping-rules.json").read_text(encoding="utf-8"))
        VERIFY.GENERIC_RENAMES.clear()
        VERIFY.GENERIC_RENAMES.update(cls.rules.get("genericTypeRenames", {}))
        VERIFY.TYPE_RENAMES.clear()
        VERIFY.TYPE_RENAMES.update(cls.rules.get("frameworkTypeMappings", {}))

    def test_interface_property_and_event_accessors_are_abstract(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.IProbe",
            "kind": "interface",
            "members": [
                {"kind": "property", "name": "Enabled", "type": "System.Boolean",
                 "static": False, "getterAccess": "public", "setterAccess": "none",
                 "parameters": []},
                {"kind": "event", "name": "Changed", "type": "System.EventHandler`1[System.EventArgs]",
                 "static": False, "addAccess": "public", "removeAccess": "public"},
            ],
        }

        members = VERIFY.mapped_members(contract, self.rules, [])
        self.assertEqual(3, len(members))
        self.assertTrue(all(member["abstract"] for member in members))

    def test_framework_compatibility_types_are_counted_in_expected_contract(self) -> None:
        reference = {"types": [{
            "name": "Microsoft.Xna.Framework.Probe",
            "kind": "class",
            "members": [],
        }]}
        expected_types, expected_members = VERIFY.expected_java_counts(reference, self.rules)
        self.assertEqual(1 + len(self.rules["syntheticTypes"]), expected_types)
        self.assertEqual(sum(len(value["members"]) for value in self.rules["syntheticTypes"]),
                         expected_members)

    def test_unsigned_clr_byte_maps_to_range_checked_java_int(self) -> None:
        self.assertEqual("int", VERIFY.map_type("System.Byte"))
        self.assertEqual("byte", VERIFY.map_type("System.SByte"))

    def test_stream_direction_and_missing_parameter_names_are_explicit(self) -> None:
        member = {
            "name": "FromStream",
            "parameters": [
                {"name": "graphicsDevice", "type": "Microsoft.Xna.Framework.Graphics.GraphicsDevice"},
                {"name": "stream", "type": "System.IO.Stream"},
            ],
        }
        parameters = VERIFY.map_member_parameters(
            "Microsoft.Xna.Framework.Graphics.Texture2D", member, self.rules)
        self.assertEqual("java.io.InputStream", parameters[1]["type"])
        open_stream = {
            "name": "OpenStream",
            "returnType": "System.IO.Stream",
            "parameters": [{"name": "assetName", "type": "System.String"}],
        }
        self.assertEqual(
            "java.io.InputStream",
            VERIFY.map_member_return_type(
                "Microsoft.Xna.Framework.Content.ContentManager", open_stream, self.rules))
        unnamed = VERIFY.map_parameters([{"name": "", "type": "System.Boolean"}])
        self.assertEqual("arg0", unnamed[0]["name"])

    def test_design_converter_projection_uses_java_concepts_and_omits_clr_context(self) -> None:
        projection = self.rules["designTypeConverterProjection"]
        self.assertEqual("java.util.Locale", projection["cultureInfo"])
        self.assertEqual("java.beans.Expression", projection["instanceDescriptor"])
        self.assertEqual("omitted", projection["typeDescriptorContext"])
        member = {
            "name": "ConvertTo",
            "returnType": "System.Object",
            "parameters": [
                {"name": "context", "type": "System.ComponentModel.ITypeDescriptorContext"},
                {"name": "culture", "type": "System.Globalization.CultureInfo"},
                {"name": "value", "type": "System.Object"},
                {"name": "destinationType", "type": "System.Type"},
            ],
        }
        parameters = VERIFY.map_member_parameters(
            "Microsoft.Xna.Framework.Design.Vector3Converter", member, self.rules)
        self.assertEqual(
            ["java.util.Locale", "java.lang.Object", "java.lang.Class<?>"],
            [value["type"] for value in parameters])
        self.assertEqual(
            "java.util.LinkedHashMap<java.lang.String,java.lang.Class<?>>",
            VERIFY.map_type("System.ComponentModel.PropertyDescriptorCollection"))

    def test_conversion_operator_requires_and_uses_an_explicit_mapping(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.Graphics.RenderTargetBinding",
            "kind": "struct",
            "interfaces": [],
            "members": [{
                "kind": "method", "name": "op_Implicit", "access": "public",
                "static": True, "abstract": False, "final": False, "virtual": False,
                "genericArity": 0,
                "returnType": "Microsoft.Xna.Framework.Graphics.RenderTargetBinding",
                "parameters": [{
                    "name": "renderTarget",
                    "type": "Microsoft.Xna.Framework.Graphics.RenderTarget2D",
                }],
            }],
        }
        findings = []
        members = VERIFY.mapped_members(contract, self.rules, findings)
        conversion = next(value for value in members
                          if value["name"] == "fromRenderTarget2D")
        self.assertEqual(
            "Microsoft.Xna.Framework.Graphics.RenderTargetBinding",
            conversion["returnType"])
        self.assertEqual(
            "Microsoft.Xna.Framework.Graphics.RenderTarget2D",
            conversion["parameters"][0]["type"])
        self.assertTrue(conversion["static"])
        self.assertEqual([], findings)

        unmapped_findings = []
        VERIFY.mapped_members(contract, {"conversionMemberMappings": {}}, unmapped_findings)
        self.assertEqual("XNA_MAPPING_MISMATCH", unmapped_findings[0]["code"])

    def test_named_color_properties_map_uniformly_to_same_cased_fields(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.Color", "kind": "struct",
            "interfaces": [],
            "members": [
                {"kind": "property", "name": "AliceBlue",
                 "type": "Microsoft.Xna.Framework.Color", "static": True,
                 "getterAccess": "public", "setterAccess": "none", "parameters": []},
                {"kind": "property", "name": "R", "type": "System.Byte", "static": False,
                 "getterAccess": "public", "setterAccess": "public", "parameters": []},
            ],
        }
        members = VERIFY.mapped_members(contract, self.rules, [])
        alice_blue = next(value for value in members if value["name"] == "AliceBlue")
        self.assertEqual("field", alice_blue["kind"])
        self.assertTrue(alice_blue["static"])
        self.assertTrue(alice_blue["final"])
        self.assertTrue(any(value["name"] == "getR" for value in members))
        self.assertTrue(any(value["name"] == "setR" for value in members))

    def test_explicit_interface_methods_are_promoted_and_packed_properties_are_boxed(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.Color", "kind": "struct",
            "interfaces": [
                "Microsoft.Xna.Framework.Graphics.PackedVector.IPackedVector`1[System.UInt32]"
            ],
            "explicitInterfaceMethods": [{
                "kind": "method", "name": "PackFromVector4", "access": "public",
                "static": False, "abstract": False, "final": True, "virtual": True,
                "genericArity": 0, "returnType": "System.Void",
                "parameters": [{
                    "name": "vector", "type": "Microsoft.Xna.Framework.Vector4"
                }],
            }],
            "members": [{
                "kind": "property", "name": "PackedValue", "type": "System.UInt32",
                "static": False, "getterAccess": "public", "setterAccess": "public",
                "getterFinal": True, "setterFinal": True, "parameters": [],
            }],
        }

        members = VERIFY.mapped_members(contract, self.rules, [])
        getter = next(value for value in members if value["name"] == "getPackedValue")
        setter = next(value for value in members if value["name"] == "setPackedValue")
        promoted = next(value for value in members if value["name"] == "PackFromVector4")
        self.assertEqual("java.lang.Long", getter["returnType"])
        self.assertEqual("java.lang.Long", setter["parameters"][0]["type"])
        self.assertEqual("public", promoted["access"])
        self.assertTrue(promoted["final"])

    def test_explicit_interface_property_accessor_uses_java_property_name(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.Graphics.VertexPositionColor",
            "kind": "struct",
            "interfaces": ["Microsoft.Xna.Framework.Graphics.IVertexType"],
            "explicitInterfaceMethods": [{
                "kind": "method", "name": "getVertexDeclaration", "access": "public",
                "static": False, "abstract": False, "final": True, "virtual": True,
                "genericArity": 0,
                "returnType": "Microsoft.Xna.Framework.Graphics.VertexDeclaration",
                "parameters": [],
            }],
            "members": [],
        }

        members = VERIFY.mapped_members(contract, self.rules, [])
        getter = next(value for value in members if value["name"] == "getVertexDeclaration")
        self.assertEqual("Microsoft.Xna.Framework.Graphics.VertexDeclaration", getter["returnType"])
        self.assertEqual("public", getter["access"])
        self.assertTrue(getter["final"])

    def test_explicit_disposable_implementation_still_projects_close(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.DisposableProbe",
            "kind": "class", "abstract": False,
            "interfaces": ["System.IDisposable"], "members": [],
        }
        members = VERIFY.mapped_members(contract, self.rules, [])
        close = next(value for value in members if value["name"] == "close")
        self.assertEqual("public", close["access"])
        self.assertTrue(close["final"])

    def test_public_dispose_keeps_its_xna_name_and_gains_a_close_bridge(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.DisposableProbe",
            "kind": "class", "abstract": False,
            "interfaces": ["System.IDisposable"],
            "members": [{
                "kind": "method", "name": "Dispose", "access": "public", "static": False,
                "abstract": False, "final": True, "virtual": True, "genericArity": 0,
                "returnType": "System.Void", "parameters": [],
            }],
        }
        members = VERIFY.mapped_members(contract, self.rules, [])
        dispose = next(value for value in members if value["name"] == "Dispose")
        self.assertEqual("public", dispose["access"])
        self.assertTrue(dispose["final"])
        self.assertEqual("void", dispose["returnType"])
        close = next(value for value in members if value["name"] == "close")
        self.assertEqual("public", close["access"])
        self.assertTrue(close["final"])
        self.assertFalse(close["abstract"])

    def test_enumerator_disposable_reaches_the_close_bridge_through_ienumerator(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.EnumeratorProbe",
            "kind": "struct", "abstract": False,
            "interfaces": ["System.Collections.Generic.IEnumerator`1[System.Int32]"],
            "members": [{
                "kind": "method", "name": "Dispose", "access": "public", "static": False,
                "abstract": False, "final": True, "virtual": True, "genericArity": 0,
                "returnType": "System.Void", "parameters": [],
            }],
        }
        members = VERIFY.mapped_members(contract, self.rules, [])
        self.assertTrue(any(value["name"] == "Dispose" for value in members))
        self.assertTrue(any(value["name"] == "close" for value in members))

    def test_disposable_interface_leaves_the_close_bridge_abstract(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.IDisposableProbe",
            "kind": "interface", "abstract": True,
            "interfaces": ["System.IDisposable"], "members": [],
        }
        members = VERIFY.mapped_members(contract, self.rules, [])
        close = next(value for value in members if value["name"] == "close")
        self.assertTrue(close["abstract"])
        self.assertFalse(close["final"])

    def test_object_contract_members_are_still_lowered(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.ObjectProbe",
            "kind": "class", "abstract": False, "interfaces": [],
            "members": [
                {"kind": "method", "name": name, "access": "public", "static": False,
                 "abstract": False, "final": False, "virtual": True, "genericArity": 0,
                 "returnType": returns, "parameters": parameters}
                for name, returns, parameters in (
                    ("ToString", "System.String", []),
                    ("GetHashCode", "System.Int32", []),
                    ("Equals", "System.Boolean",
                     [{"name": "obj", "type": "System.Object", "out": False}]),
                )
            ],
        }
        names = {value["name"] for value in VERIFY.mapped_members(contract, self.rules, [])}
        # Java's Object already declares these three; an XNA-cased twin would not
        # override them, so collections and printing would silently use the wrong one.
        self.assertEqual({"toString", "hashCode", "equals"}, names)

    def test_clr_collection_projects_iterator_and_required_java_collection_bridges(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.CurveKeyCollection",
            "kind": "class",
            "interfaces": [
                "System.Collections.Generic.ICollection`1[Microsoft.Xna.Framework.CurveKey]"
            ],
            "members": [{
                "kind": "method", "name": "GetEnumerator", "access": "public",
                "static": False, "abstract": False, "final": True, "virtual": True,
                "genericArity": 0,
                "returnType": "System.Collections.Generic.IEnumerator`1[Microsoft.Xna.Framework.CurveKey]",
                "parameters": [],
            }],
        }

        members = VERIFY.mapped_members(contract, self.rules, [])
        enumerator = next(value for value in members if value["name"] == "GetEnumerator")
        self.assertEqual(
            "java.util.Iterator<Microsoft.Xna.Framework.CurveKey>", enumerator["returnType"])
        bridges = {value["name"] for value in members if value["name"] != "GetEnumerator"}
        self.assertEqual(
            {"size", "isEmpty", "contains", "iterator", "toArray", "add", "remove",
             "containsAll", "addAll", "removeAll", "retainAll", "clear"},
            bridges)

    def test_touch_ref_out_results_and_java_list_bridges_are_explicit(self) -> None:
        location = {
            "name": "Microsoft.Xna.Framework.Input.Touch.TouchLocation",
            "kind": "struct", "interfaces": [],
            "members": [{
                "kind": "method", "name": "TryGetPreviousLocation", "access": "public",
                "static": False, "abstract": False, "final": False, "virtual": False,
                "genericArity": 0, "returnType": "System.Boolean",
                "parameters": [{"name": "previousLocation",
                                "type": "Microsoft.Xna.Framework.Input.Touch.TouchLocation&",
                                "out": True}],
            }],
        }
        findings = []
        members = VERIFY.mapped_members(location, self.rules, findings)
        result = next(value for value in members if value["name"] == "TryGetPreviousLocation")
        self.assertEqual([], findings)
        self.assertEqual([], result["parameters"])
        self.assertEqual(
            "Microsoft.Xna.Framework.Input.Touch.TouchLocation.PreviousLocationResult",
            result["returnType"])

        collection = {
            "name": "Microsoft.Xna.Framework.Input.Touch.TouchCollection",
            "kind": "struct",
            "interfaces": [
                "System.Collections.Generic.IList`1[Microsoft.Xna.Framework.Input.Touch.TouchLocation]"
            ],
            "members": [{
                "kind": "property", "name": "Item",
                "type": "Microsoft.Xna.Framework.Input.Touch.TouchLocation", "static": False,
                "getterAccess": "public", "setterAccess": "public",
                "getterAbstract": False, "setterAbstract": False,
                "getterFinal": False, "setterFinal": False,
                "getterVirtual": True, "setterVirtual": True,
                "parameters": [{"name": "index", "type": "System.Int32"}],
            }],
        }
        members = VERIFY.mapped_members(collection, self.rules, [])
        setter = next(value for value in members
                      if value["name"] == "set" and len(value["parameters"]) == 2)
        self.assertEqual("Microsoft.Xna.Framework.Input.Touch.TouchLocation",
                         setter["returnType"])
        self.assertTrue(any(value["name"] == "listIterator" for value in members))

    def test_touch_enumerator_has_reviewed_java_iterator_bridge(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.Input.Touch.TouchCollection+Enumerator",
            "kind": "struct",
            "interfaces": [
                "System.Collections.Generic.IEnumerator`1[Microsoft.Xna.Framework.Input.Touch.TouchLocation]"
            ],
            "members": [],
        }
        members = VERIFY.mapped_members(contract, self.rules, [])
        self.assertEqual({"hasNext", "next"},
                         {value["name"] for value in members
                          if value["name"] in ("hasNext", "next")})

    def test_display_mode_collection_has_reviewed_java_iterable_bridge(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.Graphics.DisplayModeCollection",
            "kind": "class",
            "interfaces": [
                "System.Collections.Generic.IEnumerable`1[Microsoft.Xna.Framework.Graphics.DisplayMode]"
            ],
            "members": [],
        }
        members = VERIFY.mapped_members(contract, self.rules, [])
        iterator = next(value for value in members if value["name"] == "iterator")
        self.assertEqual(
            "java.util.Iterator<Microsoft.Xna.Framework.Graphics.DisplayMode>",
            iterator["returnType"])

    def test_explicit_excluded_clr_member_is_a_mapping_rule_not_allowlist(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.Content.ContentLoadException",
            "kind": "class", "interfaces": [],
            "members": [{
                "kind": "constructor", "name": ".ctor", "access": "protected",
                "static": False, "abstract": False, "final": False,
                "genericArity": 0, "returnType": None,
                "parameters": [
                    {"name": "info", "type": "System.Runtime.Serialization.SerializationInfo"},
                    {"name": "context", "type": "System.Runtime.Serialization.StreamingContext"},
                ],
            }],
        }
        self.assertEqual([], VERIFY.mapped_members(contract, self.rules, []))

    def test_flags_enum_maps_to_composable_value_contract(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.ProbeFlags",
            "kind": "enum",
            "flags": True,
            "members": [
                {"kind": "field", "name": "None", "type": "Microsoft.Xna.Framework.ProbeFlags",
                 "static": True, "final": True, "constant": "0", "access": "public"},
                {"kind": "field", "name": "First", "type": "Microsoft.Xna.Framework.ProbeFlags",
                 "static": True, "final": True, "constant": "1", "access": "public"},
                {"kind": "field", "name": "value__", "type": "System.Int32",
                 "static": False, "final": False, "constant": None, "access": "public"},
            ],
        }

        members = VERIFY.mapped_enum_members(contract)
        self.assertNotIn("value__", [member["name"] for member in members])
        self.assertEqual(
            {"getValue", "FromValue", "Or", "Contains", "equals", "hashCode"},
            {member["name"] for member in members if member["kind"] == "method"})

    def test_sequential_enum_detection_is_independent_of_metadata_sort_order(self) -> None:
        contract = {
            "name": "Microsoft.Xna.Framework.ProbeEnum",
            "kind": "enum",
            "flags": False,
            "members": [
                {"kind": "field", "name": "Second", "type": "Microsoft.Xna.Framework.ProbeEnum",
                 "static": True, "final": True, "constant": "1", "access": "public"},
                {"kind": "field", "name": "First", "type": "Microsoft.Xna.Framework.ProbeEnum",
                 "static": True, "final": True, "constant": "0", "access": "public"},
                {"kind": "field", "name": "value__", "type": "System.Int32",
                 "static": False, "final": False, "constant": None, "access": "public"},
            ],
        }
        self.assertFalse(any(member["kind"] == "method"
                             for member in VERIFY.mapped_enum_members(contract)))

    def test_instance_method_in_final_class_is_effectively_final(self) -> None:
        method = {"kind": "method", "static": False, "final": False}
        self.assertTrue(VERIFY.effective_member_final(method, True))
        self.assertFalse(VERIFY.effective_member_final(method, False))
        self.assertFalse(VERIFY.effective_member_final({**method, "static": True}, True))
        self.assertFalse(VERIFY.effective_member_final(
            {"kind": "field", "static": False, "final": False}, True))

    def test_parameter_pairing_never_steals_an_exact_overload(self) -> None:
        reference = {"types": [{
            "name": "Probe.Value", "kind": "class", "access": "public",
            "abstract": False, "sealed": False, "baseType": "System.Object",
            "interfaces": [], "genericArity": 0,
            "members": [
                {"kind": "method", "name": "Equals", "access": "public",
                 "static": False, "abstract": False, "final": False, "virtual": True,
                 "genericArity": 0, "returnType": "System.Boolean",
                 "parameters": [{"name": "obj", "type": "System.Object"}]},
                {"kind": "method", "name": "Equals", "access": "public",
                 "static": False, "abstract": False, "final": False, "virtual": False,
                 "genericArity": 0, "returnType": "System.Boolean",
                 "parameters": [{"name": "other", "type": "Probe.Value"}]},
            ],
        }]}
        target = {"types": [{
            "name": "Probe.Value", "kind": "class", "access": "public",
            "abstract": False, "sealed": False, "baseType": "java.lang.Object",
            "interfaces": [], "genericArity": 0,
            "members": [{"kind": "method", "name": "equals", "access": "public",
                         "static": False, "abstract": False, "final": False,
                         "genericArity": 0, "returnType": "boolean",
                         "parameters": [{"name": "obj", "type": "java.lang.Object"}]}],
        }]}

        findings = [value for value in VERIFY.compare(reference, target, self.rules)
                    if value["subject"].startswith("Probe.Value")]
        self.assertEqual(1, len(findings))
        self.assertEqual("MISSING_MEMBER", findings[0]["code"])
        self.assertIn("Probe.Value", findings[0]["subject"])

    def test_inaccessible_clr_interface_is_not_projected(self) -> None:
        reference = {"types": [{
            "name": "Probe.Value", "kind": "class", "access": "public",
            "abstract": False, "sealed": False, "baseType": "System.Object",
            "interfaces": ["Probe.InternalMarker"], "genericArity": 0,
            "members": [],
        }]}
        target = {"types": [{
            "name": "Probe.Value", "kind": "class", "access": "public",
            "abstract": False, "sealed": False, "baseType": "java.lang.Object",
            "interfaces": [], "genericArity": 0, "members": [],
        }]}
        findings = VERIFY.compare(reference, target, self.rules)
        self.assertFalse(any(value["code"] == "INTERFACE_MISMATCH" for value in findings))

    def test_leak_guard_finds_internal_types_and_raw_long_handles(self) -> None:
        target = {"types": [{
            "name": "Microsoft.Xna.Framework.BadFacade",
            "baseType": "java.lang.Object",
            "interfaces": [],
            "members": [
                {"name": "getAdapter", "returnType": "org.openeggbert.cna.internal.NativeBindings",
                 "parameters": []},
                {"name": "getNativeHandle", "returnType": "long", "parameters": []},
            ],
        }]}
        self.assertEqual(2, len(VERIFY.leak_diagnostics(target)))

    def test_class_reader_normalizes_multi_argument_generic_type_names(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cna-java-api-test-") as directory:
            temporary = Path(directory)
            source = temporary / "source/Microsoft/Xna/Framework/ProbeMap.java"
            source.parent.mkdir(parents=True)
            source.write_text(
                "package Microsoft.Xna.Framework;\n"
                "public class ProbeMap extends java.util.LinkedHashMap<String, String> {\n"
                "    private static final long serialVersionUID = 1L;\n"
                "}\n",
                encoding="utf-8")
            classes = temporary / "classes"
            classes.mkdir()
            subprocess.run(["javac", "--release", "17", "-d", str(classes), str(source)], check=True)
            reader = temporary / "reader"
            reader.mkdir()
            target = VERIFY.read_target(str(classes), reader)
            self.assertEqual(
                "java.util.LinkedHashMap<java.lang.String,java.lang.String>",
                target["types"][0]["baseType"])

    def test_reference_profile_requires_exact_assembly_hashes(self) -> None:
        with tempfile.TemporaryDirectory(prefix="cna-java-reference-test-") as directory:
            root = Path(directory)
            assembly = root / "Reference.dll"
            assembly.write_bytes(b"reference metadata")
            profile = {
                "referenceAssemblies": [assembly.name],
                "referenceSha256": {
                    assembly.name: "5715de24b778e528a96639e0a88e4ab2ed3426b45226a0db96fc36a36ab590cf",
                },
            }
            VERIFY.validate_reference_files(str(root), profile)
            profile["referenceSha256"][assembly.name] = "0" * 64
            with self.assertRaisesRegex(ValueError, "SHA-256 mismatch"):
                VERIFY.validate_reference_files(str(root), profile)


if __name__ == "__main__":
    unittest.main()
