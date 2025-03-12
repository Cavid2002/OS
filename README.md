# HeliOS: 32-bit Operating system #

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

## The Reader ##

Most well-known books on operating systems focus primarily on theoretical concepts and ideas related to existing environments, without offering practical guidance on how to create an operating system from scratch. While resources like [osdev.org](https://wiki.osdev.org/) provide valuable information, they often assume a certain level of prior knowledge, which can make them challenging for beginners. This document aims to fill that gap by offering a detailed, step-by-step explanation of each component of the operating system as development progresses, making it easier for readers to understand and implement the concepts themselves.


