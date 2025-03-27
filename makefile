ASM = nasm
CC = gcc

CFLAGS = -ffreestanding -m32 -masm=intel -c -nostdlib -fno-pie
LDFLAGS = -m elf_i386 -T ./kernel/linker.ld --oformat=binary

OBJS = ./bin/entry.o ./bin/kernel.o ./bin/vga.o

os.bin: ./bin/boot.bin ./bin/kernel.bin
	cat ./bin/boot.bin ./bin/kernel.bin > ./os.bin
	truncate --size 1M ./os.bin


./bin/boot.bin: ./asm/boot.asm
	mkdir -p ./bin
	$(ASM) -f bin ./asm/boot.asm -o ./bin/boot.bin


./bin/kernel.o: ./kernel/kernel.c
	$(CC) $(CFLAGS) ./kernel/kernel.c -o ./bin/kernel.o


./bin/vga.o: ./kernel/vga.c ./include/vga.h
	$(CC) $(CFLAGS) ./kernel/vga.c -o ./bin/vga.o

./bin/entry.o: ./asm/entry.asm
	$(ASM) -f elf32 ./asm/entry.asm -o ./bin/entry.o 

./bin/kernel.bin: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o ./bin/kernel.bin


.PHONY: run clean

run:
	qemu-system-i386 -hda ./os.bin

clean:
	rm ./bin/* ./os.bin