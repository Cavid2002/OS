

ASM = nasm
ASMFLAGS = -f bin 
LINKFLAGS = -m


boot: boot.asm
	$(ASM) $(ASMFLAGS) boot.asm -o boot.bin


.PHONY = run
run:
	qemu-system-i386 -hda boot.bin