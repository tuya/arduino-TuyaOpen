import os
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from packager.package_platform import PackagePlatform


class PackagePlatformBuildEnvironmentTest(unittest.TestCase):
    def test_build_uses_python_from_sourced_vendor_export_script(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            sdk = root / "vendor-sdk"
            app = sdk / "apps" / "demo"
            app.mkdir(parents=True)
            invocations = root / "invocations.txt"

            export_script = sdk / "export.sh"
            export_script.write_text(
                'export OPEN_SDK_PYTHON="$1/managed-python"\n'
                f'export BUILD_MARKER="{invocations}"\n'
            )
            (sdk / "tos.py").write_text("# fake CLI entry point\n")

            managed_python = sdk / "managed-python"
            managed_python.write_text(
                "#!/bin/sh\n"
                'printf "%s\\n" "$*" >> "$BUILD_MARKER"\n'
            )
            managed_python.chmod(0o755)

            package_info = SimpleNamespace(
                output_path=str(root),
                name="vendor-sdk",
                build_app="apps/demo",
            )
            platform = PackagePlatform(package_info, str(root / "staging"))

            self.assertTrue(platform.build_platform())
            self.assertTrue(
                invocations.exists(),
                "build should use the Python environment initialized by export.sh",
            )

            self.assertEqual(
                invocations.read_text().splitlines(),
                [
                    f"{sdk}/tos.py clean -f",
                    f"{sdk}/tos.py build",
                ],
            )


if __name__ == "__main__":
    unittest.main()
