#!/usr/bin/env python3
"""Turns a fresh copy of the template into your own project.

Asks for your app's identity and what to keep, shows the plan, and applies it after you
confirm. Run it once, right after creating your repository from the template:

    python3 tools/make_it_yours.py

The demo app is its folders plus the lines marked in the shared files: a line that ends in
`// demo` or `# demo`, and every line from a `demo:begin` comment through `demo:end`.
"""

import datetime
import json
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

DEMO_FOLDERS = ["libs/shapes", "src/canvas", "src/sketchbook", "tests/canvas", "tests/sketchbook"]
DEMO_DEPENDENCIES = ["glaze"]
GUIDE_PATHS = ["docs", ".github/workflows/docs.yml"]
SOURCE_SUFFIXES = {".cpp", ".hpp", ".h", ".qml", ".js", ".cmake", ".in"}
MARKER_BEGIN = "demo:begin"
MARKER_END = "demo:end"
MARKED_LINE = re.compile(r"(//|#)\s*demo\s*$")


class Abort(Exception):
    pass


@dataclass
class Plan:
    name: str
    display_name: str
    bundle_id: str
    description: str
    holder: str
    prefix: str
    remove_demo: bool
    remove_guide: bool
    remove_script: bool


def ask(question, default="", pattern=None, hint=""):
    while True:
        suffix = f" [{default}]" if default else ""
        answer = input(f"{question}{suffix}: ").strip() or default
        if answer and (pattern is None or re.fullmatch(pattern, answer)):
            return answer
        print(f"  {hint or 'Please enter a value.'}")


def confirm(question, default=True):
    answer = ask(f"{question} (y/n)", "y" if default else "n", r"[yYnN].*", "Answer y or n.")
    return answer.lower().startswith("y")


def git(*args):
    try:
        result = subprocess.run(["git", *args], cwd=ROOT, capture_output=True, text=True)
    except FileNotFoundError:
        return None
    return result.stdout if result.returncode == 0 else None


def tracked_files():
    listed = git("ls-files")
    if listed is not None:
        return [ROOT / line for line in listed.splitlines() if (ROOT / line).is_file()]
    skip = {".git", "build", "external"}
    return [p for p in ROOT.rglob("*") if p.is_file() and not skip & set(p.relative_to(ROOT).parts)]


def source_files(files):
    """C++, QML and CMake sources: the only files that can hold demo markers."""
    return [f for f in files if f.suffix in SOURCE_SUFFIXES or f.name == "CMakeLists.txt"]


def replace_in(path, pattern, replacement):
    file = ROOT / path
    text, count = re.subn(pattern, replacement, file.read_text(encoding="utf-8"), flags=re.MULTILINE)
    if count == 0:
        raise Abort(f"{path}: found nothing matching {pattern!r}. Has the template changed?")
    file.write_text(text, encoding="utf-8")


def remove(path):
    target = ROOT / path
    if target.is_dir():
        shutil.rmtree(target)
    elif target.exists():
        target.unlink()


def strip_demo(file):
    """Removes the marked demo lines and blocks; returns how many it removed."""
    kept, block_start, removed = [], None, 0
    for number, line in enumerate(file.read_text(encoding="utf-8").splitlines(keepends=True), 1):
        if MARKER_BEGIN in line:
            if block_start:
                raise Abort(f"{file.relative_to(ROOT)}:{number}: nested {MARKER_BEGIN}")
            block_start = number
        elif MARKER_END in line:
            if not block_start:
                raise Abort(f"{file.relative_to(ROOT)}:{number}: {MARKER_END} without {MARKER_BEGIN}")
            block_start, removed = None, removed + 1
        elif not block_start and MARKED_LINE.search(line):
            removed += 1
        elif not block_start:
            kept.append(line)
    if block_start:
        raise Abort(f"{file.relative_to(ROOT)}:{block_start}: {MARKER_BEGIN} is never closed")
    if removed:
        # Removed lines can leave blank lines next to each other.
        text = re.sub(r"\n(?:[ \t]*\n){2,}", "\n\n", "".join(kept))
        file.write_text(text, encoding="utf-8")
    return removed


def set_identity(plan):
    replace_in("CMakeLists.txt", r"^project\(\w+", f"project({plan.name}")
    replace_in("CMakeLists.txt", r'^    DESCRIPTION ".*"', f'    DESCRIPTION "{plan.description}"')
    replace_in("CMakeLists.txt", r'^set\(APP_DISPLAY_NAME "[^"]*"', f'set(APP_DISPLAY_NAME "{plan.display_name}"')
    replace_in("CMakeLists.txt", r'^set\(APP_BUNDLE_ID "[^"]*"', f'set(APP_BUNDLE_ID "{plan.bundle_id}"')
    year = datetime.date.today().year
    replace_in("LICENSE", r"^Copyright .*$", f"Copyright {year} {plan.holder}")
    replace_in("REUSE.toml", r'^SPDX-FileCopyrightText = "[^"]*"',
               f'SPDX-FileCopyrightText = "{year} {plan.holder}"')


