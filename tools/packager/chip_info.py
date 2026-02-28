#coding=utf-8
# packager/chip_info.py
import logging

class chip_info_class:
    def __init__(self, chip, board, compiler_path, tools_path, output_path, sketch_name, sketch_version):
        self.chip = chip
        self.board = board
        self.compiler_path = compiler_path
        self.tools_path = tools_path
        self.output_path = output_path
        self.sketch_name = sketch_name
        self.sketch_version = sketch_version
        self.bin_file = None
        self.bin_file_QIO = None
        self.bin_file_UA  = None
        self.bin_file_UG  = None

    def print_info(self):
        logging.debug(f"chip: {self.chip}")
        logging.debug(f"board: {self.board}")
        logging.debug(f"compiler_path: {self.compiler_path}")
        logging.debug(f"tools_path: {self.tools_path}")
        logging.debug(f"output_path: {self.output_path}")
        logging.debug(f"sketch_name: {self.sketch_name}")
        logging.debug(f"sketch_version: {self.sketch_version}")
        logging.debug(f"bin_file: {self.bin_file}")
        logging.debug(f"bin_file_QIO: {self.bin_file_QIO}")
        logging.debug(f"bin_file_UA: {self.bin_file_UA}")
        logging.debug(f"bin_file_UG: {self.bin_file_UG}")



