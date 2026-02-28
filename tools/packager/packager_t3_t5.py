import os
import logging
import shutil
import platform
import subprocess
import json

from .chip_info import *

t3_config_str = '''
{
    "magic": "FreeRTOS",
    "version": "0.1",
    "count": 2,
    "section": [
        {
            "firmware": "bootloader.bin",
            "version": "2M.1220",
            "partition": "bootloader",
            "start_addr": "0x00000000",
            "size": "64K"
        },
        {
            "firmware": "app.bin",
            "version": "2M.1220",
            "partition": "app",
            "start_addr": "0x00010000",
            "size": "2176K"
        }
    ]
}
'''

t5_config_str = '''
{
    "magic": "FreeRTOS",
    "version": "0.1",
    "count": 3,
    "section": [
        {
            "firmware": "bootloader.bin",
            "version": "2M.1220",
            "partition": "bootloader",
            "start_addr": "0x0",
            "size": "68K"
        },
        {
            "firmware": "cp_app.bin",
            "version": "2M.1220",
            "partition": "cp_app",
            "start_addr": "0x00010000",
            "size": "1244K"
        },
        {
            "firmware": "app.bin",
            "version": "2M.1220",
            "partition": "ap_app",
            "start_addr": "0x00130000",
            "size": "3876K"
        }
    ]
}
'''

def get_qio_binary_t3_t5(chip_info):
    logging.debug(f"platform system: {platform.system()}")
    if platform.system() == 'Windows':
        cmake_Gen_image_tools = os.path.join(chip_info.tools_path, 'windows', 'cmake_Gen_image.exe')
        cmake_encrypt_crc_tool = os.path.join(chip_info.tools_path, 'windows', 'cmake_encrypt_crc.exe')
    elif platform.system() == 'Linux':
        cmake_Gen_image_tools = os.path.join(chip_info.tools_path, 'linux', 'cmake_Gen_image')
        cmake_encrypt_crc_tool = os.path.join(chip_info.tools_path, 'linux', 'cmake_encrypt_crc')
    elif platform.system() == 'Darwin':
        mac_arch = platform.machine()
        logging.info(f"MAC machine is: {mac_arch}")
        cmake_Gen_image_tools = os.path.join(chip_info.tools_path, 'mac', mac_arch, 'cmake_Gen_image')
        cmake_encrypt_crc_tool = os.path.join(chip_info.tools_path, 'mac', mac_arch, 'cmake_encrypt_crc')
    else:
        logging.error(f"Unknown OS: {platform.system()}")
        return False

    bootloader_file = os.path.join(chip_info.tools_path, chip_info.chip + '_bootloader.bin')

    logging.debug(f"cmake_Gen_image: {cmake_Gen_image_tools}")
    logging.debug(f"cmake_encrypt_crc: {cmake_encrypt_crc_tool}")
    logging.debug(f"bootloader_file: {bootloader_file}")

    if not os.path.exists(cmake_Gen_image_tools) or not os.path.exists(cmake_encrypt_crc_tool):
        logging.error("cmake_Gen_image or cmake_encrypt_crc not found")
        return False

    if not os.path.exists(bootloader_file):
        logging.error("bootloader_file not find")
        return False

    os.chdir(chip_info.output_path)

    # Generate json file
    if chip_info.chip == 't3':
        config_str = t3_config_str
    elif chip_info.chip == 'T5':
        config_str = t5_config_str
    else:
        return False

    config_json = json.loads(config_str)
    config_json["section"][0]["firmware"] = bootloader_file
    
    # For T5: set CP core firmware (section[1]) and AP core firmware (section[2])
    if chip_info.chip == 'T5':
        cp_app_file = os.path.join(chip_info.tools_path, 't5_cp_app.bin')
        if not os.path.exists(cp_app_file):
            logging.error(f"CP core app not found: {cp_app_file}")
            logging.error("T5 requires CP core (Communication Processor) to initialize WiFi/BLE before AP core")
            return False
        config_json["section"][1]["firmware"] = cp_app_file
        config_json["section"][2]["firmware"] = chip_info.bin_file
    else:
        # T3: only 2 sections
        config_json["section"][1]["firmware"] = chip_info.bin_file
    
    logging.debug("config_json: " + json.dumps(config_json, indent=4))
    config_file = os.path.join(chip_info.output_path, "config.json")
    with open(config_file, 'w') as f:
        json.dump(config_json, f, indent=4)

    all_app_pack_file = os.path.join(chip_info.output_path, 'all_app_pack.bin')
    
    # Build gen_image command based on chip type
    if chip_info.chip == 'T5':
        # T5: 3 files (bootloader + CP core + AP core)
        cp_app_file = os.path.join(chip_info.tools_path, 't5_cp_app.bin')
        gen_image_command = [
            cmake_Gen_image_tools,
            'genfile',
            '-injsonfile',
            config_file,
            '-infile',
            bootloader_file,
            cp_app_file,
            chip_info.bin_file,
            '-outfile',
            all_app_pack_file
        ]
    else:
        # T3: 2 files (bootloader + app)
        gen_image_command = [
            cmake_Gen_image_tools,
            'genfile',
            '-injsonfile',
            config_file,
            '-infile',
            bootloader_file,
            chip_info.bin_file,
            '-outfile',
            all_app_pack_file
        ]
    
    logging.debug("gen_image_command: " + ' '.join(gen_image_command))
    result = subprocess.run(gen_image_command)
    if result.returncode != 0 or not os.path.exists(all_app_pack_file):
        logging.error("gen_image_command failed")
        return False

    all_app_pack_crc_file = os.path.join(chip_info.output_path, 'all_app_pack_crc.bin')
    cmake_encrypt_crc_command = [
        cmake_encrypt_crc_tool,
        '-crc',
        all_app_pack_file
    ]
    logging.debug("cmake_encrypt_crc_command: " + ' '.join(cmake_encrypt_crc_command))
    result = subprocess.run(cmake_encrypt_crc_command)
    if result.returncode != 0 or not os.path.exists(all_app_pack_crc_file):
        logging.error("cmake_encrypt_crc_command failed")
        return False

    chip_info.bin_file_QIO = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_QIO_{chip_info.sketch_version}.bin")

    shutil.move(all_app_pack_crc_file, chip_info.bin_file_QIO)

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

