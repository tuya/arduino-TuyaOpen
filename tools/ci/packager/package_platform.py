# -*- coding: utf-8 -*-

import os
import re
import sys
import shutil
import logging
import subprocess

from .package_handler import PackageHandler


class PackagePlatform:
    """Base class for platform-specific vendor SDK packaging."""

    def __init__(self, package_info, data_path, config_path=None):
        self.package_info = package_info
        # Clone into the version-independent cache so gitignored toolchain
        # downloads survive across release versions; fall back to the old
        # per-version layout when no cache path is provided.
        clone_root = package_info.cache_path or package_info.output_path
        self.clone_path = os.path.join(clone_root, self.package_info.name)
        self.data_path = data_path
        self.config_path = config_path or data_path
        self.staging_path = data_path
        self.build_app_path = os.path.join(self.clone_path, self.package_info.build_app)
        self.clone_python = "python"
        self.handler = PackageHandler()

    def git_clone(self):
        logging.debug(f"Clone path: {self.clone_path}")
        if not self.handler.git_clone(
            self.package_info.source_repo,
            self.package_info.source_branch,
            self.clone_path,
            force_update=True,
        ):
            return False
        logging.info(f"Clone {self.package_info.source_repo} success")
        return True

    def init_submodules(self):
        try:
            subprocess.run(
                ["git", "-C", self.clone_path, "submodule", "update", "--init", "--recursive"],
                check=True,
                capture_output=True,
                text=True,
            )
            logging.info("Submodule update completed")
            return True
        except subprocess.CalledProcessError as e:
            logging.error(f"Submodule update failed: {e.stderr}")
            return False

    def set_platform_ini(self, ini_file):
        if not os.path.exists(ini_file):
            logging.error(f"INI file not found: {ini_file}")
            return False

        app_ini_file = os.path.join(
            self.build_app_path, "app_default.config"
        )
        if os.path.exists(app_ini_file):
            os.remove(app_ini_file)
        shutil.copy2(ini_file, app_ini_file)
        logging.info(f"Copy {ini_file} to {app_ini_file}")
        return True

    def install_clone_requirements(self):
        """Create the clone's .venv and install its Python deps for tos.py.

        Upstream expects tos.py to run inside the .venv its export script
        creates: cmake/ninja/kconfiglib come from pyproject.toml as pip
        packages, and tos.py's prepare step looks for cmake/ninja next to
        sys.executable. Build that venv ourselves (it is gitignored
        upstream, so it survives clone reuse and CI caching), point
        self.clone_python at it, and fail loudly on any pip error - a
        silently skipped install strands tos.py on a fresh CI runner.
        """
        venv_dir = os.path.join(self.clone_path, ".venv")
        bin_dir = "Scripts" if os.name == "nt" else "bin"
        self.clone_python = os.path.join(venv_dir, bin_dir, "python")

        if not os.path.exists(self.clone_python):
            result = subprocess.run(
                [sys.executable, "-m", "venv", venv_dir],
                capture_output=True,
                text=True,
            )
            if result.returncode != 0:
                logging.error(f"venv creation failed: {result.stderr[-2000:]}")
                return False
            logging.info(f"Created venv at {venv_dir}")

        targets = []
        for req in [
            os.path.join(self.clone_path, "requirements.txt"),
            os.path.join(self.clone_path, "tools", "requirements.txt"),
        ]:
            if os.path.exists(req):
                targets.append(["-r", req])
        if os.path.exists(os.path.join(self.clone_path, "pyproject.toml")):
            targets.append([self.clone_path])

        if not targets:
            logging.warning("No python requirements found in clone")
            return True

        for target in targets:
            result = subprocess.run(
                [self.clone_python, "-m", "pip", "install", "-q"] + target,
                capture_output=True,
                text=True,
            )
            if result.returncode != 0:
                logging.error(
                    f"pip install {' '.join(target)} failed: {result.stderr[-2000:]}"
                )
                return False
            logging.info(f"Installed python deps: {' '.join(target)}")
        return True

    def clone_env(self):
        """Env for tos.py with the clone's .venv activated.

        Pointing only the outer interpreter at the venv is not enough: the
        platform build resolves python/cmake/ninja from PATH again (T5's
        cmake runs confgen.py with the PATH python3), so a runner's bare
        system python lacks the SDK's deps. Prepend the venv's bin dir the
        way upstream's export script does.
        """
        env = os.environ.copy()
        venv_dir = os.path.join(self.clone_path, ".venv")
        bin_dir = os.path.join(venv_dir, "Scripts" if os.name == "nt" else "bin")
        env["VIRTUAL_ENV"] = venv_dir
        env["PATH"] = bin_dir + os.pathsep + env.get("PATH", "")
        env.pop("PYTHONHOME", None)
        return env

    def build_platform(self):
        tos = os.path.join(self.clone_path, "tos.py")
        if not os.path.exists(tos):
            logging.error(f"tos.py not found: {tos}")
            return False

        if not self.install_clone_requirements():
            return False

        work_dir = self.build_app_path

        try:
            subprocess.run(
                [self.clone_python, tos, "clean", "-f"],
                cwd=work_dir,
                env=self.clone_env(),
                capture_output=True,
                text=True,
                timeout=60,
            )
            logging.info("Build cache cleaned")
        except Exception as e:
            logging.warning(f"Clean failed: {e}, continuing")

        build_lines = []
        try:
            process = subprocess.Popen(
                [self.clone_python, tos, "build"],
                cwd=work_dir,
                env=self.clone_env(),
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
            )
            for line in process.stdout:
                build_lines.append(line.rstrip())
                logging.debug(f"[BUILD] {line.rstrip()}")
            process.wait()

            if process.returncode != 0:
                logging.error(f"Build failed with return code: {process.returncode}")
                logging.error("--- Last 50 lines of build output ---")
                for line in build_lines[-50:]:
                    logging.error(f"[BUILD] {line}")
                return False
        except Exception as e:
            logging.error(f"Build error: {e}")
            return False

        return self.verify_kconfig_applied()

    def verify_kconfig_applied(self):
        """Assert every symbol in app_default.config survived kconfig resolution.

        set_platform_ini replaces the upstream app_default.config wholesale, and
        kconfig silently ignores unknown symbols. Without this check, an upstream
        rename drops a feature from the vendor package with no warning at all.
        """
        config_file = os.path.join(self.build_app_path, "app_default.config")
        kconfig_h = os.path.join(self.build_app_path, ".build", "include", "tuya_kconfig.h")

        if not os.path.exists(config_file):
            logging.error(f"app_default.config not found: {config_file}")
            return False
        if not os.path.exists(kconfig_h):
            # Some platforms fall back to a prebuilt sdkconfig.h; nothing to check.
            logging.warning(f"tuya_kconfig.h not found, skipping symbol check: {kconfig_h}")
            return True

        defines = {}
        with open(kconfig_h, "r") as f:
            for line in f:
                m = re.match(r"^#define\s+([A-Za-z_]\w*)\s*(.*)$", line.strip())
                if m:
                    defines[m.group(1)] = m.group(2).strip()

        mismatches = []
        with open(config_file, "r") as f:
            for raw in f:
                line = raw.strip()
                if not line:
                    continue

                unset = re.match(r"^#\s*CONFIG_(\w+)\s+is not set$", line)
                if unset:
                    name = unset.group(1)
                    if name in defines:
                        mismatches.append(f"{name}: expected unset, but defined as {defines[name]}")
                    continue
                if line.startswith("#"):
                    continue

                kv = re.match(r"^CONFIG_(\w+)=(.*)$", line)
                if not kv:
                    continue
                name, value = kv.group(1), kv.group(2).strip()

                if value == "n":
                    if name in defines:
                        mismatches.append(f"{name}: expected unset, but defined as {defines[name]}")
                    continue

                if name not in defines:
                    mismatches.append(f"{name}: absent from tuya_kconfig.h (renamed upstream, or its parent menu is off)")
                    continue

                expected = "1" if value == "y" else value
                if defines[name] != expected:
                    mismatches.append(f"{name}: expected {expected}, got {defines[name]}")

        if mismatches:
            logging.error(f"{len(mismatches)} symbol(s) in app_default.config did not survive kconfig:")
            for item in mismatches:
                logging.error(f"  - {item}")
            return False

        logging.info("All app_default.config symbols verified against tuya_kconfig.h")
        return True

    def copy_libs(self, libs_list_file, output_path, prefix=None):
        if not os.path.exists(libs_list_file):
            logging.error(f"Libs list file not exists: {libs_list_file}")
            return False

        libs_list = self.handler.add_prefix_to_list(libs_list_file, prefix)
        result = True
        for lib in libs_list:
            if not os.path.exists(lib):
                logging.error(f"Lib not exists: {lib}")
                result = False
                continue
            shutil.copy2(lib, output_path)
            logging.debug(f"Copy {lib} to {output_path}")
        return result

    def copy_after_delete(self, src, dst):
        if not os.path.exists(src):
            logging.error(f"Src path not exists: {src}")
            return False

        if os.path.exists(dst):
            if os.path.isdir(src):
                shutil.rmtree(dst)
            else:
                os.remove(dst)

        if os.path.isdir(src):
            shutil.copytree(src, dst)
        else:
            shutil.copy2(src, dst)
        return True

    def copy_include_dir_from_file(self, src_file, dst, ignore_filenames=None):
        if not os.path.exists(src_file):
            logging.error(f"Src file not exists: {src_file}")
            return False

        ignore_fn = None
        if ignore_filenames:
            ignore_set = set(ignore_filenames)

            def ignore_fn(_dir, names):
                return [n for n in names if n in ignore_set]

        chip = self.package_info.chip
        chip_case_map = {
            "t2": ("platform/t2/", "platform/T2/"),
            "t3": ("platform/t3/", "platform/T3/"),
            "ln882h": ("platform/ln882h/", "platform/LN882H/"),
            "esp32": ("platform/esp32/", "platform/ESP32/"),
        }

        with open(src_file, "r") as f:
            include_dirs = [line.rstrip() for line in f if line.strip()]

        for original_include_dir in include_dirs:
            include_dir = original_include_dir

            if chip in chip_case_map:
                old, new = chip_case_map[chip]
                if include_dir.startswith(old):
                    include_dir = include_dir.replace(old, new, 1)

            src_dir = os.path.normpath(os.path.join(self.clone_path, include_dir))
            if not os.path.exists(src_dir):
                logging.warning(f"Include dir not exists: {src_dir}")
                continue

            dst_dir = os.path.join(dst, original_include_dir)
            try:
                shutil.copytree(src_dir, dst_dir, dirs_exist_ok=True, ignore=ignore_fn)
                logging.debug(f"Copy {src_dir} to {dst_dir}")
            except Exception as e:
                logging.error(f"Copy {src_dir} to {dst_dir} failed: {e}")
                return False

        return True

    def copy_include_file(self, src_file, dst, prefix=None):
        if not os.path.exists(src_file):
            logging.error(f"Src file not exists: {src_file}")
            return False

        output_list = []
        with open(src_file, "r") as f:
            for line in f:
                line = line.strip()
                if line:
                    if prefix is not None:
                        line = prefix + line
                    output_list.append(line)

        with open(dst, "w") as f:
            f.write("\n".join(output_list))
        return True

    def copy_tuya_kconfig(self, output_path):
        chip = self.package_info.chip
        tuya_kconfig = os.path.join(
            self.build_app_path, ".build", "include", "tuya_kconfig.h"
        )
        if chip == "esp32":
            tuya_kconfig_output = os.path.join(
                output_path, "platform", "ESP32", "tuya_open_sdk", "tuyaos_adapter", "include", "tuya_kconfig.h"
            )
        else:
            tuya_kconfig_output = os.path.join(
                output_path, "platform", chip, "tuyaos", "tuyaos_adapter", "include", "tuya_kconfig.h"
            )

        os.makedirs(os.path.dirname(tuya_kconfig_output), exist_ok=True)

        if os.path.exists(tuya_kconfig):
            self.copy_after_delete(tuya_kconfig, tuya_kconfig_output)
        else:
            sdkconfig_path = os.path.join(
                self.clone_path, "platform", chip, "t3_os", "tuya_build", "bk7236",
                "config", "sdkconfig.h",
            )
            if os.path.exists(sdkconfig_path):
                shutil.copy2(sdkconfig_path, tuya_kconfig_output)
                logging.info(f"Copy {sdkconfig_path} to {tuya_kconfig_output}")
            else:
                logging.warning(f"tuya_kconfig.h not found")

    def copy_tuya_open(self, output_path):
        """Copy TuyaOpen SDK includes, flags, and package as tar.bz2."""
        staging = self.staging_path

        include_tuya_open_file = os.path.join(staging, "includes", "include_tuya_open.txt")
        include_tkl_file = os.path.join(staging, "includes", "include_tkl.txt")
        include_vendor_file = os.path.join(staging, "includes", "include_vendor.txt")

        # Ship only this repo's lang_config.h: upstream generates a localized one
        # (zh-CN by default) into src/ai_components/assets/include, and two copies
        # on the include path make the language depend on search order.
        if not self.copy_include_dir_from_file(
            include_tuya_open_file, output_path, ignore_filenames={"lang_config.h"}
        ):
            return False
        logging.info("Copy open sdk include success")

        if not self.copy_include_dir_from_file(include_tkl_file, output_path):
            return False
        logging.info("Copy tkl include success")

        tools_adapter_src = os.path.join(self.clone_path, "tools", "porting", "adapter")
        tools_adapter_dst = os.path.join(output_path, "tools", "porting", "adapter")
        if os.path.exists(tools_adapter_src):
            self.copy_after_delete(tools_adapter_src, tools_adapter_dst)
            logging.info("Copy tools/porting/adapter success")

        self.copy_tuya_kconfig(output_path)

        if not self.copy_include_dir_from_file(include_vendor_file, output_path):
            return False
        logging.info("Copy vendor include success")

        flags_path = os.path.join(output_path, "flags")
        os.makedirs(flags_path, exist_ok=True)

        self.copy_include_file(
            include_tuya_open_file, os.path.join(flags_path, "include_tuya_open.txt"), "-iwithprefixbefore "
        )
        self.copy_include_file(
            include_tkl_file, os.path.join(flags_path, "include_tkl.txt"), "-iwithprefixbefore "
        )
        self.copy_include_file(
            include_vendor_file, os.path.join(flags_path, "include_vendor.txt"), "-iwithprefixbefore "
        )

        flag_files = ["c_flags.txt", "cpp_flags.txt", "S_flags.txt", "ld_flags.txt", "ar_flags.txt", "libs_flags.txt"]
        for flag_file in flag_files:
            src = os.path.join(staging, "flags", flag_file)
            if os.path.exists(src):
                self.copy_after_delete(src, os.path.join(flags_path, flag_file))

        vendor_package = os.path.join(
            self.package_info.output_path, self.package_info.package_name
        )
        self.handler.compress_package(output_path, vendor_package, "tar.bz2")

        self.package_info.compute_file_info(vendor_package)

        return True
