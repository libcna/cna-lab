#!/usr/bin/env python3
"""Negative-mutation tests for the CNA-Java native-boundary tools.

A verifier that only passes on the current tree proves nothing.  Every check in
``verify.py``, ``inventory.py`` and ``coverage.py`` is exercised here by
mutating a copy of the input until that specific check fails, so a check that
silently stopped working is itself a test failure.
"""

from __future__ import annotations

import argparse
import copy
import json
import os
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import coverage as coverage_tool  # noqa: E402
import generate_jni as generator_tool  # noqa: E402
import inventory as inventory_tool  # noqa: E402
import verify as verify_tool  # noqa: E402


ROOT = Path(__file__).resolve().parents[2]
FAILURES: list[str] = []
PASSES: list[str] = []


def check(condition: bool, description: str) -> None:
    if condition:
        PASSES.append(description)
        print(f"PASS {description}")
    else:
        print(f"FAIL {description}")
        FAILURES.append(description)


def prefixed(failures: list[str], code: str) -> bool:
    return any(failure.startswith(code) for failure in failures)


def test_inventory(include: Path) -> dict:
    live = inventory_tool.inventory(include, strict=True)
    check(live["functions"], "inventory extracts CNA C API functions")
    check(not live["problems"], "inventory reports no unparsable declaration")
    check(all(value["name"].startswith("cna_") for value in live["functions"].values()),
          "every extracted function carries a cna_ symbol name")
    check(inventory_tool.identity_prefix("CNA_SpriteSortMode") == "CNA_SPRITE_SORT_MODE_",
          "identity value prefixes derive from the identity type name")
    sort_mode = live["identities"].get("CNA_SpriteSortMode")
    check(sort_mode is not None and sort_mode["values"],
          "an identity collects the constants that name its values")
    return live


def test_manifest(live: dict) -> None:
    manifest = json.loads(verify_tool.MANIFEST.read_text(encoding="utf-8"))
    check(not verify_tool.check_manifest(manifest, live),
          "the committed manifest matches the live CNA headers")

    mutated = copy.deepcopy(manifest)
    mutated["functions"][0]["parameters"] = ["int"]
    check(prefixed(verify_tool.check_manifest(mutated, live), "PARAMETER_DRIFT"),
          "a manifest parameter-type mutation is rejected")

    mutated = copy.deepcopy(manifest)
    mutated["functions"][0]["returnType"] = "void"
    check(prefixed(verify_tool.check_manifest(mutated, live), "RETURN_TYPE_DRIFT"),
          "a manifest return-type mutation is rejected")

    mutated = copy.deepcopy(manifest)
    mutated["functions"][0]["header"] = "not_a_header.h"
    check(prefixed(verify_tool.check_manifest(mutated, live), "HEADER_DRIFT"),
          "a manifest header mutation is rejected")

    mutated = copy.deepcopy(manifest)
    mutated["functions"][0]["name"] = "cna_route_that_was_removed"
    check(prefixed(verify_tool.check_manifest(mutated, live), "BOUND_SYMBOL_REMOVED_UPSTREAM"),
          "a symbol removed upstream is rejected")


def test_jni(live: dict) -> None:
    manifest = json.loads(verify_tool.MANIFEST.read_text(encoding="utf-8"))
    check(not verify_tool.check_jni(manifest),
          "the JNI adapter loads exactly the manifest's symbols")

    mutated = copy.deepcopy(manifest)
    mutated["functions"].append({"name": "cna_get_abi_version_missing_from_jni",
                                 "header": "abi.h", "ownership": "none",
                                 "returnType": "uint32_t", "parameters": []})
    check(prefixed(verify_tool.check_jni(mutated), "MANIFEST_SYMBOL_MISSING_FROM_JNI"),
          "a manifest symbol the JNI adapter never loads is rejected")

    mutated = copy.deepcopy(manifest)
    del mutated["functions"][0]
    check(prefixed(verify_tool.check_jni(mutated), "JNI_SYMBOL_MISSING_FROM_MANIFEST"),
          "a JNI symbol absent from the manifest is rejected")

    source = "\n".join(path.read_text(encoding="utf-8")
                       for path in verify_tool.JNI_SOURCES)
    slots = verify_tool.ROUTE_PATTERN.findall(source)
    check(len(slots) == len(manifest["functions"]),
          "every dispatch-table slot is declared through CNA_JNI_ROUTE")
    symbols = {symbol for symbol, _ in slots}
    check(symbols == {entry["name"] for entry in manifest["functions"]},
          "every CNA_JNI_ROUTE names a manifest symbol")


def test_java_abi() -> None:
    """The Java load-time constant must be pinned to the manifest, and a drift must fail."""
    manifest = json.loads(verify_tool.MANIFEST.read_text(encoding="utf-8"))
    check(not verify_tool.check_java_abi(manifest),
          "the Java load-time ABI constant matches the manifest")
    drifted = copy.deepcopy(manifest)
    drifted["compiledAbi"] = "0.99.0"
    check(prefixed(verify_tool.check_java_abi(drifted), "JAVA_COMPILED_ABI_DRIFT"),
          "a manifest that moved without the Java constant is rejected")


def test_policy() -> None:
    manifest = json.loads(verify_tool.MANIFEST.read_text(encoding="utf-8"))
    major, minor, patch = (int(value) for value in manifest["compiledAbi"].split("."))

    def encode(a: int, b: int, c: int) -> int:
        return (a << 16) | (b << 8) | c

    check(not verify_tool.check_policy(manifest, encode(major, minor, patch)),
          "the exact compiled ABI is accepted")
    check(not verify_tool.check_policy(manifest, encode(major, minor, patch + 7)),
          "a differing patch component is accepted")
    check(prefixed(verify_tool.check_policy(manifest, encode(major + 1, minor, patch)),
                   "ABI_MAJOR_INCOMPATIBLE"),
          "an incompatible ABI major is rejected")
    check(prefixed(verify_tool.check_policy(manifest, encode(major, minor + 1, patch)),
                   "ABI_MINOR_INCOMPATIBLE"),
          "a different ABI minor is rejected, because a 0.x minor may be incompatible")
    check(prefixed(verify_tool.check_policy(manifest, encode(major, minor - 1, patch)),
                   "ABI_MINOR_INCOMPATIBLE"),
          "an older ABI minor is rejected")


