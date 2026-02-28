#coding=utf-8
# packager/packager.py

import os
import logging
import subprocess
import json
import platform

from .chip_info import *

def merge_bin_files(file1_path, file2_path, output_file_path):
    try:
        with open(file1_path, 'rb') as file1:
            content1 = file1.read()

        with open(file2_path, 'rb') as file2:
            content2 = file2.read()

        merged_content = content1 + content2

        with open(output_file_path, 'wb') as output_file:
            output_file.write(merged_content)

    except Exception as e:
        print(f"error: {e}")

def get_qio_binary_ln882h(chip_info):
    if platform.system() == 'Windows':
        ota_tool = os.path.join(chip_info.tools_path, 'windows', 'xz_tool.exe')
    elif platform.system() == 'Linux':
        ota_tool = os.path.join(chip_info.tools_path, 'linux', 'xz_tool')
    elif platform.system() == 'Darwin':
        mac_arch = platform.machine()
        logging.info(f"MAC machine is: {mac_arch}")
        ota_tool = os.path.join(chip_info.tools_path, 'mac', mac_arch, 'xz_tool')
    else:
        logging.error(f"Unsupported platform: {platform.system()}")
        return False
    
    ln882h_bootloader = os.path.join(chip_info.tools_path, "ln882h_bootloader.bin")

    logging.debug(f"ota_tool: {ota_tool}")
    logging.debug(f"ln882h_bootloader: {ln882h_bootloader}")

    if not os.path.exists(ota_tool):
        logging.error("ota_tool not exists")
        return False

    if not os.path.exists(ln882h_bootloader):
        logging.error("ln882h_bootloader not find")
        return False

    os.chdir(chip_info.output_path)
  
    logging.info(f"chip_info.bin_file: {chip_info.bin_file}")

    chip_info.bin_file_QIO = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_QIO_{chip_info.sketch_version}.bin")
    logging.info(f"QIO binary file: {chip_info.bin_file_QIO}")

    merge_bin_files(ln882h_bootloader, chip_info.bin_file, chip_info.bin_file_QIO)

    ota_command = [
        ota_tool,
        '-i',
        chip_info.bin_file,
        '-v',
        chip_info.sketch_version
    ]

    logging.info("ota_command: " + ' '.join(ota_command))
    result = subprocess.run(ota_command)
    if result.returncode != 0:
        logging.error("ota_command failed")
        return False

    # Print build success information
    qio_bin_name = os.path.basename(chip_info.bin_file_QIO)
    
    logging.info("")
    logging.info("[NOTE]:")
    logging.info("====================[ BUILD SUCCESS ]====================")
    logging.info(f" Target    : {qio_bin_name}")
    logging.info(f" Output    : {chip_info.output_path}")
    logging.info(f" Chip      : {chip_info.chip}")
    logging.info(f" Board     : {chip_info.board}")
    logging.info(f" Framework : Arduino")
    logging.info("========================================================")

    return True
