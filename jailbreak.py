import sys
import usb.core
import os

dev = usb.core.find(idVendor=0x05ac, idProduct=0x4141)
if dev is None:
    print("Device not found")
    sys.exit(0)
dev.set_configuration()

def send_file(path):
    with open(path, "rb") as f:
        data = f.read()
    dev.ctrl_transfer(0x21, 2, 0, 0, 0)
    dev.ctrl_transfer(0x21, 1, 0, 0, 0)
    dev.write(2, data)
    
def issue_command(command):
    try:
        dev.ctrl_transfer(0x21, 4, 0, 0, 0)
        dev.ctrl_transfer(0x21, 3, 0, 0, command)
    except Exception:
        pass

# jailbreak setup
issue_command("fuse lock\n")
issue_command("sep auto\n")
send_file("/Users/benjamin/Downloads/legacy_kpf") # https://github.com/kok3shidoll/ra1npoc/blob/ios15/headers/legacy_kpf
issue_command("modload\n")
send_file("/Users/benjamin/Downloads/legacy_ramdisk") # https://github.com/kok3shidoll/ra1npoc/blob/ios15/headers/legacy_ramdisk
issue_command("ramdisk\n")
issue_command("kpf_flags 0x2\n");
issue_command("xargs -v rootdev=md0\n")

# run patcher
send_file("trap_patcher")
issue_command("modload\n")
issue_command("trap_patch\n")

# boot
issue_command("bootx\n")
