#coding=utf-8

import os
import sys

current_path = os.path.dirname(os.path.abspath(__file__))
current_path = os.path.normpath(current_path)

compiler_path = "{runtime.tools.gcc-arm-none-eabi.path}"

class board_info_class():
    def __init__(self, name, chip, vendor_name, product_name, variant, upload_baud_rate_list, toolchain=compiler_path, toolchain_pre="arm-none-eabi-", core="tuya_open"):
        self.name = name # board name in boards.txt
        self.chip = chip # chip name
        self.vendor_name = vendor_name # board manufacturer name
        self.product_name = product_name # board name
        self.variant = variant # variant name
        self.toolchain = toolchain # toolchain
        self.toolchain_pre = toolchain_pre # toolchain prefix
        self.core = core

        # upload baud rate
        self.upload_baud_rate_list = upload_baud_rate_list

        # arduino ide display name
        self.board_name_ide = ""
        if self.vendor_name == "" :
            self.board_name_ide = product_name
        else:
            self.board_name_ide = vendor_name + " " + product_name

        # arduino ide board name
        # code use define in arduino
        self.board = ""
        if self.vendor_name == "":
            self.board = product_name.replace(" ", "_")
        else:
            self.board = vendor_name.replace(" ", "_") + "_" + product_name.replace(" ", "_")

def build_header(bi):
    print()
    print(f"# --------------------------------------------------")
    print(f"# {bi.board_name_ide.center(50)}")
    print(f"# --------------------------------------------------")
    print(f"{bi.name}.name={bi.board_name_ide}")
    print(f"{bi.name}.build.chip={bi.chip}")
    print(f"{bi.name}.build.board={bi.board}")
    print(f"{bi.name}.build.core={bi.core}")
    print(f"{bi.name}.build.variant={bi.variant}")

def build_toolchain(bi):
    print()
    print(f"{bi.name}.compiler.path={bi.toolchain}")
    print(f"{bi.name}.compiler.cmd.S={{compiler.path}}/bin/{bi.toolchain_pre}gcc")
    print(f"{bi.name}.compiler.cmd.c={{compiler.path}}/bin/{bi.toolchain_pre}gcc")
    print(f"{bi.name}.compiler.cmd.cpp={{compiler.path}}/bin/{bi.toolchain_pre}g++")
    print(f"{bi.name}.compiler.cmd.ar={{compiler.path}}/bin/{bi.toolchain_pre}ar")
    if bi.chip == "t2" or bi.chip == "t3":
        print(f"{bi.name}.compiler.cmd.ld={{compiler.path}}/bin/{bi.toolchain_pre}gcc")
    else:
        print(f"{bi.name}.compiler.cmd.ld={{compiler.path}}/bin/{bi.toolchain_pre}g++")

