/**
 * @file project_elf_src.c
 * @brief Project ELF source file for XH_WB5E board
 * @version 2.0
 * @copyright Copyright (c) 2021-2026 Tuya Inc. All Rights Reserved.
 */
volatile const char build_version[]                    = __DATE__ " " __TIME__;
void               *__dso_handle __attribute__((weak)) = 0;