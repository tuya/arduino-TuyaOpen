#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Validate a generated package index against its base index and artifacts."""

import argparse
import json
import logging
import re
import sys


CHECKSUM_RE = re.compile(r"SHA-256:[0-9a-fA-F]{64}")


def _compare_shape(value, template, path, errors):
    """Require generated JSON objects to keep the base index's field schema."""
    if isinstance(template, dict):
        if not isinstance(value, dict):
            errors.append(f"{path} must be an object")
            return

        expected_keys = set(template)
        actual_keys = set(value)
        for key in sorted(expected_keys - actual_keys):
            errors.append(f"{path} is missing field '{key}'")
        for key in sorted(actual_keys - expected_keys):
            errors.append(f"{path} has unexpected field '{key}'")
        for key in sorted(expected_keys & actual_keys):
            _compare_shape(value[key], template[key], f"{path}.{key}", errors)
        return

    if isinstance(template, list):
        if not isinstance(value, list):
            errors.append(f"{path} must be an array")
            return
        if value and not template:
            errors.append(f"{path} has entries but no base entry to validate their fields")
            return
        if template:
            for index, item in enumerate(value):
                _compare_shape(item, template[0], f"{path}[{index}]", errors)
        return

    if type(value) is not type(template):
        errors.append(f"{path} must have the same JSON type as the base index")


def _artifact_map(manifest, errors):
    artifacts = manifest.get("artifacts") if isinstance(manifest, dict) else None
    if not isinstance(artifacts, list):
        errors.append("Manifest is missing the 'artifacts' array")
        return {}

    result = {}
    for index, artifact in enumerate(artifacts):
        if not isinstance(artifact, dict) or not artifact.get("name"):
            errors.append(f"Manifest artifact at index {index} has no name")
            continue
        name = artifact["name"]
        if name in result:
            errors.append(f"Manifest contains duplicate artifact '{name}'")
        result[name] = artifact
    return result


def _check_artifact_metadata(artifact, name, errors):
    if not isinstance(artifact, dict):
        errors.append(f"Manifest artifact '{name}' is malformed")
        return
    size = artifact.get("size")
    if not isinstance(size, str) or not size.isdigit() or int(size) <= 0:
        errors.append(f"Manifest artifact '{name}' has invalid size: {size!r}")
    checksum = artifact.get("checksum")
    if not isinstance(checksum, str) or not CHECKSUM_RE.fullmatch(checksum):
        errors.append(f"Manifest artifact '{name}' has invalid SHA-256 checksum")


def _release_url(owner, repo, version, filename):
    return f"https://github.com/{owner}/{repo}/releases/download/{version}/{filename}"


def _index_map(rows, key_fn, label, errors):
    result = {}
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            errors.append(f"{label.capitalize()} entry {index} must be an object")
            continue
        key = key_fn(row)
        if key in result:
            errors.append(f"Index contains duplicate {label} '{key}'")
        result[key] = row
    return result


