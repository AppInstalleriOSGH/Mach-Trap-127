SCFLAGS = -arch arm64 -Wall -Wno-error=incompatible-function-pointer-types -Wno-error=int-conversion -c -Os -fno-stack-protector -fno-unwind-tables -fno-exceptions

all: clean
	@xcrun -sdk iphoneos clang main.S -o main.o $(SCFLAGS)
	@xcrun -sdk iphoneos clang main.c -o main-c.o $(SCFLAGS)
	
	@xcrun -sdk iphoneos ld main.o main-c.o -o payload -dylib -lSystem -syslibroot $(shell xcrun --sdk iphoneos --show-sdk-path)
	@objcopy -O binary -j .text payload payload.bin
	@python3 -c "open('payload.h', 'w').write('uint8_t payload[] = {' + ','.join(map(str, open('payload.bin', 'rb').read())) + '};\n')"
	@rm payload *.o

clean:
	@rm -f payload.bin payload.h
