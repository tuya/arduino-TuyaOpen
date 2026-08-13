#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Unified entry point for Arduino-TuyaOpen release packaging.

Usage:
    python package_release.py --version 1.2.5 --target arduino
    python package_release.py --version 1.2.5 --target t2
    python package_release.py --version 1.2.5 --target all
    python package_release.py --version 1.2.5 --target arduino --checkout-path /path/to/repo

Outputs:
    - Arduino core:  <output>/arduino_tuya_open-<version>.zip
    - Vendor SDK:    <output>/vendor-<platform>-<vendor_version>.tar.bz2
    - Manifest:      <output>/manifest.json  (size + SHA-256 for each artifact)
"""

import argparse
import json
import logging
import os
import sys

from packager.package_info import PackageInfo
from packager.package_arduino import package_arduino
from packager.package_platform_t2 import PackagePlatformT2
from packager.package_platform_t3 import PackagePlatformT3
from packager.package_platform_t5 import PackagePlatformT5
from packager.package_platform_ln882h import PackagePlatformLn882h
from packager.package_platform_esp32 import PackagePlatformEsp32

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
CONFIG_PATH = os.path.join(SCRIPT_DIR, "package-config.json")

PLATFORM_CLASSES = {
    "t2": PackagePlatformT2,
    "t3": PackagePlatformT3,
    "t5": PackagePlatformT5,
    "ln882h": PackagePlatformLn882h,
    "esp32": PackagePlatformEsp32,
}


def load_config():
    with open(CONFIG_PATH, "r") as f:
        return json.load(f)


def build_arduino(version, checkout_path, output_path):
    logging.info(f"=== Packaging Arduino core v{version} ===")
    result = package_arduino(checkout_path, version, output_path)
    if result is None:
        logging.error("Arduino core packaging failed")
        return None
    file_path, size, sha256 = result
    return {
        "name": f"arduino_tuya_open-{version}.zip",
        "path": file_path,
        "size": str(size),
        "checksum": f"SHA-256:{sha256}",
    }


def build_vendor(platform_key, platform_config, output_path, cache_path):
    logging.info(f"=== Packaging vendor SDK: {platform_key} ===")

    data_path = os.path.join(SCRIPT_DIR, "data", platform_key)
    if not os.path.exists(data_path):
        logging.error(f"Data directory not found: {data_path}")
        return None

    info = PackageInfo(
        name=platform_config["toolName"],
        version=platform_config["version"],
        chip=platform_config["chip"],
        source_repo=platform_config["sourceRepo"],
        source_branch=platform_config["sourceBranch"],
        output_path=output_path,
        compress_type=platform_config["archiveType"],
        build_app=platform_config.get("buildApp", "apps/tuya_cloud/switch_demo"),
        cache_path=cache_path,
    )

    cls = PLATFORM_CLASSES.get(platform_key)
    if cls is None:
        logging.error(f"Unknown platform: {platform_key}")
        return None

    config_path = os.path.join(SCRIPT_DIR, "config", platform_key)

    packager = cls(info, data_path, config_path)
    if not packager.package():
        logging.error(f"Vendor SDK packaging failed for {platform_key}")
        return None

    package_file = os.path.join(output_path, info.package_name)
    if not os.path.exists(package_file):
        package_file = os.path.normpath(os.path.join(
            info.output_path, info.name, "..", info.package_name
        ))

    return {
        "name": info.package_name,
        "path": package_file,
        "size": str(info.package_size),
        "checksum": f"SHA-256:{info.package_sha256}",
    }


def main():
    parser = argparse.ArgumentParser(description="Arduino-TuyaOpen release packaging")
    parser.add_argument("--version", required=True, help="Release version (e.g. 1.2.5)")
    parser.add_argument(
        "--target",
        required=True,
        help="Target to build: 'arduino', platform name (e.g. 't2'), 'enabled', or 'all'",
    )
    parser.add_argument("--output", default=None, help="Output directory (default: ./output/<version>)")
    parser.add_argument("--checkout-path", default=None, help="Path to checked-out Arduino repo (for arduino target)")
    parser.add_argument("--log-level", default="INFO", choices=["DEBUG", "INFO", "WARNING", "ERROR"])
    args = parser.parse_args()

    logging.basicConfig(
        level=getattr(logging, args.log_level),
        format="[%(levelname)-8s][%(filename)s:%(lineno)d] %(message)s",
    )

    config = load_config()
    output_path = args.output or os.path.join(SCRIPT_DIR, "output", args.version)
    os.makedirs(output_path, exist_ok=True)
    cache_path = os.path.join(SCRIPT_DIR, "output", "cache")

    manifest = {"version": args.version, "artifacts": []}
    failed = False

    targets = []
    if args.target == "arduino":
        targets = ["arduino"]
    elif args.target == "enabled":
        targets = [k for k, v in config["platforms"].items() if v.get("enabled")]
    elif args.target == "all":
        targets = ["arduino"] + list(config["platforms"].keys())
    elif args.target in config["platforms"]:
        targets = [args.target]
    else:
        logging.error(f"Unknown target: {args.target}")
        sys.exit(1)

    for target in targets:
        if target == "arduino":
            checkout = args.checkout_path or os.getcwd()
            result = build_arduino(args.version, checkout, output_path)
        else:
            platform_config = config["platforms"][target]
            result = build_vendor(target, platform_config, output_path, cache_path)

        if result:
            manifest["artifacts"].append(result)
            logging.info(f"OK: {result['name']} ({result['size']} bytes)")
        else:
            logging.error(f"FAILED: {target}")
            failed = True

    manifest_path = os.path.join(output_path, "manifest.json")
    with open(manifest_path, "w") as f:
        json.dump(manifest, f, indent=2)
    logging.info(f"Manifest written to {manifest_path}")

    if failed:
        logging.error("Some targets failed")
        sys.exit(1)

    logging.info(f"All targets completed successfully")


if __name__ == "__main__":
    main()
