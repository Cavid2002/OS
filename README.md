# HeliOS: 32-bit Operating system #

## Dependecies ##
- **GCC(GNU C Complier)**
- **NASM(Netwide Assembler)**

## Introduction: ##
HeliOS is a 32-bit operating system kernel developed in C and Assembly for the x86 architecture. It forms the central focus of a master's thesis at ADA University and is designed as an educational tool for Computer Architecture and Operating Systems courses. The kernel is structured around the following key components:

- Bootloader: Initializes the system and loads the kernel into memory.
- Terminal Screen: Provides a basic text interface for user interaction.
- Interrupt/Trap Handlers: Manages hardware interrupts and system traps for efficient handling of exceptional conditions.
- Keyboard Driver: Facilitates user input by interpreting signals from the keyboard.
- IDE HDD Driver: Enables access to files stored on an IDE hard drive.
- Virtual Memory: Implements memory management techniques for efficient use of available system resources.
- Multitasking/Scheduling: Manages multiple processes, allocating CPU time and controlling process execution.

Each component is designed to provide students and developers with hands-on experience in understanding key principles of operating system design and computer architecture. The target architecture is choosen to be x86 as this architecture is the dominant in PC market. 

## Important Notice ##

Project is at the stage of implementing 2-stage bootlader which will be able to locate the kernel binary in filesystem and pass the control to it.

## Building and Running ##

To compile and get the binary image just run
```
make 
```
It would generate os.img file that can be loaded to any disk using the following command:
```
sudo cat os.img > /dev/[block_device]
```
**Optionally** image can be run via **QEMU** virtual machine:

```
make run 
```

