#coding=utf-8
# packager/packager.py

import os
import shutil
import logging
import platform
import subprocess

from .chip_info import chip_info_class
from .packager_t2 import get_qio_binary_t2
from .packager_t3_t5 import get_qio_binary_t3_t5
from .packager_ln882h import get_qio_binary_ln882h
from .packager_esp32 import get_qio_binary_esp32

class packager_class():
    def __init__(self, chip, board, compiler_path, tools_path, output_path, sketch_name, sketch_version):
        self.chip_info = chip_info_class(chip, board, compiler_path, tools_path, output_path, sketch_name, sketch_version)

    def elf_to_binary(self):
        # .elf -> .bin tool
        objcopy_tool = ''
        if self.chip_info.chip == 'esp32':
            if platform.system() == 'Linux':
                objcopy_tool = os.path.join(self.chip_info.tools_path, 'linux', 'esptool')
            elif platform.system() == 'Windows':
                objcopy_tool = os.path.join(self.chip_info.tools_path, 'windows', 'esptool')
            elif platform.system() == 'Darwin':
                mac_arch = platform.machine()
                logging.info(f"MAC machine is: {mac_arch}")
                objcopy_tool = os.path.join(self.chip_info.tools_path, 'mac', mac_arch, 'esptool')
            else:
                logging.error(f"Unknown OS: {platform.system()}")
                return False
        else:
            objcopy_tool = os.path.join(self.chip_info.compiler_path, 'bin', 'arm-none-eabi-objcopy')
        elf_file = os.path.join(self.chip_info.output_path, self.chip_info.sketch_name + '.elf')
        bin_file = os.path.join(self.chip_info.output_path, self.chip_info.sketch_name + '.bin')

        if platform.system() == 'Windows':
            objcopy_tool += '.exe'

        if not os.path.exists(objcopy_tool):
            logging.error(f"{objcopy_tool} not found")
            return False

        if not os.path.exists(elf_file):
            logging.error(f"{elf_file} file not found")
            return False

        objcopy_command = []
        if self.chip_info.chip == 'esp32':
            # TODO: flash_freq and flash_size should be configurable
            bin_file = os.path.join(self.chip_info.output_path, self.chip_info.sketch_name + '_app.bin')
            objcopy_command = [objcopy_tool, '--chip', 'esp32', 'elf2image', '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', '4MB', '--elf-sha256-offset', '0xb0', '--min-rev-full', '0', '--max-rev-full', '399', '-o', bin_file, elf_file]
        else:
            objcopy_command = [objcopy_tool, '-O', 'binary', elf_file, bin_file]
        logging.debug("objcopy_command: " + ' '.join(objcopy_command))
        result = subprocess.run(objcopy_command)
        if result.returncode != 0 or not os.path.exists(bin_file):
            logging.error("objcopy failed")
            return False

        self.chip_info.bin_file = bin_file # .elf -> .bin

        return True

    def get_qio_binary(self):
        if self.chip_info.chip == 't2':
            return get_qio_binary_t2(self.chip_info)
        elif self.chip_info.chip == 't3' or self.chip_info.chip == 'T5':
            return get_qio_binary_t3_t5(self.chip_info)
        elif self.chip_info.chip == 'ln882h':
            return get_qio_binary_ln882h(self.chip_info)
        elif self.chip_info.chip == 'esp32':
            return get_qio_binary_esp32(self.chip_info)
        else:
            return False

    def get_arduino_upload_binary(self, arduino_upload_binary):
        if not os.path.exists(self.chip_info.bin_file_QIO):
            logging.error("bin_file_QIO not found")
            return False

        if os.path.exists(arduino_upload_binary):
            os.remove(arduino_upload_binary)

        shutil.copy2(self.chip_info.bin_file_QIO, arduino_upload_binary)

        return True