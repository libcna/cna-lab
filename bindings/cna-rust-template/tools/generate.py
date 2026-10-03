#!/usr/bin/env python3
"""Generate a standalone CNA-Rust consumer without sibling or absolute paths."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import shutil


ROOT = Path(__file__).resolve().parents[1]
NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9_-]*$")


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, help="parent directory for the generated project")
    parser.add_argument("--project-name", required=True, help="generated directory name")
    parser.add_argument("--crate-name", help="Cargo package name; defaults to project name")
    parser.add_argument("--binary-name", help="installed binary name; defaults to crate name")
    parser.add_argument("--version", default="0.1.0")
    parser.add_argument("--description", default="A native CNA-Rust 2D game")
    parser.add_argument("--license", default="MIT")
    parser.add_argument("--author", action="append", default=[])
    parser.add_argument("--repository")
    parser.add_argument("--cna-root", required=True, help="CNA-Rust checkout to vendor")
    return parser.parse_args()


def checked_name(label: str, value: str) -> str:
    if not NAME.fullmatch(value):
        raise ValueError(f"{label} must contain only letters, digits, '-' and '_'")
    return value


def toml_string(value: str) -> str:
    if "\n" in value or "\r" in value:
        raise ValueError("package metadata must be single-line")
    return json.dumps(value)


def cargo_manifest(args: argparse.Namespace, crate_name: str, binary_name: str) -> str:
    metadata = [
        "[package]",
        f"name = {toml_string(crate_name)}",
        f"version = {toml_string(args.version)}",
        'edition = "2021"',
        'rust-version = "1.74"',
        f"license = {toml_string(args.license)}",
        f"description = {toml_string(args.description)}",
    ]
    if args.author:
        metadata.append("authors = " + json.dumps(args.author))
    if args.repository:
        metadata.append(f"repository = {toml_string(args.repository)}")
    metadata.extend(
        [
            'readme = "README.md"',
            "",
            "[[bin]]",
            f"name = {toml_string(binary_name)}",
            'path = "src/main.rs"',
            "",
            "[dependencies]",
            'cna = { package = "cna-rust", path = "vendor/cna", version = "0.0.0" }',
            "",
            "[workspace]",
            'resolver = "2"',
            'members = ["vendor/cna", "vendor/cna-sys"]',
            "",
            "[workspace.package]",
            'version = "0.0.0"',
            'edition = "2021"',
            'rust-version = "1.74"',
            'license = "Ms-PL"',
            'repository = "https://github.com/openeggbert/cna-rust"',
            "",
            "[workspace.lints.rust]",
            'unsafe_op_in_unsafe_fn = "deny"',
            "",
            "[workspace.lints.clippy]",
            'all = "warn"',
            'pedantic = "warn"',
            "",
            "[lints]",
            "workspace = true",
            "",
        ]
    )
    return "\n".join(metadata)


def build_script_name(manifest: Path) -> str | None:
    """The build script a manifest names, if it names one.

    Read from the manifest rather than assumed to be `build.rs`, because Cargo
    lets a package point `build` anywhere and a vendored copy that missed it
    would fail to compile with a message about a file the reader never wrote.
    """
    for line in manifest.read_text(encoding="utf-8").splitlines():
        stripped = line.strip()
        if stripped.startswith("build ") or stripped.startswith("build="):
            _, _, value = stripped.partition("=")
            return value.strip().strip('"')
    return None


def copy_crate(cna_root: Path, name: str, destination: Path) -> None:
    source = cna_root / "crates" / name
    if not (source / "Cargo.toml").is_file() or not (source / "src").is_dir():
        raise ValueError(f"invalid CNA-Rust crate: {source}")
    destination.mkdir(parents=True)
    shutil.copy2(source / "Cargo.toml", destination / "Cargo.toml")
    shutil.copytree(source / "src", destination / "src")
    if (source / "tests").is_dir():
        shutil.copytree(source / "tests", destination / "tests")
    # A crate's manifest may name a build script, and a vendored copy without
    # it does not build at all. Taking it from the manifest rather than
    # guessing means a build script added later travels automatically -- the
    # omission this check exists for was real: `cna-sys` grew one for
    # direct-link support and the generated project stopped compiling.
    build_script = build_script_name(source / "Cargo.toml")
    if build_script is not None:
        if not (source / build_script).is_file():
            raise ValueError(f"{name} names a build script it does not have: {build_script}")
        shutil.copy2(source / build_script, destination / build_script)
    # A vendored crate carries its own licence and notice, exactly as the
    # published crate does.
    for notice in ("LICENSE", "NOTICE.md"):
        if (source / notice).is_file():
            shutil.copy2(source / notice, destination / notice)


def main() -> None:
    args = arguments()
    project_name = checked_name("project name", args.project_name)
    crate_name = checked_name("crate name", args.crate_name or project_name)
    binary_name = checked_name("binary name", args.binary_name or crate_name)
    cna_root = Path(args.cna_root).resolve()
    destination = Path(args.output).resolve() / project_name
    if destination.exists():
        raise ValueError(f"refusing to overwrite existing path: {destination}")

    destination.mkdir(parents=True)
    shutil.copytree(ROOT / "src", destination / "src")
    for source in (destination / "src").rglob("*.rs"):
        source.write_text(
            source.read_text(encoding="utf-8").replace("cna-rust-template", project_name),
            encoding="utf-8",
        )
    shutil.copytree(ROOT / "Content", destination / "Content")
    shutil.copy2(ROOT / "LICENSE", destination / "LICENSE")
    shutil.copy2(ROOT / ".gitignore", destination / ".gitignore")
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    readme = readme.replace(
        'cna = { package = "cna-rust", path = "../cna-rust/crates/cna" }',
        'cna = { package = "cna-rust", path = "vendor/cna" }',
    )
    readme = re.sub(
        r"## Generate a standalone project\n.*?(?=## Requirements)",
        "## Binding source\n\n"
        "This generated project vendors the supplied CNA-Rust checkout under `vendor/`; "
        "it has no sibling-repository dependency.\n\n",
        readme,
        flags=re.S,
    )
    (destination / "README.md").write_text(
        readme.replace("# CNA-Rust template", f"# {project_name}"), encoding="utf-8"
    )
    (destination / "Cargo.toml").write_text(
        cargo_manifest(args, crate_name, binary_name), encoding="utf-8"
    )

    vendor = destination / "vendor"
    copy_crate(cna_root, "cna", vendor / "cna")
    copy_crate(cna_root, "cna-sys", vendor / "cna-sys")
    shutil.copy2(cna_root / "LICENSE", vendor / "CNA-RUST-LICENSE")
    print(destination)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        raise SystemExit(f"generate: {error}") from error
