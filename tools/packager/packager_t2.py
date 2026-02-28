#coding=utf-8
# packager/packager.py

import os
import logging
import subprocess
import json
import platform

from .chip_info import *

def t2_combine_qio_ate(outfile, size, files):
        if len(files) < 2 or len(files) % 2 != 0:
            logging.error("Files and offsets must be provided in pairs.")
            return False

        sorted_files = [(files[i], int(files[i + 1])) for i in range(0, len(files), 2)]

        total_size = 0
        occupied = 0

        for file_path, ofs in sorted_files:
            if not os.path.exists(file_path):
                logging.error(f"File not found: {file_path}")
                return False

            file_size = os.path.getsize(file_path)
            logging.debug(f"file: {file_path} ofs: {ofs} size: {file_size}")

            if ofs + file_size > size:
                logging.error(f"{file_path} is too big or its offset is incorrect.")
                return False

            if ofs < occupied:
                logging.error("Address conflicts.")
                logging.error(f"{ofs} --- {occupied}")
                return False

            occupied = ofs + file_size
            total_size += file_size

        if total_size > size:
            logging.error("Error: total file size is greater than output file size")
            return False

        if os.path.exists(outfile):
            os.remove(outfile)

        logging.debug("--- start -----------------")
        with open(outfile, 'wb') as fout:
            for file_path, ofs in sorted_files:
                with open(file_path, 'rb') as fin:
                    file_data = fin.read()
                    fout.seek(ofs)
                    fout.write(file_data)
                    logging.debug(f"write {file_path}, seek {ofs}, write {len(file_data)}")
                    logging.debug("--------------------")
        logging.debug("--- end -----------------")
        return True

def get_qio_binary_t2(chip_info):
    if platform.system() == 'Windows':
        encrypt_tool = os.path.join(chip_info.tools_path, 'windows', 'encrypt.exe')
        t2_packager_tool = os.path.join(chip_info.tools_path, 'windows', 'beken_packager.exe')
    elif platform.system() == 'Linux':
        encrypt_tool = os.path.join(chip_info.tools_path, 'linux', 'encrypt')
        t2_packager_tool = os.path.join(chip_info.tools_path, 'linux', 'beken_packager')
    else:
        logging.error(f"Unsupported platform: {platform.system()}")
        return False
    
    t2_bootloader = os.path.join(chip_info.tools_path, "t2_bootloader_enc.bin")
    t2_ate_file = os.path.join(chip_info.tools_path, "t2_ate")

    logging.debug(f"encrypt_tool: {encrypt_tool}")
    logging.debug(f"t2_packager_tool: {t2_packager_tool}")
    logging.debug(f"t2_bootloader: {t2_bootloader}")
    logging.debug(f"t2_ate_file: {t2_ate_file}")

    if not os.path.exists(encrypt_tool) or not os.path.exists(t2_packager_tool):
        logging.error("encrypt tool or beken packager tool not exists")
        return False
    
    if not os.path.exists(t2_bootloader) or not os.path.exists(t2_ate_file):
        logging.error("t2_bootloader or t2_ate not found")
        return False

    os.chdir(chip_info.output_path)

    # get _enc.bin
    encrypt_command = [
        encrypt_tool,
        chip_info.bin_file,
        '510fb093',
        'a3cbeadc',
        '5993a17e',
        'c7adeb03',
        '10000'
    ]
    logging.debug("Objcopy command: " + ' '.join(encrypt_command))
    subprocess.run(encrypt_command)

    t2_enc_binary_file = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_enc.bin")
    if not os.path.exists(t2_enc_binary_file):
        logging.error("t2_enc_binary_file not found")
        return False

    config_str = '''
    {
        "count": 4,
        "magic": "RT-Thread",
        "section": [
            {
                "firmware": "t2_bootloader_enc.bin",
                "partition": "bootloader",
                "size": "68K",
                "start_addr": "0x00000000",
                "version": "1.00"
            },
            {
                "firmware": "t2_common_1.0.1_enc.bin",
                "partition": "app",
                "size": "1150832",
                "start_addr": "0x00011000",
                "version": "1.00"
            }
        ],
        "version": "0.1"
    }
    '''
    config_json = json.loads(config_str)
    config_json["section"][0]["firmware"] = t2_bootloader
    config_json["section"][1]["firmware"] = t2_enc_binary_file

    logging.debug("config_json: " + json.dumps(config_json, indent=4))
    config_file = os.path.join(chip_info.output_path, "config.json")
    with open(config_file, 'w') as f:
        json.dump(config_json, f, indent=4)

    t2_package_command = [
        t2_packager_tool,
        config_file
    ]
    logging.debug("t2_package_command: " + ' '.join(t2_package_command))
    subprocess.run(t2_package_command)
    all_bin_file = os.path.join(chip_info.output_path, "all_1.00.bin")

    chip_info.bin_file_QIO = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_QIO_{chip_info.sketch_version}.bin")

    if not t2_combine_qio_ate(chip_info.bin_file_QIO, 2097152, [all_bin_file, "0", t2_ate_file, "1253376"]):
        logging.error("t2_combine_qio_ate failed")
        chip_info.bin_file_QIO = None
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


