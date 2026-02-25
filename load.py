import sys
import usb.core
import time
import os

data = open("trap_patcher", "rb").read()
dev = usb.core.find(idVendor=0x05ac, idProduct=0x4141)
if dev is None:
    print("Device not found")
    sys.exit(0)
dev.set_configuration()
dev.ctrl_transfer(0x21, 2, 0, 0, 0)
dev.ctrl_transfer(0x21, 1, 0, 0, 0)
dev.write(2, data, 100000)
if len(data) % 512 == 0:
	dev.write(2,"")
dev.ctrl_transfer(0x21, 3, 0, 0, "modload\n")
dev.ctrl_transfer(0x21, 3, 0, 0, "trap_patch\n")
time.sleep(0.1)
print("".join(chr (x) for x in dev.ctrl_transfer(0xa1, 1, 0, 0, 512)))
try:
    dev.ctrl_transfer(0x21, 3, 0, 0, "bootx\n")
except Exception:
    pass
os._exit(0)