def test_coverage(cna_root: Path) -> None:
    report = coverage_tool.build(cna_root)
    counts = report["classificationCounts"]
    check(counts.get("UNMAPPED_REQUIRES_REVIEW", 0) == 0,
          "every canonical CNA C API function carries an explicit classification")
    check(report["canonicalFunctions"] == len(report["functions"]),
          "the coverage report covers every canonical function exactly once")
    check(counts.get("XNA_BACKING", 0) > 0,
          "the strict XNA projection is shown to reach real native routes")

    entries = coverage_tool.c_functions("\n".join(
        path.read_text(encoding="utf-8") for path in coverage_tool.JNI_SOURCES))
    check(sum(1 for name in entries if name.startswith("Java_")) > 0,
          "the JNI adapter's entry points are resolved from the C source")
    check("Java_org_openeggbert_cna_internal_NativeAudio_nativeDestroyCue" in entries,
          "a macro-generated JNI entry point is expanded rather than dropped")

    # A facade often hands a route to a functional interface as `Type::method` rather than
    # calling it. Missing that form would report a reached route as unreached.
    check(coverage_tool.called_names("a(); Type::b; c ();") == {"a", "b", "c"},
          "a Java method reference counts as a call site")
    check("nativeRun" not in coverage_tool.called_names("// nativeRun"),
          "a bare identifier is not a call site")

    rules = json.loads(coverage_tool.RULES.read_text(encoding="utf-8"))
    unmatched = coverage_tool.match_rule({"match": {}}, "cna_anything", "anything.h")
    check(not unmatched, "an empty rule never matches")
    check(all("purposeReason" in rule and rule["purposeReason"].strip()
              for rule in rules["rules"]),
          "every coverage rule states why the route exists")
    check(all(rule["classification"] in rules["classifications"] for rule in rules["rules"]),
          "every coverage rule uses a declared classification")
    test_binding_status(report, rules)


