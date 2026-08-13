# -*- coding: utf-8 -*-

import os
import hashlib


class PackageInfo:
    def __init__(self, name, version, chip, source_repo, source_branch, output_path,
                 compress_type="tar.bz2", build_app="apps/tuya_cloud/switch_demo",
                 cache_path=None):
        self.name = name
        self.version = version
        self.source_repo = source_repo
        self.source_branch = source_branch
        self.output_path = output_path
        self.compress_type = compress_type
        self.chip = chip
        self.build_app = build_app
        self.cache_path = cache_path

        self.package_name = f"{self.name}-{self.version}.{self.compress_type}"
        self.package_size = 0
        self.package_sha256 = ""

    def compute_file_info(self, file_path):
        self.package_size = os.path.getsize(file_path)
        sha256 = hashlib.sha256()
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(65536), b""):
                sha256.update(chunk)
        self.package_sha256 = sha256.hexdigest()

    def __repr__(self):
        fields = ["name", "version", "chip", "source_repo", "source_branch",
                  "output_path", "package_name", "package_size", "package_sha256"]
        lines = [f"  {f}: {getattr(self, f)}" for f in fields]
        return "PackageInfo(\n" + "\n".join(lines) + "\n)"