def build_flags(bi):
    print()
    # vendor path
    vendor_path = "{runtime.tools.vendor-" + bi.chip + ".path}"
    print(f"{bi.name}.vendor.path={vendor_path}")
    # build flags
    print(f"{bi.name}.compiler.flags.S=@{vendor_path}/flags/S_flags.txt")
    print(f"{bi.name}.compiler.flags.c=@{vendor_path}/flags/c_flags.txt")
    print(f"{bi.name}.compiler.flags.cpp=@{vendor_path}/flags/cpp_flags.txt")
    print(f"{bi.name}.compiler.flags.ar=@{vendor_path}/flags/ar_flags.txt")

    # sketch define flags(such as sketch version)
    print()
    print(f"{bi.name}.appBuild.flags=@{{build.path}}/tuyaTmp/appConfig/appBuildDefine.txt")

    # include flags
    # tuya open sdk includes
    print()
    print(f"{bi.name}.compiler.includes.tuya_open=-iprefix {vendor_path}/ @{vendor_path}/flags/include_tuya_open.txt")
    # tuya adapter includes
    print(f"{bi.name}.compiler.includes.tkl=-iprefix {vendor_path}/ @{vendor_path}/flags/include_tkl.txt")
    # chip vendor includes
    print(f"{bi.name}.compiler.includes.vendor=-iprefix {vendor_path}/ @{vendor_path}/flags/include_vendor.txt")
    print(f"{bi.name}.compiler.includes={{compiler.includes.tuya_open}} {{compiler.includes.tkl}} {{compiler.includes.vendor}}")

    ## combine
    print()
    print(f"{bi.name}.compiler.flags.ld=@{vendor_path}/flags/ld_flags.txt")
    if bi.chip == 'esp32':
        print(f"{bi.name}.compiler.flags.libs=-L{vendor_path}/libs -L{vendor_path}/link_path/tuya_open_sdk/build/esp-idf/esp_system/ld -L{vendor_path}/link_path/esp-idf/components/bt/controller/lib_esp32/esp32 -L{vendor_path}/link_path/esp-idf/components/esp_coex/lib/esp32 -L{vendor_path}/link_path/esp-idf/components/esp_phy/lib/esp32 -L{vendor_path}/link_path/esp-idf/components/esp_rom/esp32/ld -L{vendor_path}/link_path/esp-idf/components/esp_wifi/lib/esp32 -L{vendor_path}/link_path/esp-idf/components/soc/esp32/ld @{vendor_path}/flags/libs_flags.txt")
        print(f"{bi.name}.compiler.flags.ld_scripts=-Wl,--cref -Wl,-Map={{build.path}}/{{build.project_name}}.map -o {{build.path}}/{{build.project_name}}.elf")
    else:
        print(f"{bi.name}.compiler.flags.libs=-L{vendor_path}/libs @{vendor_path}/flags/libs_flags.txt")
        ld_scripts_path = vendor_path + "/packager-tools/" + bi.chip + ".ld"
        print(f"{bi.name}.compiler.flags.ld_scripts=-Wl,--cref -Wl,-Map={{build.path}}/{{build.project_name}}.map -T{ld_scripts_path} -o {{build.path}}/{{build.project_name}}.elf")

def build_upload(bi):
    print()
    print(f"{bi.name}.upload.tool=tyutool")
    print(f"{bi.name}.upload.tool.default=tyutool")

def build_monitor(bi):
    print()
    print(f"{bi.name}.monitor_port.serial.baudrate=115200")

def build_menu_head():
    print(f"menu.UploadSpeed=Upload Speed")

def build_menu(bi):
    print()
    for baud_rate in bi.upload_baud_rate_list:
        print(f"{bi.name}.menu.UploadSpeed.{baud_rate}={baud_rate}")
        print(f"{bi.name}.menu.UploadSpeed.{baud_rate}.upload.speed={baud_rate}")

def make_board(board_info):

    build_header(board_info)
    build_menu(board_info)
    build_toolchain(board_info)
    build_flags(board_info)
    build_upload(board_info)
    build_monitor(board_info)

    return True

if __name__ == "__main__":
    # print() redirect to a file
    board_file = os.path.join(current_path, "../" "boards.txt")
    board_file = os.path.normpath(board_file)
    sys.stdout = open(board_file, "w", newline='\n')

    build_menu_head()

    t2_upload_speed_list = [921600, 115200, 230400, 460800, 1500000, 2000000]
    t2_bi = board_info_class("t2", "t2", "", "T2", "t2", t2_upload_speed_list, compiler_path, "arm-none-eabi-", "tuya_open")
    make_board(t2_bi)

    t3_upload_speed_list = [921600, 115200, 230400, 460800, 1500000, 2000000]
    t3_bi = board_info_class("t3", "t3", "", "T3", "t3", t3_upload_speed_list, compiler_path, "arm-none-eabi-", "tuya_open")
    make_board(t3_bi)

    t5_upload_speed_list = [921600, 115200, 230400, 460800, 1500000, 2000000]
    t5_bi = board_info_class("t5", "t5", "", "T5", "t5", t5_upload_speed_list, compiler_path, "arm-none-eabi-", "tuya_open")
    make_board(t5_bi)

    ln882h_upload_speed_list = [921600, 115200, 230400, 460800, 1500000, 2000000]
    ln882h_bi = board_info_class("ln882h", "ln882h", "", "LN882H", "ln882h", ln882h_upload_speed_list, compiler_path, "arm-none-eabi-", "tuya_open")
    make_board(ln882h_bi)

    esp32_upload_speed_list = [921600, 115200, 230400, 460800, 1500000, 2000000]
    esp_bi = board_info_class("esp32", "esp32", "", "ESP32", "esp32", esp32_upload_speed_list, "{runtime.tools.xtensa-esp-elf.path}", "xtensa-esp32-elf-", "tuya_open")
    make_board(esp_bi)