def validate_index_data(index, version, manifest, base_index, config, owner, repo):
    """Return validation errors and warnings for a generated package index."""
    errors = []
    warnings = []

    if not isinstance(index, dict) or not isinstance(base_index, dict):
        return ["Index and base index must be JSON objects"], warnings

    _compare_shape(index, base_index, "index", errors)

    for key in (set(base_index) & set(index)) - {"packages"}:
        if index[key] != base_index[key]:
            errors.append(f"Top-level field '{key}' was unexpectedly modified")

    packages = index.get("packages")
    base_packages = base_index.get("packages")
    if not isinstance(packages, list) or not packages:
        errors.append("Missing 'packages' array")
        return errors, warnings
    if not isinstance(base_packages, list) or len(packages) != len(base_packages):
        errors.append("Generated index must preserve the base package entries")
        return errors, warnings

    for package_index, (current_package, base_package) in enumerate(zip(packages, base_packages)):
        if not isinstance(current_package, dict) or not isinstance(base_package, dict):
            errors.append(f"Package entry {package_index} must be an object")
            continue
        if current_package.get("name") != base_package.get("name"):
            errors.append(f"Package entry {package_index} identity was unexpectedly modified")
        if package_index > 0 and current_package != base_package:
            errors.append(f"Unrelated package entry {package_index} was unexpectedly modified")
        for key in (set(base_package) & set(current_package)) - {"platforms", "tools"}:
            if current_package[key] != base_package[key]:
                errors.append(f"Package field '{key}' was unexpectedly modified")

    package = packages[0]
    base_package = base_packages[0]
    if not isinstance(package, dict) or not isinstance(base_package, dict):
        errors.append("Package entry 0 must be an object")
        return errors, warnings
    platforms = package.get("platforms", [])
    base_platforms = base_package.get("platforms", [])
    tools = package.get("tools", [])
    base_tools = base_package.get("tools", [])
    if not isinstance(platforms, list) or not isinstance(base_platforms, list) or not platforms or not base_platforms:
        errors.append("Index and base index must contain platform entries")
        return errors, warnings

    platform_versions = [item.get("version") for item in platforms if isinstance(item, dict)]
    base_versions = [item.get("version") for item in base_platforms if isinstance(item, dict)]
    expected_versions = [version] + [item for item in base_versions if item != version]
    if platform_versions != expected_versions:
        errors.append("Generated index must add the requested version first and retain all other platform versions")
    if len(platform_versions) != len(set(platform_versions)):
        errors.append("Platform versions must be unique")

    target_platform = platforms[0]
    if not isinstance(target_platform, dict) or target_platform.get("version") != version:
        errors.append(f"Expected platform version {version} at platforms[0]")
        return errors, warnings

    base_platform_by_version = {item.get("version"): item for item in base_platforms if isinstance(item, dict)}
    for platform in platforms[1:]:
        if isinstance(platform, dict) and platform.get("version") in base_platform_by_version:
            if platform != base_platform_by_version[platform["version"]]:
                errors.append(f"Existing platform version {platform['version']} was unexpectedly modified")

    expected_boards = [dict(board) for board in base_platforms[0].get("boards", [])]
    expected_board_names = [board.get("name") for board in expected_boards]
    for platform_config in config.get("platforms", {}).values():
        board_name = platform_config.get("boardName")
        if board_name and board_name not in expected_board_names:
            expected_board_names.append(board_name)
            expected_boards.append({"name": board_name})
    board_names = [board.get("name") for board in target_platform.get("boards", []) if isinstance(board, dict)]
    if board_names != expected_board_names:
        errors.append("New platform must retain the base boards and add only configured boards")

    expected_dependencies = [dict(dep) for dep in base_platforms[0].get("toolsDependencies", [])]
    enabled_tools = []
    for platform_config in config.get("platforms", {}).values():
        if not platform_config.get("enabled", False):
            continue
        tool_name = platform_config["toolName"]
        tool_version = platform_config["version"]
        enabled_tools.append((tool_name, tool_version, platform_config))
        matching = [dep for dep in expected_dependencies if dep.get("name", "").lower() == tool_name.lower()]
        if matching:
            matching[0]["name"] = tool_name
            matching[0]["version"] = tool_version
        else:
            dependency = {"packager": "tuya_open", "name": tool_name, "version": tool_version}
            insert_at = next(
                (i for i, dep in enumerate(expected_dependencies) if dep.get("name") == "env-python"),
                len(expected_dependencies),
            )
            expected_dependencies.insert(insert_at, dependency)
    if target_platform.get("toolsDependencies") != expected_dependencies:
        errors.append("New platform toolsDependencies do not match the base index and enabled package config")

    tools = tools if isinstance(tools, list) else []
    base_tools = base_tools if isinstance(base_tools, list) else []
    tool_key = lambda item: (item.get("name", "").lower(), item.get("version"))
    tools_by_key = _index_map(tools, tool_key, "tool", errors)
    base_tools_by_key = _index_map(base_tools, tool_key, "base tool", errors)
    expected_tool_keys = set(base_tools_by_key)
    expected_tool_keys.update((name.lower(), tool_version) for name, tool_version, _ in enabled_tools)
    if set(tools_by_key) != expected_tool_keys:
        errors.append("Generated index must retain existing tools and include each enabled tool version")

    target_dependencies = target_platform.get("toolsDependencies", [])
    if isinstance(target_dependencies, list):
        for dependency in target_dependencies:
            if not isinstance(dependency, dict):
                continue
            dependency_key = (dependency.get("name", "").lower(), dependency.get("version"))
            if dependency_key not in tools_by_key:
                errors.append(
                    f"toolsDependency '{dependency.get('name')}' version '{dependency.get('version')}' "
                    "is missing from the tools array"
                )

    manifest_by_name = _artifact_map(manifest, errors)
    expected_core_name = f"arduino_tuya_open-{version}.zip"
    required_artifacts = [expected_core_name]
    for tool_name, tool_version, platform_config in enabled_tools:
        archive_type = platform_config.get("archiveType", "tar.bz2")
        required_artifacts.append(f"{tool_name}-{tool_version}.{archive_type}")
    for artifact_name in required_artifacts:
        artifact = manifest_by_name.get(artifact_name)
        if artifact is None:
            errors.append(f"Required release artifact '{artifact_name}' is missing from manifest")
        else:
            _check_artifact_metadata(artifact, artifact_name, errors)

    core_artifact = manifest_by_name.get(expected_core_name)
    expected_core_url = _release_url(owner, repo, version, expected_core_name)
    if core_artifact:
        for field in ("archiveFileName", "size", "checksum"):
            index_field = "archiveFileName" if field == "archiveFileName" else field
            expected_value = expected_core_name if field == "archiveFileName" else core_artifact.get(field)
            if target_platform.get(index_field) != expected_value:
                errors.append(f"Arduino platform {index_field} does not match release manifest")
        if target_platform.get("url") != expected_core_url:
            errors.append("Arduino platform URL does not match the versioned release asset")

        expected_platform = dict(base_platforms[0])
        expected_platform.update({
            "version": version,
            "url": expected_core_url,
            "archiveFileName": expected_core_name,
            "size": core_artifact.get("size"),
            "checksum": core_artifact.get("checksum"),
            "boards": expected_boards,
            "toolsDependencies": expected_dependencies,
        })
        if target_platform != expected_platform:
            errors.append("New platform metadata differs from the base platform and release manifest")

    for tool_name, tool_version, platform_config in enabled_tools:
        archive_type = platform_config.get("archiveType", "tar.bz2")
        artifact_name = f"{tool_name}-{tool_version}.{archive_type}"
        artifact = manifest_by_name.get(artifact_name)
        key = (tool_name.lower(), tool_version)
        tool = tools_by_key.get(key)
        dependencies = [
            dep for dep in target_platform.get("toolsDependencies", [])
            if dep.get("name", "").lower() == tool_name.lower()
        ]
        if len(dependencies) != 1 or dependencies[0].get("version") != tool_version:
            errors.append(f"Platform dependency for {tool_name} must reference version {tool_version}")
        if tool is None:
            errors.append(f"Enabled tool {tool_name} {tool_version} is missing from the tools array")
            continue
        if tool.get("name") != tool_name:
            errors.append(f"Tool name for {tool_name} {tool_version} does not match package config")
        if artifact is None:
            continue

        template_tool = next(
            (entry for entry in base_tools if entry.get("name", "").lower() == tool_name.lower()),
            base_tools[0] if base_tools else {},
        )
        expected_hosts = [system.get("host") for system in template_tool.get("systems", [])]
        systems = tool.get("systems", [])
        actual_hosts = [system.get("host") for system in systems if isinstance(system, dict)]
        if actual_hosts != expected_hosts:
            errors.append(f"Tool {tool_name} {tool_version} must retain the base host list")
        expected_url = _release_url(owner, repo, version, artifact_name)
        for system in systems:
            if not isinstance(system, dict):
                errors.append(f"Tool {tool_name} {tool_version} has a malformed system entry")
                continue
            expected_values = {
                "archiveFileName": artifact_name,
                "size": artifact.get("size"),
                "checksum": artifact.get("checksum"),
                "url": expected_url,
            }
            for field, expected_value in expected_values.items():
                if system.get(field) != expected_value:
                    errors.append(f"Tool {tool_name} {tool_version} system {field} does not match release manifest")

    enabled_keys = {(name.lower(), tool_version) for name, tool_version, _ in enabled_tools}
    for key, base_tool in base_tools_by_key.items():
        if key in enabled_keys:
            continue
        if tools_by_key.get(key) != base_tool:
            errors.append(f"Existing tool {base_tool.get('name')} {base_tool.get('version')} was unexpectedly modified")

    for tool in tools:
        if not isinstance(tool, dict):
            continue
        for system in tool.get("systems", []):
            if isinstance(system, dict):
                if system.get("size") == "0" or system.get("checksum") == "SHA-256:0":
                    errors.append(
                        f"Tool {tool.get('name')} {tool.get('version')} has placeholder size/checksum"
                    )
                tool_url = system.get("url", "")
                if tool_url and not tool_url.startswith("https://github.com/"):
                    warnings.append(f"Tool {tool.get('name')} {tool.get('version')} URL not on GitHub: {tool_url}")

    return errors, warnings