def test_binding_status(report: dict, rules: dict) -> None:
    """Purpose and binding status are two questions, and the rules must answer both.

    The failure this exists for is the one the previous session found by hand: a rule
    whose whole text explained why a route is a CNA extension rather than XNA, which is
    an answer to a different question and reads like an answer to this one.  Every check
    here is exercised by mutating a rule set until it fails, so a check that stopped
    working is itself a failure.
    """
    bound = {record["name"] for record in report["functions"] if record["bound"]}
    check(not report["ruleProblems"],
          "every coverage rule answers both the purpose and the binding question")
    check(all(rule["bindingStatus"] in coverage_tool.BINDING_STATUSES
              for rule in rules["rules"]),
          "every coverage rule uses a declared binding status")
    check(all(status in rules["bindingStatuses"]
              for status in coverage_tool.BINDING_STATUSES),
          "the rules file documents every binding status the tool accepts")
    check(all(record.get("bindingStatus") for record in report["functions"]),
          "every canonical route carries a binding status")
    check(all(record["bindingStatus"] == "BOUND" for record in report["functions"]
              if record["bound"]),
          "a bound route's binding status is derived from being bound, never declared")

    def mutated(change) -> list[str]:
        copied = copy.deepcopy(rules)
        change(copied)
        return coverage_tool.rule_problems(copied, bound)

    # 1. A rule that answers only the purpose question is the exact defect this schema
    #    was split to make visible.
    def drop_binding_reason(value: dict) -> None:
        value["rules"][0].pop("bindingReason")
    check(prefixed(mutated(drop_binding_reason), "MISSING_BINDING_REASON"),
          "a rule that gives a purpose but no binding reason is refused")

    def drop_binding_status(value: dict) -> None:
        value["rules"][0].pop("bindingStatus")
    check(prefixed(mutated(drop_binding_status), "MISSING_BINDING_STATUS"),
          "a rule that gives a purpose but no binding status is refused")

    def drop_purpose(value: dict) -> None:
        value["rules"][0]["purposeReason"] = "   "
    check(prefixed(mutated(drop_purpose), "MISSING_PURPOSE_REASON"),
          "a rule that gives a binding status but no purpose is refused")

    # 2. An extension-purpose route can still be locally actionable. The two axes are
    #    independent, and a schema that could not express this would have re-created the
    #    conflation it replaced.
    def extension_is_actionable(value: dict) -> None:
        rule = next(item for item in value["rules"]
                    if item["classification"] == coverage_tool.CENSUS_PURPOSE)
        rule["bindingStatus"] = "ACTIONABLE_LOCAL"
        rule["bindingReason"] = "Nothing outside this repository blocks it."
        rule.pop("evidence", None)
    check(not mutated(extension_is_actionable),
          "a CNA-extension route may be ACTIONABLE_LOCAL without further justification")

    # 3. A route the reference API declares is not left out on taste.
    def xna_without_evidence(value: dict) -> None:
        rule = next(item for item in value["rules"]
                    if item["classification"] in coverage_tool.PURPOSES_ALLOWING_DEFERRAL)
        rule["bindingStatus"] = "DELIBERATE_NON_BINDING"
        rule["bindingReason"] = "Decided against."
        rule.pop("evidence", None)
    check(prefixed(mutated(xna_without_evidence), "XNA_NON_BINDING_WITHOUT_EVIDENCE"),
          "an XNA-backing route may be DELIBERATE_NON_BINDING only with stated evidence")

    def xna_with_evidence(value: dict) -> None:
        xna_without_evidence(value)
        rule = next(item for item in value["rules"]
                    if item["bindingStatus"] == "DELIBERATE_NON_BINDING"
                    and item["classification"] in coverage_tool.PURPOSES_ALLOWING_DEFERRAL)
        rule["evidence"] = "Measured in probe.c; the managed path answers identically."
    check(not mutated(xna_with_evidence),
          "the same rule passes once it states the evidence")

    # 4. A blocker that outlived its block. Nothing reads a stale rule, so nothing else
    #    would ever report it.
    def blocker_over_bound(value: dict) -> None:
        value["rules"][0]["match"] = {"symbols": [sorted(bound)[0]]}
    check(prefixed(mutated(blocker_over_bound), "BLOCKER_RULE_MATCHES_BOUND_ROUTE"),
          "a rule that still names a route something has since bound is refused")

    # 4b. The same staleness one level up, and the one that actually kept happening: a
    #     prefix or contains rule names no symbol, so check 4 cannot see it, and a
    #     blocker that no longer decides any route still reads as a live reason not to
    #     bind. Three families were found blocked that way, each on a reason that had
    #     been true when it was written.
    each_decides_one = {index: 1 for index in range(len(rules["rules"]))}
    check(not prefixed(coverage_tool.rule_problems(rules, bound, each_decides_one),
                       "STALE_BLOCKER_RULE_DECIDES_NOTHING"),
          "a blocker that still decides an unbound route is not called stale")
    # Every upstream blocker this repository recorded has been lifted, so the checks below
    # plant one: the first rule, restated as a blocker, exactly as a new finding would be.
    blocked = copy.deepcopy(rules)
    blocked["rules"][0]["bindingStatus"] = "BLOCKED_UPSTREAM"
    check(not prefixed(coverage_tool.rule_problems(blocked, bound, each_decides_one),
                       "STALE_BLOCKER_RULE_DECIDES_NOTHING"),
          "a planted blocker that still decides an unbound route is not called stale")
    emptied = dict(each_decides_one)
    emptied[0] = 0
    check(prefixed(coverage_tool.rule_problems(blocked, bound, emptied),
                   "STALE_BLOCKER_RULE_DECIDES_NOTHING"),
          "a blocker that decides no unbound route is refused, even with others still live")
    check(not prefixed(coverage_tool.rule_problems(rules, bound),
                       "STALE_BLOCKER_RULE_DECIDES_NOTHING"),
          "a caller with no inventory gets no staleness verdict rather than a guess")

    def non_blocker_decides_nothing(value: dict) -> None:
        for rule in value["rules"]:
            if rule["bindingStatus"].startswith("BLOCKED_"):
                rule["bindingStatus"] = "DELIBERATE_NON_BINDING"
                rule["evidence"] = "Measured; the managed path answers identically."
    copied = copy.deepcopy(blocked)
    non_blocker_decides_nothing(copied)
    check(not prefixed(coverage_tool.rule_problems(copied, bound, {}),
                       "STALE_BLOCKER_RULE_DECIDES_NOTHING"),
          "only a blocker is stale for deciding nothing: a decision still stands")
    check(prefixed(coverage_tool.rule_problems(blocked, bound, {}),
                   "STALE_BLOCKER_RULE_DECIDES_NOTHING"),
          "the same rule set with its blockers intact is refused when none decides anything")

    # 4c. A two-call pair is one operation, so its halves are one decision. The failure
    #     this catches is the one that hid a lifted blocker: _get_type_name_size had a
    #     rule and _get_type_name_byte_count did not, so eight size halves fell through
    #     to a header rule while the copy halves beside them carried the true reason.
    pairs = coverage_tool.size_copy_pairs({record["name"] for record in report["functions"]})
    check(len(pairs) > 100, "the two-call pairs are found, in all three size spellings")
    exceptions = rules.get("pairExceptions", [])
    check(not coverage_tool.pair_problems(report["functions"], exceptions),
          "every two-call pair is decided as one, bar the exceptions the rules record")
    check(not exceptions,
          "no pair is decided apart: the one exception went with JAVA-UPSTREAM-018")

    def apart(field: str, value: str) -> list[str]:
        copied = copy.deepcopy(report["functions"])
        by_name = {record["name"]: record for record in copied}
        size, _ = next((first, second) for first, second in pairs
                       if (first, second) not in
                       {(entry["size"], entry["copy"]) for entry in exceptions})
        by_name[size][field] = value
        return coverage_tool.pair_problems(copied, exceptions)

    check(prefixed(apart("bound", True), "HALF_BOUND_PAIR"),
          "a pair with one half bound and the other not is refused")
    check(prefixed(apart("classification", "NOT_USEFUL_IN_JAVA"), "PAIR_CLASSIFIED_APART")
          or prefixed(apart("classification", "CNA_EXTENSION_CANDIDATE"),
                      "PAIR_CLASSIFIED_APART"),
          "a pair whose halves are classified apart is refused")
    check(prefixed(apart("bindingStatus", "ACTIONABLE_LOCAL"), "PAIR_DECIDED_APART"),
          "a pair whose halves carry different binding statuses is refused")
    check(prefixed(coverage_tool.pair_problems(
              report["functions"],
              [{"size": "cna_not_a_route_get_size", "copy": "cna_not_a_route_copy",
                "reason": "invented"}]),
          "PAIR_EXCEPTION_NAMES_NOTHING"),
          "an exception for a pair that does not exist is refused")
    check(prefixed(coverage_tool.pair_problems(
              report["functions"],
              [{"size": pairs[0][0], "copy": pairs[0][1], "reason": "  "}]),
          "PAIR_EXCEPTION_WITHOUT_REASON"),
          "an exception that states no reason is refused")

    # 5. DEFERRED_TRACKED is the one status that names a backlog task instead of a
    #    measurement, so it is fenced to the purpose that has one.
    def deferral_outside_backlog(value: dict) -> None:
        rule = next(item for item in value["rules"]
                    if item["classification"] == coverage_tool.CENSUS_PURPOSE)
        rule["bindingStatus"] = "DEFERRED_TRACKED"
        rule["task"] = "JAVA-EXT-999"
    check(prefixed(mutated(deferral_outside_backlog), "DEFERRAL_OUTSIDE_XNA_BACKLOG"),
          "a CNA-extension route may not be deferred to a backlog task instead of classified")

    def deferral_without_task(value: dict) -> None:
        rule = next(item for item in value["rules"]
                    if item["bindingStatus"] == "DEFERRED_TRACKED")
        rule.pop("task")
    check(prefixed(mutated(deferral_without_task), "DEFERRAL_WITHOUT_TASK"),
          "a deferred XNA-backing route must name the task that owns it")

    # A `native` declaration names its own method, so an unfiltered call scan reads it
    # as a call site and every generated route reports itself as reached. That would
    # make the reachability question unanswerable, which is why it is filtered.
    declaration = ("class Probe {\n"
                   "    public static native int probeRoute(long handle, long[] out);\n"
                   "}\n")
    check("probeRoute" not in coverage_tool.called_names(
              coverage_tool.without_native_declarations(declaration)),
          "a native declaration is not a call site for itself")
    check("probeRoute" in coverage_tool.called_names(
              coverage_tool.without_native_declarations(
                  "class Probe {\n    void use() { probeRoute(0L, null); }\n}\n")),
          "a real call to a native method is still a call site")


