PONGOOS_ROOT = $(HOME)/Desktop/PongoOS

all: build load

build: clean
	@make -C payload
	@xcrun -sdk iphoneos clang --target=arm64-apple-ios12.0 -Wall -O3 -flto -ffreestanding -U__nonnull -nostdlibinc -I$(PONGOOS_ROOT)/newlib/aarch64-none-darwin/include -nostdlib -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z -nostdlib -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z -I$(PONGOOS_ROOT)/apple-include/ -Iinclude/ -I$(PONGOOS_ROOT)/include/ -I$(PONGOOS_ROOT)/src/kernel -I$(PONGOOS_ROOT)/src/drivers -I$(PONGOOS_ROOT)/src/lib -Wl,-kext main.c -o trap_patcher

load:
	@python3 load.py
	
jb:
	@python3 jailbreak.py

clean:
	@rm -f trap_patcher
	@make -C payload clean