def validate_index(index_path, version, manifest_path, base_index_path, config_path):
    try:
        with open(index_path, "r", encoding="utf-8") as f:
            index = json.load(f)
        with open(manifest_path, "r", encoding="utf-8") as f:
            manifest = json.load(f)
        with open(base_index_path, "r", encoding="utf-8") as f:
            base_index = json.load(f)
        with open(config_path, "r", encoding="utf-8") as f:
            config = json.load(f)
    except (OSError, json.JSONDecodeError) as error:
        return [f"Could not load index validation input: {error}"], []

    package_config = config.get("package", {})
    return validate_index_data(
        index=index,
        version=version,
        manifest=manifest,
        base_index=base_index,
        config=config,
        owner=package_config.get("githubOwner", ""),
        repo=package_config.get("githubRepo", ""),
    )


def main():
    parser = argparse.ArgumentParser(description="Validate a generated package index")
    parser.add_argument("--index", required=True, help="Path to generated package index JSON")
    parser.add_argument("--version", required=True, help="Expected new platform version")
    parser.add_argument("--manifest", required=True, help="Release artifact manifest JSON")
    parser.add_argument("--base-index", required=True, help="Base global index JSON used for generation")
    parser.add_argument("--config", required=True, help="Package configuration JSON")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format="[%(levelname)-8s] %(message)s")
    errors, warnings = validate_index(
        args.index, args.version, args.manifest, args.base_index, args.config
    )

    for warning in warnings:
        logging.warning(warning)
    for error in errors:
        logging.error(error)

    if errors:
        logging.error(f"Validation FAILED with {len(errors)} error(s)")
        sys.exit(1)
    logging.info(f"Validation PASSED ({len(warnings)} warning(s))")


if __name__ == "__main__":
    main()
