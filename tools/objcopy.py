#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import shutil
import logging
import argparse
import platform
import subprocess
import json

toolspath = os.path.dirname(os.path.realpath(__file__))
toolspath = os.path.normpath(toolspath)

sys.path.insert(0, os.path.join(toolspath))
import sketch_version
import fileutils
import packager

def log_config(level=logging.INFO):
    # FORMAT = '[%(levelname)-8s][%(asctime)s][%(filename)s:%(lineno)d]%(message)s'
    FORMAT = '[%(levelname)-4s][%(asctime)s]%(message)s'
    logging.basicConfig(
        level=level,
        format=FORMAT,
        datefmt='%H:%M:%S'
    )

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--chip", type=str, required=True, help="chip type")
    parser.add_argument("--board", type=str, required=True, help="board name")
    parser.add_argument("--compiler-path", type=str, required=True, help="path to compiler tool path")
    parser.add_argument("--vendor-path", type=str, required=True, help="path to chip vendor")
    parser.add_argument("--build-path", type=str, required=True, help="path to build")
    parser.add_argument("--sketch-name", type=str, required=True, help="sketch name")
    parser.add_argument("-v", "--verbose", action='store_true', help="verbose output")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    if args.verbose:
        log_config(logging.DEBUG)
    else:
        log_config()

    logging.info(f"Platform: {platform.system()}")

    compiler_path = os.path.normpath(args.compiler_path)
    vendor_path = os.path.normpath(args.vendor_path)
    build_path = os.path.normpath(args.build_path)

    logging.debug(f"Chip type: {args.chip}")
    logging.debug(f"Board name: {args.board}")
    logging.debug(f"Sketch name: {args.sketch_name}")
    logging.debug(f"Compiler_path: {compiler_path}")
    logging.debug(f"Vendor path: {vendor_path}")
    logging.debug(f"Build path: {build_path}")

    sketch_version_file = os.path.join(build_path, 'tuyaTmp', 'appConfig', 'appConfig.json')
    sketch_version = sketch_version.sketch_version_parse(sketch_version_file)
    output_path = os.path.join(build_path, 'tuyaTmp', 'output', sketch_version)
    logging.debug(f"Sketch version: {sketch_version}")

    fileutils.delete_if_exists(output_path)
    os.makedirs(output_path)
    if not os.path.exists(output_path):
        logging.error(f"Failed to create output path: {output_path}")
        sys.exit(1)

    # copy .elf, .map file to output path
    elf_file = os.path.join(build_path, f"{args.sketch_name}.elf")
    map_file = os.path.join(build_path, f"{args.sketch_name}.map")
    shutil.copy2(elf_file, output_path)
    shutil.copy2(map_file, output_path)

    tools_path = os.path.join(vendor_path, 'packager-tools')
    packager = packager.packager_class(args.chip, args.board, compiler_path, tools_path, output_path, args.sketch_name, sketch_version)

    if not packager.elf_to_binary():
        sys.exit(1)
    logging.debug("elf_to_binary success")

    if not packager.get_qio_binary():
        sys.exit(1)
    logging.debug("get_qio_binary success")

    arduino_upload_file = os.path.join(build_path, f"{args.sketch_name}_QIO.bin")
    if not packager.get_arduino_upload_binary(os.path.join(output_path, arduino_upload_file)):
        logging.error("get_arduino_upload_binary failed")
        sys.exit(1)
    logging.debug("get_arduino_upload_binary success")

    sys.exit(0)
