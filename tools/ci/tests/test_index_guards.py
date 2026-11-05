import json
import re
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from generate_index_json import generate_index
from validate_index import validate_index_data


def base_index():
    checksum = "SHA-256:" + "a" * 64
    systems = [
        {
            "host": host,
            "url": "https://github.com/tuya/arduino-TuyaOpen/releases/download/1.0.0/vendor-esp32-0.0.6.tar.bz2",
            "archiveFileName": "vendor-esp32-0.0.6.tar.bz2",
            "size": "200",
            "checksum": checksum,
        }
        for host in (
            "x86_64-linux-gnu",
            "x86_64-cygwin",
            "arm64-apple-darwin",
            "x86_64-apple-darwin",
        )
    ]
    return {
        "packages": [
            {
                "name": "tuya_open",
                "maintainer": "tuya",
                "websiteURL": "https://www.tuya.com/",
                "email": "",
                "help": {"online": ""},
                "platforms": [
                    {
                        "name": "Tuya Open",
                        "architecture": "tuya_open",
                        "version": "1.0.0",
                        "category": "Contributed",
                        "help": {"online": ""},
                        "url": "https://github.com/tuya/arduino-TuyaOpen/releases/download/1.0.0/arduino_tuya_open-1.0.0.zip",
                        "archiveFileName": "arduino_tuya_open-1.0.0.zip",
                        "size": "100",
                        "checksum": checksum,
                        "boards": [{"name": "esp32"}],
                        "toolsDependencies": [
                            {"packager": "tuya_open", "name": "vendor-esp32", "version": "0.0.6"}
                        ],
                    }
                ],
                "tools": [{"name": "vendor-esp32", "version": "0.0.6", "systems": systems}],
            }
        ]
    }


def esp32_config():
    return {
        "platforms": {
            "esp32": {
                "enabled": True,
                "toolName": "vendor-esp32",
                "boardName": "esp32",
                "version": "0.0.7",
                "archiveType": "tar.bz2",
            }
        }
    }


def manifest():
    return {
        "artifacts": [
            {
                "name": "arduino_tuya_open-1.0.1.zip",
                "size": "110",
                "checksum": "SHA-256:" + "b" * 64,
            },
            {
                "name": "vendor-esp32-0.0.7.tar.bz2",
                "size": "300",
                "checksum": "SHA-256:" + "c" * 64,
            },
        ]
    }


class GenerateIndexGuardsTest(unittest.TestCase):
    def test_missing_arduino_core_artifact_fails_generation(self):
        manifest = {
            "artifacts": [
                {
                    "name": "vendor-esp32-0.0.7.tar.bz2",
                    "size": "300",
                    "checksum": "SHA-256:" + "b" * 64,
                }
            ]
        }

        with self.assertLogs(level="ERROR"):
            result = generate_index(base_index(), manifest, esp32_config(), "1.0.1", "tuya", "arduino-TuyaOpen")

        self.assertIsNone(result)


class ValidateIndexGuardsTest(unittest.TestCase):
    def generated_index(self):
        result = generate_index(base_index(), manifest(), esp32_config(), "1.0.1", "tuya", "arduino-TuyaOpen")
        self.assertIsNotNone(result)
        return result

    def validate(self, index, artifacts=None):
        errors, _ = validate_index_data(
            index=index,
            version="1.0.1",
            manifest=artifacts or manifest(),
            base_index=base_index(),
            config=esp32_config(),
            owner="tuya",
            repo="arduino-TuyaOpen",
        )
        return errors

    def test_generated_index_matches_base_schema_and_manifest(self):
        errors = self.validate(self.generated_index())

        self.assertEqual(errors, [])

    def test_removed_platform_field_is_rejected(self):
        index = self.generated_index()
        del index["packages"][0]["platforms"][0]["category"]

        errors = self.validate(index)

        self.assertTrue(any("category" in error for error in errors), errors)

    def test_added_platform_field_is_rejected(self):
        index = self.generated_index()
        index["packages"][0]["platforms"][0]["unexpected"] = "value"

        errors = self.validate(index)

        self.assertTrue(any("unexpected field 'unexpected'" in error for error in errors), errors)

    def test_existing_package_metadata_change_is_rejected(self):
        index = self.generated_index()
        index["packages"][0]["maintainer"] = "unexpected"

        errors = self.validate(index)

        self.assertTrue(any("maintainer" in error for error in errors), errors)

    def test_manifest_metadata_mismatch_is_rejected(self):
        index = self.generated_index()
        index["packages"][0]["platforms"][0]["checksum"] = "SHA-256:" + "d" * 64

        errors = self.validate(index)

        self.assertTrue(any("checksum" in error for error in errors), errors)

    def test_missing_enabled_vendor_artifact_is_rejected(self):
        index = self.generated_index()
        artifacts = manifest()
        artifacts["artifacts"] = artifacts["artifacts"][:1]

        errors = self.validate(index, artifacts)

        self.assertTrue(any("vendor-esp32-0.0.7.tar.bz2" in error for error in errors), errors)

    def test_vendor_system_checksum_must_match_manifest(self):
        index = self.generated_index()
        vendor = next(
            tool for tool in index["packages"][0]["tools"] if tool["version"] == "0.0.7"
        )
        vendor["systems"][0]["checksum"] = "SHA-256:" + "d" * 64

        errors = self.validate(index)

        self.assertTrue(any("system checksum does not match" in error for error in errors), errors)


class ReleaseWorkflowSafetyTest(unittest.TestCase):
    def test_release_workflow_fails_if_global_index_cannot_be_downloaded(self):
        workflow = (
            Path(__file__).resolve().parents[3] / ".github/workflows/release-ci-cd.yml"
        ).read_text()
        download_step = re.search(
            r"      - name: Download current global index\n(?P<body>.*?)(?=      - name: )",
            workflow,
            re.DOTALL,
        ).group("body")

        self.assertNotIn("|| {", download_step)
        self.assertNotIn("cp package.json global_index.json", download_step)

    def test_release_workflow_serializes_global_index_updates(self):
        workflow = (
            Path(__file__).resolve().parents[3] / ".github/workflows/release-ci-cd.yml"
        ).read_text()

        self.assertRegex(workflow, r"(?m)^concurrency:\s*$")
        self.assertIn("group: release-index-${{ github.repository }}", workflow)
        self.assertIn("queue: max", workflow)

    def test_t5_is_disabled_for_this_release(self):
        config = json.loads(
            (Path(__file__).resolve().parents[1] / "package-config.json").read_text()
        )

        self.assertFalse(config["platforms"]["t5"]["enabled"])


if __name__ == "__main__":
    unittest.main()
