PREFIX	:= arm-none-eabi
AS	:= $(PREFIX)-as
OBJCOPY	:= $(PREFIX)-objcopy

.PHONY: all clean
all:
	$(AS) -mbig-endian -mcpu=arm926ej-s stub.s -o stub.elf
	$(OBJCOPY) -O binary stub.elf stub.bin
	xxd -i stub.bin ../stub.h
clean:
	rm -rf *.bin *.elf
