ASM = nasm
CC = gcc

CFLAGS = -ffreestanding -m32 -masm=intel -c -nostdlib -fno-pie -fno-stack-protector
LDFLAGS = -static -m elf_i386 -T ./boot/linker.ld

OBJS = ./bin/entry.o ./bin/boot.o ./bin/VGA.o \
		./bin/interrupt.o ./bin/PIC.o ./bin/portio.o \
		./bin/interrupt_wrapper.o ./bin/printf.o \
		./bin/PS2.o \
		./bin/memory.o ./bin/delay.o ./bin/ATAPIO.o \
		./bin/string.o ./bin/ext2.o

bootloader.bin: ./bin/init.bin ./bin/boot.bin ./bin/mbr.bin
	cat ./bin/mbr.bin ./bin/init.bin ./bin/boot.bin > ./bootloader.bin
	cp ./bootloader.bin ./bootloader.img
	truncate -s 6G ./bootloader.img

./bin/boot.bin: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o ./bin/boot.bin

./bin/init.bin: ./asm/init.asm
	mkdir -p ./bin
	$(ASM) -f bin ./asm/init.asm -o ./bin/init.bin

./bin/mbr.bin: ./asm/mbr.asm
	$(ASM) -f bin ./asm/mbr.asm -o ./bin/mbr.bin

./bin/boot.o: ./boot/boot.c
	$(CC) $(CFLAGS) ./boot/boot.c -o ./bin/boot.o


./bin/VGA.o: ./boot/VGA.c ./include/VGA.h
	$(CC) $(CFLAGS) ./boot/VGA.c -o ./bin/VGA.o

./bin/entry.o: ./asm/entry.asm
	$(ASM) -f elf32 ./asm/entry.asm -o ./bin/entry.o 


./bin/portio.o: ./asm/portio.asm
	$(ASM) -f elf32 ./asm/portio.asm -o ./bin/portio.o


./bin/interrupt.o: ./include/interrupt.h ./boot/interrupt.c
	$(CC) $(CFLAGS) ./boot/interrupt.c -o ./bin/interrupt.o


./bin/PIC.o: ./include/PIC.h ./boot/PIC.c
	$(CC) $(CFLAGS) ./boot/PIC.c -o ./bin/PIC.o


./bin/interrupt_wrapper.o: ./asm/interrupt_wrapper.asm
	$(ASM) -f elf32 ./asm/interrupt_wrapper.asm -o ./bin/interrupt_wrapper.o

./bin/printf.o: ./include/printf.h ./boot/printf.c
	$(CC) $(CFLAGS) ./boot/printf.c -o ./bin/printf.o

./bin/memory.o: ./include/memory.h ./boot/memory.c
	$(CC) $(CFLAGS) ./boot/memory.c -o ./bin/memory.o

./bin/delay.o: ./include/delay.h ./boot/delay.c
	$(CC) $(CFLAGS) ./boot/delay.c -o ./bin/delay.o

./bin/ATAPIO.o: ./include/ATAPIO.h ./boot/ATAPIO.c
	$(CC) $(CFLAGS) ./boot/ATAPIO.c -o ./bin/ATAPIO.o

./bin/ext2.o: ./include/ext2.h ./include/MBR.h ./boot/ext2.c
	$(CC) $(CFLAGS) ./boot/ext2.c -o ./bin/ext2.o

./bin/PS2.o: ./include/PS2.h ./boot/PS2.c
	$(CC) $(CFLAGS) ./boot/PS2.c -o ./bin/PS2.o

./bin/string.o: ./include/string.h ./boot/string.c
	$(CC) $(CFLAGS) ./boot/string.c -o ./bin/string.o



.PHONY: run clean dasm-32 dasm-16

run:
	qemu-system-i386 -drive file=bootloader.img,format=raw,if=ide,snapshot=off

clean:
	rm -f ./bin/* ./bootloader.bin ./bootloader.img

dasm-32:
	ndisasm -b 32 bootloader.bin > bit32.txt

dasm-16:
	ndisasm -b 16 bootloader.bin > bit16.txt