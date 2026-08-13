# -*- coding: utf-8 -*-

import logging
import os
import re
import subprocess
import shutil
import json
import hashlib
import tempfile

from .package_platform import PackagePlatform


class PackagePlatformT5(PackagePlatform):
    def _set_language(self, chinese):
        """Flip the AI language choice in the working app_default.config.

        Kconfig models the language as an exclusive choice, so exactly one
        of the two symbols must be =y and the other 'is not set'.
        """
        config_file = os.path.join(self.build_app_path, "app_default.config")
        with open(config_file, "r") as f:
            text = f.read()

        en_on = "CONFIG_ENABLE_AI_LANGUAGE_ENGLISH=y"
        en_off = "# CONFIG_ENABLE_AI_LANGUAGE_ENGLISH is not set"
        zh_on = "CONFIG_ENABLE_AI_LANGUAGE_CHINESE=y"
        zh_off = "# CONFIG_ENABLE_AI_LANGUAGE_CHINESE is not set"

        if en_on not in text or zh_off not in text:
            logging.error("Language symbols not found in app_default.config baseline")
            return False
        if chinese:
            text = text.replace(en_on, en_off).replace(zh_off, zh_on)
        with open(config_file, "w") as f:
            f.write(text)
        logging.info(f"Language set to {'Chinese' if chinese else 'English'}")
        return True

    def _toolchain_tool(self, name):
        """Locate an arm-none-eabi binutil next to the compiler used by the build."""
        compile_commands_file = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258_ap",
            "compile_commands.json",
        )
        if not os.path.exists(compile_commands_file):
            logging.error(f"compile_commands.json not found: {compile_commands_file}")
            return None
        with open(compile_commands_file, "r") as f:
            compiler = json.load(f)[0]["command"].split(" ")[0]
        tool = os.path.join(os.path.dirname(compiler), f"arm-none-eabi-{name}")
        if not os.path.exists(tool):
            logging.error(f"Toolchain tool not found: {tool}")
            return None
        return tool

    def _ai_member_names(self):
        """Archive members owned by src/ai_components: <basename>.c -> <basename>.c.o."""
        members = set()
        root = os.path.join(self.clone_path, "src", "ai_components")
        for _dirpath, _dirs, files in os.walk(root):
            for name in files:
                if name.endswith(".c"):
                    members.add(name + ".o")
        return members

    def _find_built_libtuyaos(self):
        build_ninja = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258_ap",
            "build.ninja",
        )
        if not os.path.exists(build_ninja):
            logging.error(f"build.ninja not found: {build_ninja}")
            return None
        base = os.path.dirname(build_ninja)
        with open(build_ninja, "r") as f:
            for line in f:
                if "LINK_LIBRARIES" not in line:
                    continue
                for token in line.split():
                    if token.endswith("libtuyaos.a"):
                        if not os.path.isabs(token):
                            token = os.path.normpath(os.path.join(base, token))
                        return token
        logging.error("libtuyaos.a not found in AP build.ninja LINK_LIBRARIES")
        return None

    def _extract_lang_archive(self, ar, src_archive, member_names, out_archive):
        """Pull the ai_components members out of src_archive into out_archive.

        Returns the extracted member list, or None on failure.
        """
        result = subprocess.run([ar, "t", src_archive], capture_output=True, text=True)
        if result.returncode != 0:
            logging.error(f"ar t failed on {src_archive}: {result.stderr}")
            return None
        members = [m for m in result.stdout.split() if m in member_names]
        if not members:
            logging.error(f"No ai_components members found in {src_archive}")
            return None

        try:
            with tempfile.TemporaryDirectory() as tmp:
                subprocess.run([ar, "x", os.path.abspath(src_archive)] + members,
                               cwd=tmp, check=True)
                if os.path.exists(out_archive):
                    os.remove(out_archive)
                subprocess.run([ar, "rcs", os.path.abspath(out_archive)] + members,
                               cwd=tmp, check=True)
        except subprocess.CalledProcessError as e:
            logging.error(f"ar extraction into {out_archive} failed: {e}")
            return None
        logging.info(f"Packed {len(members)} members into {out_archive}")
        return members

    def _defined_symbols(self, nm, archive):
        result = subprocess.run([nm, "-g", "--defined-only", archive],
                                capture_output=True, text=True)
        return result.stdout

    def verify_lang_split(self, ar, nm, output_lib_path, member_names):
        """Fail packaging unless the language split is complete and non-empty."""
        ok = True

        en_archive = os.path.join(output_lib_path, "libailang_en.a")
        if "media_src_prologue_en" not in self._defined_symbols(nm, en_archive):
            logging.error("libailang_en.a does not define media_src_prologue_en")
            ok = False

        zh_archive = os.path.join(output_lib_path, "libailang_zh.a")
        if "media_src_prologue_zh" not in self._defined_symbols(nm, zh_archive):
            logging.error(
                "libailang_zh.a does not define media_src_prologue_zh "
                "(Chinese voice data compiled out?)"
            )
            ok = False

        base = os.path.join(output_lib_path, "libtuyaos.a")
        result = subprocess.run([ar, "t", base], capture_output=True, text=True)
        if result.returncode != 0:
            logging.error(f"ar t failed on {base}: {result.stderr}")
            ok = False
        else:
            leftovers = [m for m in result.stdout.split() if m in member_names]
            if leftovers:
                logging.error(f"libtuyaos.a still contains ai members: {leftovers}")
                ok = False

        if ok:
            logging.info("Language split verified: en/zh archives populated, base clean")
        return ok

    def _header_string_keys(self, path):
        keys = set()
        with open(path, "r", encoding="utf-8") as f:
            for line in f:
                m = re.match(r'\s*#define\s+([A-Za-z_0-9]+)\s+"', line)
                if m:
                    keys.add(m.group(1))
        return keys

    def verify_repo_lang_header(self, generated_headers):
        """The repo's bilingual lang_config.h must cover every upstream key.

        Guards against upstream adding strings (branch is unpinned master):
        a missing macro would break Arduino-side compilation of vendor headers.
        """
        repo_root = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", "..")
        )
        repo_header = os.path.join(
            repo_root, "libraries", "AIcomponents", "src", "lang_config.h"
        )
        repo_keys = self._header_string_keys(repo_header)
        ok = True
        for generated in generated_headers:
            missing = self._header_string_keys(generated) - repo_keys
            if missing:
                logging.error(
                    f"lang_config.h drift, missing keys {sorted(missing)} "
                    f"(vs {generated}); regenerate with "
                    "tools/gen_bilingual_lang_header.py"
                )
                ok = False
        return ok

    def copy_from_string(self, input_str, output_lib_path, base_path=None):
        input_str = input_str.strip()
        tmp_list = [x for x in input_str.split(" ") if x]
        lib_list = list(set(lib for lib in tmp_list if lib.endswith(".a")))

        remove_lib_list = ["libtuyaapp.a"]
        rename_lib_dict = {"/bk7258/libs/libbk_phy.a": "libbk7258_bk_phy.a"}

        output_lib_dict = {}
        for lib in lib_list:
            if os.path.basename(lib) in remove_lib_list:
                continue
            if not os.path.isabs(lib):
                if base_path:
                    lib = os.path.join(base_path, lib)
                elif lib.startswith("armino/"):
                    lib = os.path.join(self.vendor_path, "t5_os", "build", "bk7258", lib)
                lib = os.path.normpath(lib)
            output_lib_dict[lib] = os.path.basename(lib)
            for rename_key, new_name in rename_lib_dict.items():
                if rename_key in lib:
                    output_lib_dict[lib] = new_name
                    break

        for lib, out_name in output_lib_dict.items():
            output_lib_file = os.path.join(output_lib_path, out_name)
            shutil.copy2(lib, output_lib_file)
            logging.debug(f"Copy {lib} to {output_lib_file} success")
        return True

    def get_libs_flags(self, input_str, output_file):
        link_txt_lists = [x for x in input_str.split(" ") if x]

        remove_lib_list = ["libtuyaapp.a"]
        for remove_lib in remove_lib_list:
            link_txt_lists = [x for x in link_txt_lists if remove_lib not in x]

        lib_list = [x for x in link_txt_lists if x.startswith("-l") or x.endswith(".a")]

        rename_lib_dict = {"/bk7258/libs/libbk_phy.a": "libbk7258_bk_phy.a"}
        write_lib_list = []
        for lib in lib_list:
            tmp_name = os.path.basename(lib)
            for rename_key, new_name in rename_lib_dict.items():
                if rename_key in lib:
                    tmp_name = new_name
                    break
            if tmp_name.endswith(".a"):
                write_lib_list.append(f"-l{tmp_name[3:-2]}")
            else:
                write_lib_list.append(tmp_name)

        for i in range(min(9, len(write_lib_list) - 1), -1, -1):
            if write_lib_list[i] in ["-lm", "-lgcc", "-lc", "-lnosys"]:
                write_lib_list.pop(i)

        with open(output_file, "w") as f:
            for lib in write_lib_list:
                lib_stripped = lib.strip()
                if lib_stripped:
                    f.write(f"{lib_stripped}\n")
        return True

    def get_include_flags(self, compile_commands_file, output_path):
        with open(compile_commands_file, "r") as f:
            compile_json = json.load(f)
            compile_commands = compile_json[0]["command"]

        compile_list = [x for x in compile_commands.split(" ") if x]
        include_list = list(set(
            item[2:].replace(self.clone_path + "/", "")
            for item in compile_list if item.startswith("-I")
        ))

        tuya_open_include_list = [x for x in include_list if x.startswith("src/")]
        with open(os.path.join(output_path, "include_tuya_open.txt"), "w") as f:
            f.write("\n".join(tuya_open_include_list) + "\n")

        tuyaos_adapter_include_list = [
            x for x in include_list if x.startswith("platform/T5AI/tuyaos/tuyaos_adapter/")
        ]
        with open(os.path.join(output_path, "include_tkl.txt"), "w") as f:
            f.write("platform/T5AI/tuyaos/tuyaos_adapter/include\n")
            f.write("platform/T5AI/tuyaos/tuyaos_adapter/include/security\n")
            for item in tuyaos_adapter_include_list:
                f.write(f"{item}\n")

        vendor_include_list = [x for x in include_list if x.startswith("platform/T5AI/")]
        with open(os.path.join(output_path, "include_vendor.txt"), "w") as f:
            f.write("\n".join(vendor_include_list) + "\n")
        return True

    def copy_tuya_kconfig(self, output_path):
        pass

    def copy_assets(self, output_tmp_path, chip, partitions_file):
        build_ninja_path = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258_ap", "build.ninja"
        )
        build_base_path = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258_ap"
        )
        if not os.path.exists(build_ninja_path):
            logging.error(f"Can't find {build_ninja_path}")
            return False

        link_info = ""
        with open(build_ninja_path, "r") as f:
            for line in f:
                if "LINK_LIBRARIES" in line:
                    link_info = line.strip()
                    break
        if not link_info:
            logging.error("Can't find LINK_LIBRARIES")
            return False

        output_lib_path = os.path.join(output_tmp_path, "libs")
        if os.path.exists(output_lib_path):
            shutil.rmtree(output_lib_path)
        os.makedirs(output_lib_path)

        self.copy_from_string(link_info, output_lib_path, build_base_path)

        self.staging_path = os.path.join(output_tmp_path, "_staging")
        os.makedirs(self.staging_path, exist_ok=True)
        shutil.copytree(
            os.path.join(self.data_path, "flags"),
            os.path.join(self.staging_path, "flags"),
        )

        libs_flags_file = os.path.join(self.staging_path, "flags", "libs_flags.txt")
        self.get_libs_flags(link_info, libs_flags_file)

        compile_commands_file = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258_ap", "compile_commands.json"
        )
        if not os.path.exists(compile_commands_file):
            logging.error(f"Can't find {compile_commands_file}")
            return False

        staging_includes = os.path.join(self.staging_path, "includes")
        os.makedirs(staging_includes, exist_ok=True)
        self.get_include_flags(compile_commands_file, staging_includes)

        self.copy_after_delete(
            os.path.join(self.data_path, "packager-tools"),
            os.path.join(output_tmp_path, "packager-tools"),
        )

        package_tmp_path = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "package", "tmp"
        )
        packager_tools_out = os.path.join(output_tmp_path, "packager-tools")

        # Bootloader
        bootloader_src = os.path.join(package_tmp_path, "bootloader.bin")
        if os.path.exists(bootloader_src):
            shutil.copy2(bootloader_src, os.path.join(packager_tools_out, "T5_bootloader.bin"))
            logging.info(f"Copied T5_bootloader.bin ({os.path.getsize(bootloader_src)} bytes)")
        else:
            logging.error(f"T5 bootloader not found: {bootloader_src}")
            return False

        # TuyaBoot
        tuyaboot_src = os.path.join(package_tmp_path, "tuyaboot.bin")
        if os.path.exists(tuyaboot_src):
            shutil.copy2(tuyaboot_src, os.path.join(packager_tools_out, "T5_tuyaboot.bin"))
            logging.info(f"Copied T5_tuyaboot.bin ({os.path.getsize(tuyaboot_src)} bytes)")
        else:
            logging.error(f"T5 tuyaboot not found: {tuyaboot_src}")
            return False

        # CP firmware
        cp_app_src = os.path.join(package_tmp_path, "app.bin")
        if os.path.exists(cp_app_src):
            with open(cp_app_src, "rb") as f:
                cp_md5 = hashlib.md5(f.read()).hexdigest()
            cp_size = os.path.getsize(cp_app_src)
            logging.info(f"CP core app.bin: size={cp_size}, md5={cp_md5}")
            shutil.copy2(cp_app_src, os.path.join(packager_tools_out, "t5_cp_app.bin"))
        else:
            logging.error(f"CP core app.bin not found: {cp_app_src}")
            return False

        return True

    def build_platform_t5(self):
        tos = os.path.join(self.clone_path, "tos.py")
        if not os.path.exists(tos):
            logging.error(f"tos.py not found: {tos}")
            return False

        if not self.install_clone_requirements():
            return False

        work_dir = self.build_app_path
        if not os.path.exists(work_dir):
            logging.error(f"Work directory not exists: {work_dir}")
            return False

        try:
            subprocess.run(
                [self.clone_python, tos, "clean", "-f"],
                cwd=work_dir, env=self.clone_env(),
                capture_output=True, text=True, timeout=60,
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

    def apply_platform_patch(self):
        patch_file = os.path.join(self.data_path, "t5_platform.patch")
        if not os.path.exists(patch_file):
            logging.info("No platform patch file found, skipping")
            return True

        logging.info(f"Applying platform patch from: {patch_file}")
        try:
            subprocess.run(
                ["git", "-C", self.clone_path, "reset", "--hard", "HEAD"],
                capture_output=True, text=True, timeout=30,
            )
            subprocess.run(
                ["git", "-C", self.clone_path, "clean", "-fd"],
                capture_output=True, text=True, timeout=30,
            )

            result = subprocess.run(
                ["git", "-C", self.clone_path, "apply", "--whitespace=nowarn", patch_file],
                capture_output=True, text=True, timeout=30,
            )
            if result.returncode == 0:
                logging.info("Platform patch applied successfully")
                return True

            logging.error(f"Failed to apply patch: {result.stderr}")
            result = subprocess.run(
                ["git", "-C", self.clone_path, "apply", "--reject", "--whitespace=nowarn", patch_file],
                capture_output=True, text=True, timeout=30,
            )
            if result.returncode == 0:
                logging.warning("Patch applied with rejections")
                return True

            logging.error(f"Failed to apply patch: {result.stderr}")
            return False
        except Exception as e:
            logging.error(f"Patch error: {e}")
            return False

    def package(self):
        self.vendor_path = os.path.join(self.clone_path, "platform", "T5AI")

        if not self.git_clone():
            return False
        if not self.apply_platform_patch():
            return False
        ini_file = os.path.join(self.config_path, "app_default.config")

        # Pass 1: Chinese build, only to harvest the language archive and the
        # generated zh lang_config.h. Everything else ships from pass 2.
        if not self.set_platform_ini(ini_file):
            return False
        if not self._set_language(chinese=True):
            return False
        if not self.build_platform_t5():
            return False

        ar = self._toolchain_tool("ar")
        nm = self._toolchain_tool("nm")
        if not ar or not nm:
            return False

        lang_stash = os.path.join(self.package_info.output_path, "lang_stash")
        os.makedirs(lang_stash, exist_ok=True)
        ai_members = self._ai_member_names()

        zh_libtuyaos = self._find_built_libtuyaos()
        if not zh_libtuyaos:
            return False
        zh_archive = os.path.join(lang_stash, "libailang_zh.a")
        if not self._extract_lang_archive(ar, zh_libtuyaos, ai_members, zh_archive):
            return False
        upstream_header = os.path.join(
            self.clone_path, "src", "ai_components", "assets", "include", "lang_config.h"
        )
        zh_header = os.path.join(lang_stash, "lang_config_zh.h")
        shutil.copy2(upstream_header, zh_header)

        # Pass 2: English build; libs, flags, includes and boot binaries all
        # ship from this build, exactly as before.
        if not self.set_platform_ini(ini_file):
            return False
        if not self.build_platform_t5():
            return False
        en_header = os.path.join(lang_stash, "lang_config_en.h")
        shutil.copy2(upstream_header, en_header)

        output_tmp_path = os.path.join(self.package_info.output_path, "tmp", self.package_info.name)
        if os.path.exists(output_tmp_path):
            shutil.rmtree(output_tmp_path)
        os.makedirs(output_tmp_path)

        output_lib_path = os.path.join(output_tmp_path, "libs")
        os.makedirs(output_lib_path, exist_ok=True)

        build_ninja_main = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258", "build.ninja"
        )
        build_base_main = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258"
        )
        if os.path.exists(build_ninja_main):
            logging.info("Extracting libraries from bk7258 main core...")
            with open(build_ninja_main, "r") as f:
                for line in f:
                    if "LINK_LIBRARIES" in line:
                        self.copy_from_string(line.strip(), output_lib_path, build_base_main)
                        break

        build_ninja_ap = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258_ap", "build.ninja"
        )
        build_base_ap = os.path.join(
            self.vendor_path, "t5_os", "build", "bk7258", "tuya_app", "bk7258_ap"
        )
        if not os.path.exists(build_ninja_ap):
            logging.error(f"AP build.ninja not found: {build_ninja_ap}")
            return False

        link_info_ap = ""
        with open(build_ninja_ap, "r") as f:
            for line in f:
                if "LINK_LIBRARIES" in line:
                    link_info_ap = line.strip()
                    break
        if not link_info_ap:
            logging.error("Can't find LINK_LIBRARIES in AP build.ninja")
            return False

        self.copy_from_string(link_info_ap, output_lib_path, build_base_ap)

        partitions_file = os.path.join(
            self.vendor_path, "t5_os", "projects", "tuya_app", "config", "bk7258", "configuration.json"
        )
        if not self.copy_assets(output_tmp_path, "bk7258", partitions_file):
            return False

        tuya_kconfig_src = os.path.join(
            self.build_app_path, ".build", "include", "tuya_kconfig.h"
        )
        tuya_kconfig_dst_dir = os.path.join(
            output_tmp_path, "platform", "T5AI", "tuyaos", "tuyaos_adapter", "include"
        )
        os.makedirs(tuya_kconfig_dst_dir, exist_ok=True)
        tuya_kconfig_dst = os.path.join(tuya_kconfig_dst_dir, "tuya_kconfig.h")

        if os.path.exists(tuya_kconfig_src):
            with open(tuya_kconfig_src, "r") as f:
                content = f.read()
            content = re.sub(
                r'(#define\s+PROJECT_VERSION\s+"[^"]+")',
                r'#ifndef PROJECT_VERSION\n\1\n#endif',
                content,
            )
            content = re.sub(
                r'(#define\s+TUYA_PRODUCT_ID\s+"[^"]+")',
                r'#ifndef TUYA_PRODUCT_ID\n\1\n#endif',
                content,
            )
            with open(tuya_kconfig_dst, "w") as f:
                f.write(content)
        else:
            logging.error(f"tuya_kconfig.h not found at {tuya_kconfig_src}")
            return False

        # Split language objects out of the shipped libtuyaos.a and drop in
        # both language archives. Must run after copy_assets (it recreates libs/).
        output_lib_path = os.path.join(output_tmp_path, "libs")
        shipped_libtuyaos = os.path.join(output_lib_path, "libtuyaos.a")
        en_members = self._extract_lang_archive(
            ar, shipped_libtuyaos, ai_members,
            os.path.join(output_lib_path, "libailang_en.a"),
        )
        if not en_members:
            return False
        try:
            subprocess.run([ar, "d", shipped_libtuyaos] + en_members, check=True)
            subprocess.run([ar, "s", shipped_libtuyaos], check=True)
        except subprocess.CalledProcessError as e:
            logging.error(f"Stripping ai members from {shipped_libtuyaos} failed: {e}")
            return False
        shutil.copy2(zh_archive, os.path.join(output_lib_path, "libailang_zh.a"))

        if not self.verify_lang_split(ar, nm, output_lib_path, ai_members):
            return False
        if not self.verify_repo_lang_header([en_header, zh_header]):
            return False

        if not self.copy_tuya_open(output_tmp_path):
            return False

        return True

