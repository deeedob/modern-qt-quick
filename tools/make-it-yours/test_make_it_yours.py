import datetime
import json
import shutil
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

PROJECT = Path(__file__).resolve().parents[2]
import make_it_yours as setup


class TemplateSetupTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(dir=PROJECT / "tests")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.root_patch = patch.object(setup, "ROOT", self.root)
        self.root_patch.start()
        self.addCleanup(self.root_patch.stop)
        self.git_patch = patch.object(setup, "git", return_value=None)
        self.git_patch.start()
        self.addCleanup(self.git_patch.stop)

    def copy(self, relative):
        target = self.root / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(PROJECT / relative, target)
        return target

    def plan(self, **overrides):
        values = dict(name="River", display_name="River Studio", bundle_id="org.example.river",
                      description="A drawing app", holder='Example "Labs" \\ Research', prefix="App",
                      remove_demo=False, remove_guide=False, remove_script=False)
        values.update(overrides)
        return setup.Plan(**values)

    def test_identity_preserves_third_party_attribution(self):
        for path in ("CMakeLists.txt", "LICENSE", "REUSE.toml"):
            self.copy(path)
        original = (self.root / "REUSE.toml").read_text().splitlines()
        plan = self.plan()
        setup.set_identity(plan)
        updated = (self.root / "REUSE.toml").read_text().splitlines()
        self.assertEqual(json.loads(updated[10].split(" = ", 1)[1]),
                         f"{datetime.date.today().year} {plan.holder}")
        self.assertEqual(updated[11:], original[11:])
        self.assertIn(plan.holder, (self.root / "LICENSE").read_text())

    def test_invalid_display_name_is_rejected(self):
        answers = iter(['River', 'River "Studio"', 'River Studio', 'org.example.river',
                        'A drawing app', 'Example Person', 'App', 'n', 'n', 'n'])
        with patch("builtins.input", side_effect=lambda _: next(answers)):
            with patch("builtins.print"):
                plan = setup.collect_plan()
        self.assertEqual(plan.display_name, "River Studio")

    def test_prefix_includes_showcase(self):
        showcase = self.copy("tools/showcase/showcase.cpp")
        self.copy("AGENTS.md")
        setup.rename_prefix("Nova")
        text = showcase.read_text()
        self.assertIn('engine.loadFromModule("Nova", "Main")', text)
        self.assertNotIn("app::", text)

    def test_demo_removal_removes_canvas_links_and_test(self):
        cmake = self.copy("src/app/CMakeLists.txt")
        self.copy("vcpkg.json")
        self.copy("tests/canvas/tst_SketchTitle.qml")
        setup.remove_demo()
        self.assertNotIn("app_canvas", cmake.read_text())
        self.assertNotIn("app_sketchbook", cmake.read_text())
        self.assertFalse((self.root / "tests/canvas").exists())

    def test_demo_removal_strips_sketchbook_error_state(self):
        main = self.copy("src/app/Main.qml")
        self.copy("vcpkg.json")
        setup.remove_demo()
        text = main.read_text()
        self.assertNotIn("SketchbookViewModel", text)
        self.assertNotIn("hasFailedChanges", text)
        self.assertNotIn("storageLog", text)

    def test_customization_removes_template_about_links(self):
        for path in ("CMakeLists.txt", "LICENSE", "REUSE.toml"):
            self.copy(path)
        about = self.copy("src/app/AboutDialog.qml")
        setup.apply(self.plan())
        self.assertNotIn("github.com/deeedob/qt-kickstart", about.read_text())
        self.assertNotIn("deeedob.github.io/qt-kickstart", about.read_text())

    def test_customization_removes_template_tests_and_cmake_registration(self):
        for path in ("CMakeLists.txt", "LICENSE", "REUSE.toml", "src/app/AboutDialog.qml"):
            self.copy(path)
        test = self.copy("tools/make-it-yours/test_make_it_yours.py")
        cmake = self.copy("tools/make-it-yours/CMakeLists.txt")
        script = self.copy("tools/make-it-yours/make_it_yours.py")
        setup.apply(self.plan())
        self.assertFalse(test.exists())
        self.assertFalse(cmake.exists())
        self.assertTrue(script.exists())

    def test_customization_removes_script_and_test_registration(self):
        for path in ("CMakeLists.txt", "LICENSE", "REUSE.toml", "src/app/AboutDialog.qml"):
            self.copy(path)
        test = self.copy("tools/make-it-yours/test_make_it_yours.py")
        script = self.copy("tools/make-it-yours/make_it_yours.py")
        cmake = self.copy("tools/make-it-yours/CMakeLists.txt")
        setup.apply(self.plan(remove_script=True))
        self.assertFalse(test.exists())
        self.assertFalse(script.exists())
        self.assertFalse(cmake.exists())
        self.assertFalse(cmake.parent.exists())


if __name__ == "__main__":
    unittest.main()
