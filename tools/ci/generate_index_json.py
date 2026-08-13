#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate package_tuya_open_index.json from a base index and release manifest.

Usage:
    python generate_index_json.py \
        --base-index global_index.json \
        --manifest manifest.json \
        --version 1.2.5 \
        --config package-config.json \
        --github-owner tuya \
        --github-repo arduino-TuyaOpen \
        --output package_tuya_open_index.json
"""

import argparse
import copy
import json
import logging
import os
import sys


HOSTS = [
    "x86_64-linux-gnu",
    "x86_64-cygwin",
    "arm64-apple-darwin",
    "x86_64-apple-darwin",
]


def make_download_url(owner, repo, tag, filename, base_url=None):
    if base_url:
        return f"{base_url.rstrip('/')}/{filename}"
    return f"https://github.com/{owner}/{repo}/releases/download/{tag}/{filename}"


def find_artifact(manifest, name_prefix):
    for artifact in manifest.get("artifacts", []):
        if artifact["name"].startswith(name_prefix):
            return artifact
    return None


def generate_vendor_tool_entry(tool_name, version, download_url, archive_filename, size, checksum):
    systems = []
    for host in HOSTS:
        systems.append({
            "host": host,
            "url": download_url,
            "archiveFileName": archive_filename,
            "size": size,
            "checksum": checksum,
        })
    return {
        "name": tool_name,
        "version": version,
        "systems": systems,
    }


def generate_index(base_index, manifest, config, version, owner, repo, base_url=None):
    package_data = copy.deepcopy(base_index)
    platforms = package_data["packages"][0]["platforms"]
    tools = package_data["packages"][0]["tools"]

    if not platforms:
        logging.error("No platform entries in base index")
        return None

    latest_platform = platforms[0]

    existing_versions = {p["version"] for p in platforms}
    if version in existing_versions:
        logging.info(f"Platform version {version} already exists, replacing")
        platforms[:] = [p for p in platforms if p["version"] != version]

    new_platform = copy.deepcopy(latest_platform)
    new_platform["version"] = version

    arduino_artifact = find_artifact(manifest, "arduino_tuya_open-")
    if arduino_artifact:
        arduino_filename = arduino_artifact["name"]
        new_platform["url"] = make_download_url(owner, repo, version, arduino_filename, base_url)
        new_platform["archiveFileName"] = arduino_filename
        new_platform["size"] = arduino_artifact["size"]
        new_platform["checksum"] = arduino_artifact["checksum"]
    else:
        arduino_filename = f"arduino_tuya_open-{version}.zip"
        new_platform["url"] = make_download_url(owner, repo, version, arduino_filename, base_url)
        new_platform["archiveFileName"] = arduino_filename
        logging.warning("Arduino artifact not in manifest, URL updated but size/checksum unchanged")

    existing_boards = {b["name"] for b in new_platform.get("boards", [])}
    for _, plat_config in config["platforms"].items():
        board_name = plat_config["boardName"]
        if board_name not in existing_boards:
            new_platform["boards"].append({"name": board_name})
            logging.info(f"Added board: {board_name}")

    tools_deps = new_platform.get("toolsDependencies", [])
    for plat_key, plat_config in config["platforms"].items():
        tool_name = plat_config["toolName"]
        new_version = plat_config["version"]
        enabled = plat_config.get("enabled", False)

        found = False
        for dep in tools_deps:
            if dep["name"].lower() == tool_name.lower():
                if enabled:
                    dep["name"] = tool_name
                    dep["version"] = new_version
                    logging.info(f"Updated toolsDependency: {tool_name} -> {new_version}")
                found = True
                break

        if not found and enabled:
            new_dep = {
                "packager": "tuya_open",
                "name": tool_name,
                "version": new_version,
            }
            insert_idx = len(tools_deps)
            for i, dep in enumerate(tools_deps):
                if dep["name"] == "env-python":
                    insert_idx = i
                    break
            tools_deps.insert(insert_idx, new_dep)
            logging.info(f"Added toolsDependency: {tool_name} {new_version}")

    new_platform["toolsDependencies"] = tools_deps
    platforms.insert(0, new_platform)

    existing_tools = {}
    for tool in tools:
        key = (tool["name"].lower(), tool["version"])
        existing_tools[key] = tool

    for plat_key, plat_config in config["platforms"].items():
        if not plat_config.get("enabled", False):
            continue

        tool_name = plat_config["toolName"]
        tool_version = plat_config["version"]
        archive_type = plat_config.get("archiveType", "tar.bz2")
        archive_filename = f"{tool_name}-{tool_version}.{archive_type}"

        tool_key = (tool_name.lower(), tool_version)

        vendor_artifact = find_artifact(manifest, f"{tool_name}-{tool_version}")
        if not vendor_artifact:
            if tool_key in existing_tools:
                logging.info(f"Tool {tool_name} {tool_version} already in index, no new artifact")
                continue
            logging.error(
                f"Enabled vendor artifact {archive_filename} not found in manifest. "
                f"Cannot generate index with missing artifacts."
            )
            return None

        size = vendor_artifact["size"]
        checksum = vendor_artifact["checksum"]
        download_url = make_download_url(owner, repo, version, archive_filename, base_url)

        if tool_key in existing_tools:
            existing = existing_tools[tool_key]
            for sys_entry in existing.get("systems", []):
                sys_entry["size"] = size
                sys_entry["checksum"] = checksum
                sys_entry["url"] = download_url
                sys_entry["archiveFileName"] = archive_filename
            logging.info(f"Updated existing tool {tool_name} {tool_version} with new artifact")
            continue

        tool_entry = generate_vendor_tool_entry(
            tool_name, tool_version, download_url, archive_filename, size, checksum
        )

        insert_idx = len(tools)
        first_same_name_idx = -1
        for i, tool in enumerate(tools):
            if tool["name"].lower() == tool_name.lower():
                first_same_name_idx = i
                break
        if first_same_name_idx >= 0:
            insert_idx = first_same_name_idx
        else:
            for i, tool in enumerate(tools):
                if tool["name"] == "env-python":
                    insert_idx = i
                    break
        tools.insert(insert_idx, tool_entry)
        logging.info(f"Added tool entry: {tool_name} {tool_version}")

    return package_data


def main():
    parser = argparse.ArgumentParser(description="Generate package index JSON")
    parser.add_argument("--base-index", required=True, help="Path to current global index JSON")
    parser.add_argument("--manifest", required=True, help="Path to release manifest JSON")
    parser.add_argument("--version", required=True, help="Release version")
    parser.add_argument("--config", required=True, help="Path to package-config.json")
    parser.add_argument("--github-owner", required=True, help="GitHub owner")
    parser.add_argument("--github-repo", required=True, help="GitHub repo name")
    parser.add_argument("--base-url", default=None, help="Override download host, e.g. http://127.0.0.1:8765 (default: GitHub release URL)")
    parser.add_argument("--output", required=True, help="Output JSON path")
    parser.add_argument("--log-level", default="INFO")
    args = parser.parse_args()

    logging.basicConfig(
        level=getattr(logging, args.log_level),
        format="[%(levelname)-8s] %(message)s",
    )

    with open(args.base_index, "r", encoding="utf-8") as f:
        base_index = json.load(f)

    with open(args.manifest, "r") as f:
        manifest = json.load(f)

    with open(args.config, "r") as f:
        config = json.load(f)

    result = generate_index(
        base_index, manifest, config, args.version,
        args.github_owner, args.github_repo, args.base_url,
    )

    if result is None:
        logging.error("Index generation failed")
        sys.exit(1)

    with open(args.output, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2, ensure_ascii=False)

    logging.info(f"Index written to {args.output}")


if __name__ == "__main__":
    main()
