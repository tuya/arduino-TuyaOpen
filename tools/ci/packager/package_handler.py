# -*- coding: utf-8 -*-

import os
import re
import sys
import shutil
import logging
import subprocess
import tarfile
import zipfile
import hashlib


class PackageHandler:
    @staticmethod
    def _is_commit_sha(ref):
        """A 7-40 char hex string is treated as a commit, not a branch name."""
        return re.fullmatch(r"[0-9a-fA-F]{7,40}", ref) is not None

    def git_clone(self, git_url, git_branch, clone_path, force_update=True):
        pinned = self._is_commit_sha(git_branch)

        if os.path.exists(clone_path):
            if force_update:
                logging.info(f"{clone_path} exists, updating to latest...")
                try:
                    subprocess.run(
                        ["git", "-C", clone_path, "fetch", "origin"],
                        check=True, capture_output=True, text=True
                    )
                    # --force: a previous build leaves tracked files modified, and a
                    # plain checkout aborts on those, falling through to the fresh
                    # clone below - which deletes gitignored downloads worth GBs
                    # (platform/ carries the ~700M toolchain).
                    checkout_args = ["--force"] + (["--detach"] if pinned else []) + [git_branch]
                    subprocess.run(
                        ["git", "-C", clone_path, "checkout"] + checkout_args,
                        check=True, capture_output=True, text=True
                    )
                    # origin/<sha> is not a ref, so a pinned commit resets onto itself
                    subprocess.run(
                        ["git", "-C", clone_path, "reset", "--hard",
                         git_branch if pinned else f"origin/{git_branch}"],
                        check=True, capture_output=True, text=True
                    )
                    # Drop untracked leftovers such as the src/ai_components headers an
                    # older packager forged. No -x, so gitignored downloads (platform/,
                    # .build/) are preserved.
                    subprocess.run(
                        ["git", "-C", clone_path, "clean", "-fd"],
                        check=True, capture_output=True, text=True
                    )
                    logging.info(f"Updated {clone_path} to {'commit' if pinned else 'latest'} {git_branch}")

                    submodule_path = os.path.join(clone_path, ".gitmodules")
                    if os.path.exists(submodule_path):
                        subprocess.run(
                            ["git", "-C", clone_path, "submodule", "update", "--init", "--recursive", "--force"],
                            check=True, capture_output=True, text=True
                        )
                        logging.info("Updated submodules")
                    return True
                except subprocess.CalledProcessError as e:
                    logging.error(f"Update failed: {e.stderr}, will try fresh clone")
                    shutil.rmtree(clone_path)
            else:
                logging.warning(f"{clone_path} exists, skipping update")
                return True

        try:
            logging.info(f"Cloning {git_url} {'commit' if pinned else 'branch'} {git_branch} to {clone_path}")
            if pinned:
                # git clone --branch does not accept a raw sha
                subprocess.run(
                    ["git", "clone", git_url, clone_path],
                    check=True, capture_output=True, text=True
                )
                subprocess.run(
                    ["git", "-C", clone_path, "checkout", "--detach", git_branch],
                    check=True, capture_output=True, text=True
                )
            else:
                subprocess.run(
                    ["git", "clone", "--branch", git_branch, git_url, clone_path],
                    check=True, capture_output=True, text=True
                )
        except subprocess.CalledProcessError as e:
            logging.error(f"Clone failed: {e.stderr}")
            return False

        logging.info(f"Clone {git_url} success")

        submodule_path = os.path.join(clone_path, ".gitmodules")
        if os.path.exists(submodule_path):
            try:
                subprocess.run(
                    ["git", "-C", clone_path, "submodule", "update", "--init", "--recursive"],
                    check=True, capture_output=True, text=True
                )
                logging.info("Clone submodule success")
            except subprocess.CalledProcessError as e:
                logging.error(f"Submodule init failed: {e.stderr}")
                return False

        return True

    def remove_unused_files(self, remove_path, remove_list):
        for remove_file in remove_list:
            remove_file = os.path.normpath(remove_file)
            remove_file_path = os.path.join(remove_path, remove_file)
            if os.path.isfile(remove_file_path):
                os.remove(remove_file_path)
                logging.debug(f"Remove file {remove_file_path}")
            elif os.path.isdir(remove_file_path):
                shutil.rmtree(remove_file_path)
                logging.debug(f"Remove directory {remove_file_path}")
            else:
                logging.debug(f"{remove_file_path} not exists")
        logging.info("Remove unused files success")

    def calculate_sha256(self, file_path):
        sha256 = hashlib.sha256()
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(65536), b""):
                sha256.update(chunk)
        return sha256.hexdigest()

    def compress_package(self, input_path, output_path, compress_type="zip"):
        logging.info(f"Compressing {input_path} to {output_path}")
        if compress_type == "zip":
            self._zip_folder(input_path, output_path)
        elif compress_type == "tar.bz2":
            self._tarbz2_folder(input_path, output_path)

    def _zip_folder(self, folder_path, output_path):
        with zipfile.ZipFile(output_path, "w", zipfile.ZIP_DEFLATED) as zipf:
            for foldername, _, filenames in os.walk(folder_path):
                for filename in filenames:
                    file_path = os.path.join(foldername, filename)
                    arcname = os.path.relpath(file_path, folder_path)
                    zipf.write(file_path, arcname)

    def _tarbz2_folder(self, folder_path, output_path):
        with tarfile.open(output_path, "w:bz2") as tar:
            tar.add(folder_path, arcname=os.path.basename(folder_path))

    def add_prefix_to_list(self, file_path, prefix=None, delimiter=None):
        file_list = []
        with open(file_path, "r") as f:
            for line in f:
                line = line.strip()
                if line:
                    items = [line] if delimiter is None else line.split(delimiter)
                    for item in items:
                        if prefix is not None:
                            file_list.append(os.path.join(prefix, item))
                        else:
                            file_list.append(item)
        return file_list
