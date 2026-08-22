#!/usr/bin/env python3
"""Create a standalone CNA-Java desktop starter from this maintained template."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil


SOURCE_PACKAGE = "com.openeggbert.cna.template"
SOURCE_GAME_CLASS = "HelloGame"


def java_name(value: str, label: str) -> str:
    if not re.fullmatch(r"[A-Za-z_$][A-Za-z0-9_$]*", value):
        raise argparse.ArgumentTypeError(f"{label} is not a Java identifier: {value}")
    return value


def java_package(value: str) -> str:
    parts = value.split(".")
    if not parts or any(not re.fullmatch(r"[A-Za-z_$][A-Za-z0-9_$]*", part) for part in parts):
        raise argparse.ArgumentTypeError(f"invalid Java package: {value}")
    return value


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--project-name", default="My CNA Game")
    parser.add_argument("--package", default="com.example.game", type=java_package)
    parser.add_argument("--application-id", default=None, type=java_package)
    parser.add_argument(
        "--game-class",
        default="MyGame",
        type=lambda value: java_name(value, "game class"),
    )
    parser.add_argument("--group", default="com.example", type=java_package)
    parser.add_argument("--artifact-id", default="my-cna-game")
    arguments = parser.parse_args()

    if not re.fullmatch(r"[A-Za-z0-9_.-]+", arguments.artifact_id):
        parser.error("artifact ID may contain only letters, digits, dots, underscores, and hyphens")

    source = Path(__file__).resolve().parents[1]
    output = arguments.output.resolve()
    shutil.copytree(
        source,
        output,
        ignore=shutil.ignore_patterns(
            ".git", ".gradle", "build", ".cna-repository", "__pycache__"
        ),
    )

    replacements = {
        SOURCE_PACKAGE: arguments.package,
        SOURCE_GAME_CLASS: arguments.game_class,
        "cna-java-template": arguments.artifact_id,
        "CNA-Java desktop starter": arguments.project_name,
        "org.openeggbert.examples": arguments.group,
    }
    for path in output.rglob("*"):
        if path.is_file() and path.suffix in {".java", ".gradle", ".md"}:
            text = path.read_text(encoding="utf-8")
            for old, new in replacements.items():
                text = text.replace(old, new)
            path.write_text(text, encoding="utf-8")

    old_main = output / "game/src/main/java" / Path(SOURCE_PACKAGE.replace(".", "/"))
    old_test = output / "game/src/test/java" / Path(SOURCE_PACKAGE.replace(".", "/"))
    new_main = output / "game/src/main/java" / Path(arguments.package.replace(".", "/"))
    new_test = output / "game/src/test/java" / Path(arguments.package.replace(".", "/"))
    new_main.parent.mkdir(parents=True, exist_ok=True)
    new_test.parent.mkdir(parents=True, exist_ok=True)
    shutil.move(str(old_main), str(new_main))
    shutil.move(str(old_test), str(new_test))
    (new_main / f"{SOURCE_GAME_CLASS}.java").rename(
        new_main / f"{arguments.game_class}.java"
    )

    application_id = arguments.application_id or arguments.package
    (output / "gradle.properties").write_text(
        f"applicationId={application_id}\nartifactId={arguments.artifact_id}\n",
        encoding="utf-8",
    )
    print(f"Generated {arguments.project_name} at {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