def remove_demo():
    removed = sum(strip_demo(f) for f in source_files(tracked_files()))
    if removed == 0:
        raise Abort("found no demo markers. Has the template changed?")
    for folder in DEMO_FOLDERS:
        remove(folder)
    if (ROOT / "libs").is_dir() and not any((ROOT / "libs").iterdir()):
        (ROOT / "libs").rmdir()

    manifest = ROOT / "vcpkg.json"
    text = manifest.read_text(encoding="utf-8")
    for dependency in DEMO_DEPENDENCIES:
        text = re.sub(rf',\s*"{dependency}"(?=\s*\])|"{dependency}",\s*', "", text)
    if any(d in json.loads(text)["dependencies"] for d in DEMO_DEPENDENCIES):
        raise Abort("vcpkg.json: could not remove the demo's dependencies")
    manifest.write_text(text, encoding="utf-8")
    return removed


def remove_guide(display_name):
    for path in GUIDE_PATHS:
        remove(path)
    (ROOT / "README.md").write_text(
        f"# {display_name}\n\n"
        "Built from the [Modern Qt Quick](https://github.com/deeedob/modern-qt-quick) template.\n\n"
        "```sh\n"
        "cmake --workflow --preset dev            # configure, build, run all tests\n"
        "cmake --build --preset dev --target run  # start the app\n"
        "```\n",
        encoding="utf-8")


def rename_prefix(prefix):
    namespace = prefix.lower()
    rules = [
        (r"\bApp\.", f"{prefix}."),
        (r'"App"', f'"{prefix}"'),
        (r"\bURI App\b", f"URI {prefix}"),
        (r"\bapp::", f"{namespace}::"),
        (r"\bnamespace app\b", f"namespace {namespace}"),
    ]
    folders = ("src", "tests", "libs", "cmake")
    renamed = 0
    files = [f for f in source_files(tracked_files())
             if f.relative_to(ROOT).parts[0] in folders] + [ROOT / "AGENTS.md"]
    for file in files:
        if not file.exists():
            continue
        text = file.read_text(encoding="utf-8")
        new = text
        for old, replacement in rules:
            new = re.sub(old, replacement, new)
        if new != text:
            file.write_text(new, encoding="utf-8")
            renamed += 1
    return renamed


def collect_plan():
    print("Make the template yours. Press Enter to accept a [default].\n")
    name = ask("Project name, a CMake identifier such as Acme", "Sketchbook",
               r"[A-Za-z][A-Za-z0-9_]*", "Use a letter, then letters, digits or _.")
    display_name = ask("Display name", name)
    bundle_id = ask("Bundle id (reverse DNS)", f"org.example.{name}",
                    r"[A-Za-z][A-Za-z0-9-]*(\.[A-Za-z][A-Za-z0-9-]*)+", "Use a form like com.example.App.")
    description = ask("One-line description", display_name, r'[^"\\]+', "No quotes or backslashes.")
    holder = ask("Copyright holder", (git("config", "user.name") or "").strip())
    prefix = ask("QML prefix (App); the C++ namespace becomes its lower case", "App",
                 r"[A-Z][A-Za-z0-9]*", "Start with an upper-case letter, then letters or digits.")
    return Plan(name, display_name, bundle_id, description, holder, prefix,
                remove_demo=confirm("Remove the demo app (canvas, sketchbook, libs/shapes)?"),
                remove_guide=confirm("Remove the guide (docs/) and start a new README?"),
                remove_script=confirm("Delete this script when done?"))


def print_plan(plan):
    print("\nThe plan:")
    print(f"  project {plan.name}, shown as \"{plan.display_name}\", bundle id {plan.bundle_id}")
    print(f"  copyright {datetime.date.today().year} {plan.holder} in LICENSE and REUSE.toml")
    if plan.prefix != "App":
        print(f"  rename App to {plan.prefix} in QML and app:: to {plan.prefix.lower()}:: in C++")
    if plan.remove_demo:
        print(f"  remove {', '.join(DEMO_FOLDERS)}, the marked demo lines, and {', '.join(DEMO_DEPENDENCIES)}")
    if plan.remove_guide:
        print(f"  remove {', '.join(GUIDE_PATHS)} and replace README.md")
    if plan.remove_script:
        print("  delete tools/make_it_yours.py")


def apply(plan):
    set_identity(plan)
    if plan.remove_demo:
        print(f"Removed the demo ({remove_demo()} marked lines and blocks).")
    if plan.remove_guide:
        remove_guide(plan.display_name)
        print("Removed the guide.")
    if plan.prefix != "App":
        print(f"Renamed the prefix in {rename_prefix(plan.prefix)} files.")
    if plan.remove_script:
        remove("tools/make_it_yours.py")
        if (ROOT / "tools").is_dir() and not any((ROOT / "tools").iterdir()):
            (ROOT / "tools").rmdir()


def main():
    status = git("status", "--porcelain")
    if status:
        print("The working tree has uncommitted changes. Commit or stash them first, so you can")
        print("review and undo what this script changes with git.")
        return 1
    plan = collect_plan()
    print_plan(plan)
    if not confirm("\nApply these changes?"):
        print("Nothing changed.")
        return 0
    try:
        apply(plan)
    except Abort as error:
        print(f"error: {error}", file=sys.stderr)
        print("Undo the partial changes with: git checkout -- . && git clean -fd", file=sys.stderr)
        return 1
    steps = ["cmake --workflow --preset dev --fresh",
             "cmake --build --preset dev --target update_translations"]
    if plan.prefix != "App":
        steps.append("cmake --build --preset dev --target format    # renaming changed line lengths")
    steps.append("cmake --build --preset dev --target check")
    print("\nDone. Review the changes with `git status`, then build and check:")
    print("\n".join(f"  {step}" for step in steps))
    print("The license is still MIT-0: change LICENSE and REUSE.toml if you want another one.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
