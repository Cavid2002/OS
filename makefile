ASM = nasm
CC = gcc

CFLAGS = -ffreestanding -m32 -masm=intel -c -nostdlib -fno-pie
LDFLAGS = -m elf_i386 -T ./kernel/linker.ld --oformat=binary

OBJS = ./bin/entry.o ./bin/kernel.o ./bin/VGA.o \
		./bin/interrupt.o ./bin/PIC.o ./bin/portio.o \
		./bin/interrupt_wrapper.o

os.bin: ./bin/boot.bin ./bin/kernel.bin ./bin/mbr.bin
	cat ./bin/mbr.bin ./bin/boot.bin ./bin/kernel.bin > ./os.bin
	truncate --size 5M ./os.bin

./bin/kernel.bin: $(OBJS)
	ld $(LDFLAGS) $(OBJS) -o ./bin/kernel.bin

./bin/boot.bin: ./asm/boot.asm
	mkdir -p ./bin
	$(ASM) -f bin ./asm/boot.asm -o ./bin/boot.bin

./bin/mbr.bin: ./asm/mbr.asm
	$(ASM) -f bin ./asm/mbr.asm -o ./bin/mbr.bin

./bin/kernel.o: ./kernel/kernel.c
	$(CC) $(CFLAGS) ./kernel/kernel.c -o ./bin/kernel.o


./bin/VGA.o: ./kernel/VGA.c ./include/VGA.h
	$(CC) $(CFLAGS) ./kernel/VGA.c -o ./bin/VGA.o

./bin/entry.o: ./asm/entry.asm
	$(ASM) -f elf32 ./asm/entry.asm -o ./bin/entry.o 


./bin/portio.o: ./asm/portio.asm
	$(ASM) -f elf32 ./asm/portio.asm -o ./bin/portio.o


./bin/interrupt.o: ./include/interrupt.h ./kernel/interrupt.c
	$(CC) $(CFLAGS) ./kernel/interrupt.c -o ./bin/interrupt.o


./bin/PIC.o: ./include/PIC.h ./kernel/PIC.c
	$(CC) $(CFLAGS) ./kernel/PIC.c -o ./bin/PIC.o


./bin/interrupt_wrapper.o: ./asm/interrupt_wrapper.asm
	$(ASM) -f elf32 ./asm/interrupt_wrapper.asm -o ./bin/interrupt_wrapper.o

.PHONY: run clean

run:
	qemu-system-i386 -hda ./os.bin

clean:
	rm ./bin/* ./os.bin

dasm-32:
	ndisasm -b 32 os.bin > bit32.txt

dasm-16:
	ndisasm -b 16 os.bin > bit16.txt