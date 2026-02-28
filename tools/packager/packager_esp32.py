import os
import logging
import shutil
import platform

from .chip_info import *

OFFSET_START = 0x0
OFFSET_BOOTLOADER = 0x1000
OFFSET_PARTITIONS = 0x8000
OFFSET_OTA_DATA_INITIAL = 0xd000
OFFSET_APPLICATION = 0x10000
OFFSET_END = 0x190000

def esp32_image_gen(chip_info):
    bin_path_bootloader = os.path.join(chip_info.tools_path, "bootloader.bin")
    bin_path_partitions_table = os.path.join(chip_info.tools_path, "partition-table.bin")
    bin_path_ota_data_init = os.path.join(chip_info.tools_path, "ota_data_initial.bin")
    bin_path_app = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_app.bin")

    # output bin
    chip_info.bin_file_QIO = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_QIO_{chip_info.sketch_version}.bin")

    if not os.path.exists(bin_path_bootloader):
        logging.error(f"{bin_path_bootloader} not found")
        return False
    
    if not os.path.exists(bin_path_partitions_table):
        logging.error(f"{bin_path_partitions_table} not found")
        return False
    
    if not os.path.exists(bin_path_ota_data_init):
        logging.error(f"{bin_path_ota_data_init} not found")
        return False
    
    if not os.path.exists(bin_path_app):
        logging.error(f"{bin_path_app} not found")
        return False
    
    bin_files_in = [
        ("bootloader", OFFSET_BOOTLOADER, bin_path_bootloader),
        ('partitions', OFFSET_PARTITIONS, bin_path_partitions_table),
        ('ota_data_initial', OFFSET_OTA_DATA_INITIAL, bin_path_ota_data_init),
        ('application', OFFSET_APPLICATION, bin_path_app)
    ]

    with open(chip_info.bin_file_QIO, 'wb') as bin_out:
        cur_offset = OFFSET_START
        for name, offset, bin_in in bin_files_in:
            bin_out.write(b'\xff' * (offset - cur_offset))
            cur_offset = offset
            with open(bin_in, 'rb') as bin_in:
                data = bin_in.read()
                bin_out.write(data)
                cur_offset += len(data)
        offset = OFFSET_END
        bin_out.write(b'\xff' * (offset - cur_offset))
        logging.debug(f"ESP32 QIO binary: {chip_info.bin_file_QIO}")
        logging.debug("package success.")
    
    # Copy bin_path_app
    chip_info.bin_file_UA = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_UA_{chip_info.sketch_version}.bin")
    chip_info.bin_file_UG = os.path.join(chip_info.output_path, f"{chip_info.sketch_name}_UG_{chip_info.sketch_version}.bin")
    shutil.copy2(bin_path_app, chip_info.bin_file_UA)
    shutil.copy2(bin_path_app, chip_info.bin_file_UG)

    # remove bin_path_app
    os.remove(bin_path_app)

    return True

def get_qio_binary_esp32(chip_info):
    logging.debug(f"platform system: {platform.system()}")

    if False == esp32_image_gen(chip_info):
        logging.error("esp32_image_gen failed")
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