def test_generator(live: dict) -> None:
    """The generated boundary must not silently narrow or silently drop data."""

    def plan(symbol: str) -> dict:
        return generator_tool.plan({"java": "probe", "symbol": symbol}, live)

    def struct_step(entry: dict) -> dict:
        return next(step for step in entry["steps"]
                    if step["shape"] in ("struct", "struct_array", "struct_value"))

    # A C double must travel in its own double[] carrier. Sharing the float one would
    # narrow every reading silently, which is the failure this separation prevents.
    compass = plan("cna_compass_get_current_value")
    reading = struct_step(compass)
    check([path for path, _ in reading["doubles"]]
          == ["heading_accuracy", "magnetic_heading", "true_heading"],
          "a struct's C double leaves travel in their own double carrier")
    check(all(leaf == "float" for _, leaf in reading["floating"]),
          "the float carrier holds no double leaf")
    _, parameters = generator_tool.java_signature(compass)
    check(any(value.startswith("double[]") for value in parameters),
          "a struct with double leaves declares a double[] parameter")
    check("jdoubleArray" in generator_tool.render_c("Probe", [compass]),
          "the generated adapter reads the double carrier as a jdoubleArray")

    # A structure carrying CNA_StringView fields: each one is its own byte[] rather than a
    # slot in a shared array, because a view is a pointer and a length. Before this, a whole
    # route was refused because flatten_struct reached the view's const char*.
    discovered = plan("cna_available_network_session_create_ext")
    info = struct_step(discovered)
    check([path for path, _ in info["views"]] == ["host_gamertag", "host_address"],
          "a struct's CNA_StringView fields are collected as views, in declaration order")
    check(all(leaf != "CNA_StringView" for _, leaf in info["integral"]),
          "a view never lands in the integral carrier")
    _, parameters = generator_tool.java_signature(discovered)
    check("byte[] createInfoHostAddress" in parameters,
          "each view field declares its own byte[] parameter named after the field")
    adapter = generator_tool.render_c("Probe", [discovered])
    check("create_info_value.host_address.data = (const char*)createInfoHostAddress_bytes"
          in adapter,
          "the adapter points the view at the pinned Java bytes")
    check("create_info_value.host_address.byte_length = (uint64_t)createInfoHostAddress_size"
          in adapter,
          "the adapter sets the view's length from the array, not from a terminator")
    check(adapter.count("ReleaseByteArrayElements(environment, createInfoHostAddress") == 1,
          "the pinned bytes are released exactly once")

    # A non-const struct pointer is an output, or it is read and written, and the declaration
    # does not say which. Guessing wrong is silent: an in/out structure treated as an output
    # starts zeroed, so the caller's values are discarded and the route answers about a structure
    # nobody asked about.
    try:
        plan("cna_viewport_set_bounds")
        check(False, "an undeclared non-const struct pointer is refused rather than guessed at")
    except generator_tool.Unsupported:
        check(True, "an undeclared non-const struct pointer is refused rather than guessed at")

    declared = generator_tool.plan(
        {"java": "probe", "symbol": "cna_viewport_set_bounds", "inOut": ["viewport"]}, live)
    adapter = generator_tool.render_c("Probe", [declared])
    check("GetLongArrayRegion" in adapter,
          "a declared in/out structure reads the caller's values in")
    check("SetLongArrayRegion" in adapter,
          "and writes the corrected values back out")

    # CNA's own names for a pure output still pass without a declaration, which is what keeps
    # every copy-out route working: `out_*` and `destination` are its two spellings.
    check(struct_step(plan("cna_model_copy_bone_transforms"))["shape"] == "struct_array",
          "a parameter named destination is still read as an output")

    # A route that acquires more than one thing must release what it holds when a later
    # acquisition fails. Before this, the second failed pin of a four-array route returned
    # straight out and stranded the first -- on an out-of-memory path, which is when leaking is
    # least affordable.
    multi = plan("cna_available_network_session_create_ext")
    adapter = generator_tool.render_c("Probe", [multi])
    first = adapter.index("createInfoHostGamertag_bytes == NULL")
    second = adapter.index("createInfoHostAddress_bytes == NULL")
    check("ReleaseByteArrayElements" not in adapter[first:adapter.index("}", first)],
          "the first acquisition's failure has nothing to release")
    check("createInfoHostGamertag_bytes, JNI_ABORT"
          in adapter[second:adapter.index("return (jint)CNA_RESULT_OUT_OF_MEMORY", second)],
          "a later acquisition's failure releases the earlier one")

    # And each is still released exactly once on the success path. The earlier one appears
    # twice in the file because the later one's failure branch also releases it; the last
    # acquisition appears once, because a pin that failed has nothing to give back.
    check(adapter.count("ReleaseByteArrayElements(environment, createInfoHostGamertag, ") == 2,
          "an earlier acquisition is released on the success path and on the later failure")
    check(adapter.count("ReleaseByteArrayElements(environment, createInfoHostAddress, ") == 1,
          "the last acquisition is released once, because a pin that failed acquired nothing")

    # An input array whose length CNA states in prose rather than in a count parameter. The
    # generator will not infer it -- a wrong guess is a read past the end inside CNA -- so
    # routes.json declares it and the adapter checks the caller's array against the declaration.
    skeleton = generator_tool.plan({
        "java": "probe", "symbol": "cna_cnb_model_set_skeleton",
        "arrayLengths": {
            "bind_pose": {"per": 16, "count": "joint_count"},
            "inverse_bind_pose": {"per": 16, "count": "joint_count"},
            "root_prefix": {"per": 16, "count": "joint_count", "nullable": True},
        }}, live)
    sized = [step for step in skeleton["steps"] if step["shape"] == "sized_array"]
    check(len(sized) == 3, "each declared array becomes its own sized_array step")
    adapter = generator_tool.render_c("Probe", [skeleton])
    guard = adapter[:adapter.index("GetIntArrayElements")]
    check(guard.count("CNA_RESULT_INVALID_ARGUMENT") == 3,
          "every declared length is checked before anything is acquired, so a wrong length "
          "refuses without leaking what earlier parameters pinned")
    check("bind_pose == NULL ||" in adapter,
          "a non-nullable declared array refuses null")
    check("root_prefix != NULL &&" in adapter,
          "a nullable declared array accepts null and checks the length only when present")
    check("16 * (*environment)->GetArrayLength(environment, hierarchy)" in adapter,
          "a length stated per element of another array resolves to that array's own length")

    # A fixed extent needs no count at all.
    bone = generator_tool.plan({"java": "probe", "symbol": "cna_cnb_model_add_bone",
                                "arrayLengths": {"transform": {"length": 16}}}, live)
    check("!= (jsize)16" in generator_tool.render_c("Probe", [bone]),
          "a fixed-extent array is checked against the literal length")

    # And a declaration naming something the route cannot read up front is refused, rather than
    # emitting C that names a variable which does not exist.
    try:
        generator_tool.plan({"java": "probe", "symbol": "cna_cnb_model_add_bone",
                             "arrayLengths": {"transform": {"per": 4, "count": "nonsense"}}},
                            live)
        check(False, "a declared count that is not a readable parameter is refused")
    except generator_tool.Unsupported:
        check(True, "a declared count that is not a readable parameter is refused")

    # And a view whose lifetime the declaration does not state is refused rather than guessed.
    # An output structure's view would point at memory CNA owns on terms the header does not
    # give, and an array of such structures would need one pointer to outlive its element's
    # marshalling. Both are refused, which is checked by asking the planner to accept the same
    # structure in each of those positions.
    for direction, position in (("CNA_AvailableNetworkSessionCreateInfo*", "output structure"),
                                ("const CNA_AvailableNetworkSessionCreateInfo*", "array")):
        synthetic = dict(live)
        synthetic["functions"] = dict(live["functions"])
        parameters = [{"type": direction, "name": "info"}]
        if position == "array":
            parameters.append({"type": "uint64_t", "name": "count"})
        synthetic["functions"]["cna_probe_views"] = {
            "name": "cna_probe_views", "header": "probe.h", "returnType": "CNA_Result",
            "parameters": parameters,
        }
        try:
            generator_tool.plan({"java": "probe", "symbol": "cna_probe_views"}, synthetic)
            check(False, f"a CNA_StringView in an {position} is refused rather than guessed at")
        except generator_tool.Unsupported:
            check(True, f"a CNA_StringView in an {position} is refused rather than guessed at")

    # An output array of structs has to be copied back, or the route returns success
    # and Java sees nothing. This was a real defect: the only such route bound at the
    # time never wrote a single element back.
    copy_to = plan("cna_network_session_properties_copy_to")
    check(struct_step(copy_to)["shape"] == "struct_array" and not struct_step(copy_to)["input"],
          "a non-const struct pointer followed by a count is an output struct array")
    emitted = generator_tool.render_c("Probe", [copy_to])
    check("SetLongArrayRegion" in emitted,
          "an output array of structs is copied back to Java")
    check(emitted.index("SetLongArrayRegion") < emitted.rindex("free(destination_values);"),
          "the copy back happens before the C buffer is freed")

    # One struct is split across up to four parallel Java arrays, and only one of them supplies
    # the element count. A caller whose other arrays are shorter would have every element past
    # their end read out of bounds in C -- a heap overread inside the marshalling loop, not a
    # Java exception -- so a mismatch has to be refused before anything is allocated.
    vertices = plan("cna_debug_draw_copy_vertices")
    check(struct_step(vertices)["shape"] == "struct_array",
          "a debug renderer's copy-out is an array of structs")
    emitted = generator_tool.render_c("Probe", [vertices])
    check("destination_count * 4" in emitted and "destination_count * 3" in emitted,
          "every parallel carrier's length is checked against the element count")
    check("return (jint)CNA_RESULT_INVALID_ARGUMENT;" in emitted,
          "a mismatched carrier is refused rather than read past its end")
    guard = emitted.index("CNA_RESULT_INVALID_ARGUMENT")
    check(guard < emitted.index("calloc("),
          "the refusal happens before anything is allocated")
    check(guard < emitted.index("GetLongArrayRegion"),
          "and before any carrier is read")

    # Shapes no live route has any more. The routes that had them went with CNA's engine layer at
    # ABI 0.30, but the generator still has to refuse and marshal them correctly, so they are
    # declared here as synthetic functions and structures beside the live inventory.
    synthetic = copy.deepcopy(live)
    synthetic["functions"]["cna_probe_corners"] = {
        "name": "cna_probe_corners", "header": "probe.h", "returnType": "CNA_Result",
        "parameters": [
            {"type": "const CNA_BoundingBox*", "name": "box", "doc": "The box."},
            {"type": "CNA_Vector3*", "name": "out_corners",
             "doc": "Destination for eight corners."}]}
    synthetic["functions"]["cna_probe_set_environment"] = {
        "name": "cna_probe_set_environment", "header": "probe.h", "returnType": "CNA_Result",
        "parameters": [{"type": "CNA_Handle", "name": "probe", "doc": "The probe."},
                       {"type": "CNA_Handle", "name": "environment", "doc": "Its environment."}]}
    synthetic["structures"]["CNA_ProbeSamplerSet"] = {
        "name": "CNA_ProbeSamplerSet", "header": "probe.h", "fields": [
            {"type": "uint32_t", "name": "struct_size"},
            {"type": "uint32_t", "name": "struct_version"},
            {"type": "CNA_SamplerState[3]", "name": "samplers"}]}
    synthetic["structures"]["CNA_ProbeContext"] = {
        "name": "CNA_ProbeContext", "header": "probe.h", "fields": [
            {"type": "uint32_t", "name": "struct_size"},
            {"type": "uint32_t", "name": "struct_version"},
            {"type": "CNA_Handle", "name": "source"},
            {"type": "int32_t", "name": "width"},
            {"type": "float", "name": "elapsed_seconds"},
            {"type": "const CNA_SamplerState*", "name": "settings"},
            {"type": "float", "name": "tail"}]}
    synthetic["constants"]["CNA_PROBE_CONTEXT_SIZE_V1"] = {
        "name": "CNA_PROBE_CONTEXT_SIZE_V1", "header": "probe.h", "value": "UINT32_C(32)",
        "integer": 32}
    synthetic["functions"]["cna_probe_apply"] = {
        "name": "cna_probe_apply", "header": "probe.h", "returnType": "CNA_Result",
        "parameters": [{"type": "CNA_Handle", "name": "chain", "doc": "The chain."},
                       {"type": "const CNA_ProbeContext*", "name": "context",
                        "doc": "The context."}]}

    # A bare `T*` is one structure or an array of them, and the C declaration does not say
    # which. CNA states the difference in prose -- "destination for eight corners" -- so the
    # generator reads the parameter's own documentation and refuses rather than marshalling one
    # element and handing it to a function that writes eight. This was a real defect: two
    # cascade helpers were generated as single Vector3 parameters, which is a stack overflow on
    # the way out and a heap overread on the way in.
    corners = generator_tool.plan(
        {"java": "probe", "symbol": "cna_probe_corners",
         "arrayLengths": {"out_corners": {"length": 8}}}, synthetic)
    step = next(entry for entry in corners["steps"] if entry["name"] == "out_corners")
    check(step["shape"] == "struct_array" and step["extent"] == 8,
          "a declared eight-element destination is marshalled as eight, not one")
    emitted = generator_tool.render_c("Probe", [corners])
    check("const jsize out_corners_count = 8;" in emitted,
          "the extent is the declaration's, not the Java array's length")
    check("!= out_corners_count * 3" in emitted,
          "a Java array of the wrong length is refused rather than overrun")
    check("(uint64_t)out_corners_count" not in emitted,
          "a fixed-extent array passes no count, because CNA already knows it")

    try:
        generator_tool.plan({"java": "probe", "symbol": "cna_probe_corners"}, synthetic)
        check(False, "an undeclared counted destination is refused rather than guessed at")
    except generator_tool.Unsupported as refusal:
        check("arrayLengths" in str(refusal),
              "an undeclared counted destination is refused rather than guessed at")

    # The detector reads prose, so it has to tell a number of *things* from a number. A value
    # such as "range twenty" is not a count, and "two zero positions" describes one structure
    # with two fields; neither is an array. A number alone never fires, and a route can say so
    # explicitly.
    check(not generator_tool.counts_more_than_one(
              "Receives a rectangle at the origin, half a unit across each way, white, at "
              "intensity one and range twenty."),
          "a number that is a value rather than a count does not fire the detector")
    check(generator_tool.counts_more_than_one("Destination for eight corners."),
          "a number of things does")
    check(generator_tool.counts_more_than_one(
              "Receives CNA_BOUNDING_BOX_CORNER_COUNT corners."),
          "and so does one of CNA's own count constants")
    sticks = generator_tool.plan(
        {"java": "probe", "symbol": "cna_gamepad_thumb_sticks_init",
         "singleStructs": ["out_thumb_sticks"]}, live)
    check(any(entry["shape"] == "struct" and entry["name"] == "out_thumb_sticks"
              for entry in sticks["steps"]),
          "singleStructs says the count in the prose is the structure's own fields")

    # And the detector must not fire on an ordinary single structure, or every route taking a
    # box or a matrix would need a declaration it does not want.
    single = plan("cna_debug_draw_add_box")
    check(any(entry["shape"] == "struct" and entry["name"] == "bounds"
              for entry in single["steps"]),
          "a single structure whose documentation names no count stays a single structure")

    # The adapter's own JNI parameters are called `environment` and `declaring_class`, and a CNA
    # route whose parameter is called `environment` too cannot share that name. The first
    # version of this generator emitted exactly that, and the whole adapter stopped compiling.
    renamed = generator_tool.plan({"java": "probe", "symbol": "cna_probe_set_environment"},
                                  synthetic)
    emitted = generator_tool.render_c("Probe", [renamed])
    check("jlong environment_parameter" in emitted,
          "a CNA parameter that collides with the adapter's own is renamed in C")
    check("JNIEnv* environment," in emitted,
          "and the adapter keeps its own name")
    _, parameters = generator_tool.java_signature(renamed)
    check(any(value == "long environment" for value in parameters),
          "the Java declaration is untouched, because nothing a caller sees changed")

    # An input struct must not be written back over the caller's data.
    haptic = plan("cna_haptic_device_get_is_effect_supported")
    effect = next(step for step in haptic["steps"] if step["shape"] == "struct")
    check(effect["input"], "a const struct pointer is an input parameter")
    check("SetLongArrayRegion" not in generator_tool.render_c("Probe", [haptic]),
          "an input struct is never written back over the caller's array")

    # A required-size output has to cross even when the buffer was too small, or the two-call
    # size protocol -- ask with no buffer, be told the size, ask again -- can never work.
    encode = plan("cna_cnb_encode_texture2d")
    emitted = generator_tool.render_c("Probe", [encode])
    check("call_result == CNA_RESULT_BUFFER_TOO_SMALL" in emitted,
          "an output is copied back when the caller's buffer was too small")
    check(emitted.count("if (call_result == CNA_RESULT_SUCCESS)") == 1,
          "the array write-back still happens only on success, because a refused copy wrote "
          "nothing into the buffer")

    # A fixed array inside a structure is a layout, not a shape to refuse: an extent written as
    # one of CNA's own macros, and elements that are themselves structures, both expand.
    touches = generator_tool.flatten_struct("CNA_TouchState", live)
    check(("touches[7].previous_position.y", "float") in touches,
          "a macro extent is resolved through CNA's own constant rather than refused")
    check(sum(1 for path, _ in touches
              if path.startswith("touches[") and path.endswith(".id")) == 8,
          "an array whose elements are structures expands element by element")
    visualization = generator_tool.flatten_struct("CNA_VisualizationData", live)
    check(("samples[255]", "float") in visualization
          and sum(1 for path, _ in visualization if path.startswith("frequencies[")) == 256,
          "and a scalar array with a macro extent expands the same way")

    # The half of that which is silent when it is wrong: every element of an array of versioned
    # structures has to be stamped, not just the first. Unstamped elements would each tell CNA
    # they were zero bytes long, and nothing in Java would ever say so.
    stamped = generator_tool.version_paths("CNA_ProbeSamplerSet", synthetic)
    check(sum(1 for path, _, _ in stamped
              if path.startswith("samplers[") and path.endswith("struct_size")) == 3,
          "every element of an array of versioned structures is stamped")
    lines = generator_tool.stamp_versions("probe", stamped)
    check("    probe.samplers[2].struct_size = (uint32_t)(sizeof probe.samplers[2]);" in lines,
          "and each is sized from itself rather than from the outer structure")
    check(not any(path.rsplit(".", 1)[-1] in generator_tool.VERSION_FIELDS
                  for path, _ in generator_tool.group_leaves(generator_tool.flatten_struct(
                      "CNA_ProbeSamplerSet", synthetic))["integral"]),
          "a nested stamped field never crosses into Java, however deeply nested it is")

    # An extent the generator cannot resolve to a plain positive integer is refused rather than
    # evaluated, because guessing at an expression is how a structure is silently mis-sized.
    for bad in ("CNA_NOT_A_CONSTANT", "0"):
        try:
            generator_tool.array_extent("probe.field", bad, live)
            check(False, f"an unresolvable array extent {bad} is refused")
        except generator_tool.Unsupported:
            check(True, f"an unresolvable array extent {bad} is refused")
    # CNA writes its counts with the standard fixed-width constant macros. That form means
    # exactly its argument, so unwrapping it is reading the header rather than evaluating an
    # expression.
    check(generator_tool.array_extent("probe.field", "CNA_TOUCH_MAX_TOUCHES", live) == 8,
          "a UINT32_C-wrapped extent is read as the integer it is")

    # CNA grows some structures by appending and documents the earlier size as a constant, so a
    # caller compiled against version one sets struct_size to it and every route still works.
    # That is the mechanism a pointer field is reached past -- a field the generator would
    # otherwise refuse the whole route over.
    context_prefix = {"stopBefore": "settings",
                      "sizeConstant": "CNA_PROBE_CONTEXT_SIZE_V1", "version": 1}
    full = generator_tool.flatten_struct("CNA_ProbeContext", synthetic, prefix=context_prefix)
    check(not any(path.startswith("settings") or path == "tail" for path, _ in full),
          "a declared prefix stops before the field it names")
    check(any(path == "elapsed_seconds" for path, _ in full),
          "and keeps everything before it")
    applied = generator_tool.plan(
        {"java": "probe", "symbol": "cna_probe_apply",
         "structPrefixes": {"context": context_prefix}}, synthetic)
    emitted = generator_tool.render_c("Probe", [applied])
    check("context_value.struct_size = (uint32_t)(CNA_PROBE_CONTEXT_SIZE_V1);" in emitted,
          "the stamped size is CNA's own constant, not sizeof")
    check("(uint32_t)(sizeof context_value)" not in emitted,
          "because sizeof would tell CNA the tail was written when it never was")

    # And the declaration is checked against the headers rather than believed: a field that
    # does not exist, or a constant CNA does not define, is refused.
    for broken in ({"stopBefore": "not_a_field", "sizeConstant":
                    "CNA_PROBE_CONTEXT_SIZE_V1", "version": 1},
                   {"stopBefore": "settings", "sizeConstant": "CNA_NOT_A_CONSTANT",
                    "version": 1}):
        try:
            generator_tool.plan({"java": "probe", "symbol": "cna_probe_apply",
                                 "structPrefixes": {"context": broken}}, synthetic)
            check(False, f"a prefix declaration that the headers contradict is refused: {broken}")
        except generator_tool.Unsupported:
            check(True, f"a prefix declaration that the headers contradict is refused")

    # Without the declaration the route stays refused, because a borrowed pointer inside a
    # structure is exactly the shape this generator will not guess at.
    try:
        generator_tool.plan({"java": "probe", "symbol": "cna_probe_apply"}, synthetic)
        check(False, "a struct carrying a pointer is refused when no prefix is declared")
    except generator_tool.Unsupported:
        check(True, "a struct carrying a pointer is refused when no prefix is declared")

    # A struct carrier that arrived null would make every GetXArrayRegion below it raise and
    # then be read as uninitialised stack -- a JVM-level fault for what is an ordinary missing
    # argument. Every input struct now refuses it with the result CNA would have given.
    reading = plan("cna_accelerometer_reading_copy_string")
    emitted = generator_tool.render_c("Probe", [reading])
    check("if (readingIntegral == NULL || readingFloating == NULL) {" in emitted,
          "a null struct carrier is refused before anything is read")
    check(emitted.index("== NULL") < emitted.index("GetLongArrayRegion"),
          "and refused before it, not after")

    # An optional structure is a different thing again: CNA documents SpriteBatch's sampler as
    # "or null" for its default, and a null pointer says that where an all-zero structure would
    # say wrap-addressed linear filtering, which is a real setting that happens to look like an
    # absence.
    sampler = generator_tool.plan(
        {"java": "probe", "symbol": "cna_sprite_batch_begin_with_effect",
         "optionalStructs": ["sampler_state"]}, live)
    emitted = generator_tool.render_c("Probe", [sampler])
    check("const CNA_SamplerState* sampler_state_pointer = NULL;" in emitted,
          "an optional structure starts as no pointer at all")
    check("if (sampler_stateIntegral != NULL && sampler_stateFloating != NULL) {" in emitted,
          "and is built only when Java sent one")
    check("sampler_state_pointer," in emitted,
          "so a caller that sent none passes NULL rather than a zeroed structure")
    without = generator_tool.render_c(
        "Probe", [plan("cna_sprite_batch_begin_with_effect")])
    check("&sampler_state_value" in without,
          "and a structure not declared optional is still passed by address")

    # The sampler is write-only on this runtime: no route reads one back and the headless
    # renderer accepts anything, so no Java assertion can catch a carrier packed in the wrong
    # order. Pinned against the live header instead -- these are the positions SamplerState's
    # snapshot for SpriteBatch.Begin writes.
    sampler_leaves = generator_tool.group_leaves(
        generator_tool.flatten_struct("CNA_SamplerState", live))
    check([path for path, _ in sampler_leaves["integral"]]
          == ["address_u", "address_v", "address_w", "filter", "max_anisotropy",
              "max_mip_level", "reserved"],
          "the sampler's integral leaves are the seven SamplerState writes, in order")
    check([path for path, _ in sampler_leaves["floating"]] == ["mip_map_level_of_detail_bias"],
          "and its one float is the level-of-detail bias")

    # An opaque `void*` buffer. C says nothing about it at all -- not what it points at, not how
    # many bytes CNA touches, and not whether that count is one parameter or the product of two --
    # so the generator refuses it until routes.json states the extent.
    try:
        plan("cna_vertex_buffer_set_data_raw")
        check(False, "an undeclared void* is refused rather than guessed at")
    except generator_tool.Unsupported:
        check(True, "an undeclared void* is refused rather than guessed at")

    # A declared extent that names something which is not a parameter of the route is refused
    # too, rather than emitting C that will not compile or, worse, reading a stale name.
    try:
        generator_tool.plan({"java": "probe", "symbol": "cna_vertex_buffer_set_data_raw",
                             "byteBuffers": {"data": {"extent": ["not_a_parameter"]}}}, live)
        check(False, "a byteBuffers extent naming a non-parameter is refused")
    except generator_tool.Unsupported:
        check(True, "a byteBuffers extent naming a non-parameter is refused")

    # An empty extent would mean "no bytes at all", which is not a shape any of these routes has
    # and would silently pass a pointer CNA reads with a size from somewhere else.
    try:
        generator_tool.plan({"java": "probe", "symbol": "cna_vertex_buffer_set_data_raw",
                             "byteBuffers": {"data": {"extent": []}}}, live)
        check(False, "a byteBuffers entry with no extent is refused")
    except generator_tool.Unsupported:
        check(True, "a byteBuffers entry with no extent is refused")

    single = generator_tool.plan({"java": "probe", "symbol": "cna_vertex_buffer_set_data_raw",
                                  "byteBuffers": {"data": {"extent": ["data_byte_count"]}}}, live)
    _, parameters = generator_tool.java_signature(single)
    check(parameters == ["long vertexBuffer", "byte[] data", "long dataByteCount",
                         "long vertexCount", "int vertexStride"],
          "a declared void* is a byte[] and its extent stays a parameter of its own")
    adapter = generator_tool.render_c("Probe", [single])
    check("(const void*)data_bytes" in adapter,
          "a const void* input reaches CNA as a const void*")
    check("data_bytes, JNI_ABORT" in adapter,
          "and an input's pinned bytes are released without copying back")

    # The output form differs in exactly two places: the cast and the release mode. Getting the
    # second wrong would leave every read-back buffer holding what it held before the call.
    output = generator_tool.plan(
        {"java": "probe", "symbol": "cna_vertex_buffer_get_data_raw",
         "byteBuffers": {"destination": {"extent": ["destination_byte_count"]}}}, live)
    adapter = generator_tool.render_c("Probe", [output])
    check("(void*)destination_bytes" in adapter,
          "a non-const void* output reaches CNA as a writable void*")
    check("destination_bytes, 0);" in adapter,
          "and an output's pinned bytes are copied back on release")

    # The whole reason the extent is declared rather than inferred: the adapter checks the Java
    # array against what it was told to pass, factor by factor, and each multiplication is
    # checked BEFORE it happens. A product that overflowed would wrap into a small number that
    # passed the final comparison and let CNA read far past the end of a pinned array. The two
    # factors here are the route's vertex count and stride, declared as the extent for the test.
    product = generator_tool.plan(
        {"java": "probe", "symbol": "cna_vertex_buffer_set_data_raw",
         "byteBuffers": {"data": {"extent": ["vertex_count", "vertex_stride"]}}}, live)
    adapter = generator_tool.render_c("Probe", [product])
    check("data_required > (jlong)data_length / (jlong)vertex_count" in adapter,
          "the first factor is checked against the array before it is multiplied in")
    check("data_required > (jlong)data_length / (jlong)vertex_stride" in adapter,
          "and so is the second")
    check("(jlong)vertex_count < 0" in adapter and "(jlong)vertex_stride < 0" in adapter,
          "a negative extent is refused rather than sign-extended into a huge one")
    check("return (jint)CNA_RESULT_INVALID_ARGUMENT;" in adapter,
          "and a refused extent never reaches CNA")
    _, parameters = generator_tool.java_signature(product)
    check(parameters == ["long vertexBuffer", "byte[] data", "long dataByteCount",
                         "long vertexCount", "int vertexStride"],
          "both extent parameters stay in the Java signature, because CNA reads both")

    # A counted array whose count is NOT its length. A vec3 uniform array is three tightly packed
    # floats per element, so a float[] of 3n floats is n vectors -- and both C parameters are just
    # floats and a count, so nothing in the declaration says which. Undeclared, the generator
    # passes the length, which would describe an array three times the size the shader declares.
    packed = generator_tool.plan(
        {"java": "probe", "symbol": "cna_shader_effect_set_uniform_vec3_array",
         "elementFloats": {"values": 3}}, live)
    _, parameters = generator_tool.java_signature(packed)
    check(parameters == ["long effect", "byte[] name", "float[] values"],
          "a grouped array is one Java float[] and the count disappears")
    adapter = generator_tool.render_c("Probe", [packed])
    check("(int32_t)(values_size / 3)" in adapter,
          "the count CNA is given is the element count, not the float count")
    check("if (values_size % 3 != 0) {" in adapter,
          "and a length that is not a whole number of elements is refused")
    check("return (jint)CNA_RESULT_INVALID_ARGUMENT;" in adapter,
          "refused before CNA is called")
    # The refusal path has to give back everything the earlier parameters pinned, which for this
    # route is the name's bytes as well as the array's own.
    refusal = adapter[adapter.index("if (values_size % 3 != 0) {"):]
    refusal = refusal[:refusal.index("CNA_Result call_result")]
    check("ReleaseFloatArrayElements" in refusal and "ReleaseByteArrayElements" in refusal,
          "and the refusal releases the array and the name it had already pinned")

    # Undeclared, the same route passes the float count -- which is the bug the declaration
    # exists to prevent, and is what makes stating it worth the words.
    plain = generator_tool.plan(
        {"java": "probe", "symbol": "cna_shader_effect_set_uniform_vec3_array"}, live)
    plain_adapter = generator_tool.render_c("Probe", [plain])
    check("(int32_t)values_size" in plain_adapter and "values_size % " not in plain_adapter,
          "an undeclared grouping is one element per float, which is what the C says")

    # A grouping of zero or less would divide by zero or pass a negative count.
    try:
        generator_tool.plan({"java": "probe", "symbol": "cna_shader_effect_set_uniform_vec3_array",
                             "elementFloats": {"values": 0}}, live)
        check(False, "a non-positive elementFloats grouping is refused")
    except generator_tool.Unsupported:
        check(True, "a non-positive elementFloats grouping is refused")

    # A trailing int32_t is read as a count only where CNA named it one. Anything else next to a
    # pointer might be an offset, a stride or a mode, and reading it as a length would be the kind
    # of guess this generator exists to refuse.
    check(generator_tool.plan(
        {"java": "probe", "symbol": "cna_cnb_cnj_result_copy_bytes"}, live)["steps"][1]["shape"]
        == "array",
        "a uint64_t capacity beside a copy-out buffer is still read as its length")

    # An array of CNA_StringView inputs: a message box's button labels. It is the one string
    # shape that cannot be pinned in place -- several pinned arrays at once would each need a
    # local reference held for as long as they are pinned, and JNI promises only sixteen -- so
    # the elements are copied. What the plan has to get right is that the count parameter
    # disappears into the Java array, exactly as it does for a scalar array.
    labels = generator_tool.plan(
        {"java": "probe", "symbol": "cna_message_box_show_ext"}, live)
    check([step["shape"] for step in labels["steps"]]
          == ["value", "value", "string", "string", "string_array", "out"],
          "a CNA_StringView array followed by its count is one string_array step")
    check("button_count" not in [step["name"] for step in labels["steps"]],
          "and the count parameter disappears, because a Java array carries its own length")
    check(generator_tool.java_signature(labels)[1][4] == "byte[][] buttonLabels",
          "the Java declaration takes one byte[][] rather than a string and a count")
    rendered = generator_tool.render_c("Probe", [labels])
    borrow = rendered.index("cna_jni_borrow_string_views(")
    call = rendered.index("cna.message_box_show_ext(")
    release = rendered.index("cna_jni_free_string_views(&button_labels_views)")
    check(borrow < call < release,
          "the adapter borrows the views before the call and frees them after it")
    # A borrow that fails part way through must not leak what the earlier steps pinned. Both
    # strings before it are released on that path, in reverse acquisition order.
    refusal = rendered[borrow:call]
    check(refusal.index("message, message_bytes") < refusal.index("title, title_bytes"),
          "a failed borrow releases the strings pinned before it, newest first")

    # An OUTPUT array of views would be CNA's own memory on terms the declaration does not
    # state, which is the same thing an output structure carrying a view is refused for.
    outputs = copy.deepcopy(live)
    for parameter in outputs["functions"]["cna_message_box_show_ext"]["parameters"]:
        if parameter["type"] == "const CNA_StringView*":
            parameter["type"] = "CNA_StringView*"
    try:
        generator_tool.plan({"java": "probe", "symbol": "cna_message_box_show_ext"}, outputs)
        check(False, "a non-const CNA_StringView array is refused")
    except generator_tool.Unsupported:
        check(True, "a non-const CNA_StringView array is refused")

    # The generator refuses a shape it cannot prove rather than guessing at it.
    try:
        plan("cna_text_input_subscribe_text_input_ext")
        check(False, "a callback route is refused rather than guessed at")
    except generator_tool.Unsupported:
        check(True, "a callback route is refused rather than guessed at")


def test_probe(include: Path) -> None:
    try:
        verify_tool.compile_probe(include)
        check(True, "the layout and signature probe compiles against the live headers")
    except Exception as failure:  # noqa: BLE001 - the diagnostic is the point
        check(False, f"the layout and signature probe compiles against the live headers: {failure}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cna-root", default=os.environ.get("CNA_ROOT"))
    arguments = parser.parse_args()
    cna_root = verify_tool.resolve_cna_root(arguments.cna_root)
    include = cna_root / "modules/c-api/include"

    live = test_inventory(include)
    test_manifest(live)
    test_jni(live)
    test_java_abi()
    test_policy()
    test_coverage(cna_root)
    test_generator(live)
    test_probe(include)

    print(f"NATIVE_TOOL_TESTS={len(PASSES)} passed, {len(FAILURES)} failed")
    return 1 if FAILURES else 0


if __name__ == "__main__":
    raise SystemExit(main())
