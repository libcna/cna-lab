#!/usr/bin/env python3
"""Generate an isolated ordinary Python consumer from the maintained starter."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import shutil

ROOT = Path(__file__).resolve().parents[1]


def identifier(value: str, label: str) -> str:
    if not value.isidentifier():
        raise ValueError(f"{label} must be a Python identifier")
    return value


def distribution(value: str) -> str:
    if not re.fullmatch(r"[A-Za-z0-9]+(?:[-_.][A-Za-z0-9]+)*", value):
        raise ValueError("distribution name is invalid")
    return value


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("destination")
    parser.add_argument("--distribution-name", default="my-cna-game")
    parser.add_argument("--module-name", default="my_cna_game")
    parser.add_argument("--game-class", default="MyGame")
    args = parser.parse_args()
    destination = Path(args.destination).resolve()
    if destination.exists() and any(destination.iterdir()):
        raise ValueError("destination must not exist or must be empty")
    destination.mkdir(parents=True, exist_ok=True)
    module_name = identifier(args.module_name, "module name")
    game_class = identifier(args.game_class, "game class")
    distribution_name = distribution(args.distribution_name)
    module = destination / module_name
    module.mkdir()
    source = (ROOT / "game/hello_game.py").read_text()
    source = source.replace("class HelloGame(Game):", f"class {game_class}(Game):")
    source = source.replace('parents[1] / "Content"', 'parents[1] / "Content"')
    (module / "game.py").write_text(source)
    (module / "__init__.py").write_text(f"from .game import {game_class}\n\n__all__ = [\"{game_class}\"]\n")
    main_source = (ROOT / "main.py").read_text().replace("from game import HelloGame", f"from {module_name} import {game_class}")
    # Rename every use of the class, not one call spelling: matching a single call
    # form leaves the name behind the moment the call gains an argument, and the
    # generated consumer then fails at run time with an undefined name.
    main_source = re.sub(r"\bHelloGame\b", game_class, main_source)
    main_source = main_source.replace("cna-python-template:", f"{distribution_name}:")
    (destination / "main.py").write_text(main_source)
    # The optional `--verify-cnb` check travels with the consumer, so a generated
    # project can prove the CNA content extension reaches it from an installed
    # wheel with no source checkout on its path.
    tools = destination / "tools"
    tools.mkdir()
    (tools / "__init__.py").write_text((ROOT / "tools/__init__.py").read_text())
    verify = (ROOT / "tools/verify_cnb.py").read_text()
    verify = verify.replace("cna-python-template:", f"{distribution_name}:")
    (tools / "verify_cnb.py").write_text(verify)
    # `--verify-engine` travels too, for the same reason: a generated project
    # should be able to prove the engine extension reaches it from an installed
    # wheel with no source checkout on its path.
    verify_engine = (ROOT / "tools/verify_engine.py").read_text()
    verify_engine = verify_engine.replace("cna-python-template:", f"{distribution_name}:")
    (tools / "verify_engine.py").write_text(verify_engine)
    content = destination / "Content"
    content.mkdir()
    shutil.copyfile(ROOT / "Content/logo.png", content / "logo.png")
    shutil.copyfile(ROOT / "Content/logo.xnb", content / "logo.xnb")
    pyproject = (ROOT / "pyproject.toml").read_text()
    pyproject = pyproject.replace('name = "cna-python-starter"', f'name = "{distribution_name}"')
    pyproject = pyproject.replace('include = ["game*"]', f'include = ["{module_name}*"]')
    (destination / "pyproject.toml").write_text(pyproject)
    (destination / "README.md").write_text(
        f"# {distribution_name}\n\nGenerated desktop CNA-Python consumer.\n"
    )
    print(f"generated {destination}")


if __name__ == "__main__":
    main()
