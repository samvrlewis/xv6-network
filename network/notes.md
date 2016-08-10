# System calls
* System call allows a user program to ask for an OS service.
* Kernel handles all interrupts but needs to maintain isolation of user processes and the kernel.
* Upon an interrupt, system must save all processor registers.
* xv6 and conventional unix terminology for interrupts is called 'traps'. Traps are caused by processors and interrupts called by devices.

## Protection
* Four protection levels 0 (most privilege) to 3 (least privilege)
* In practice, only 0 (kernel mode) and 3 (user mode) used by most OSs
* Privilege level stored in %cs register in CPL (current protection level & DPL is descriptor privilege level) field
* xv6 has interrupt descriptor table, each entry has %cs and %eip to be used

# Drivers
* Early PC motherboards had a programmable interrupt controller (PIC) - look at piciqr.c
* Each PIC handles 8 devices, PICs can be cascaded to handle more than 8
* ideinit in ide.c shows a good way of starting the IDE driver
  * Need to call picenable (for single processor) and ioapicenable (for multiprocessor) to enable the IRQ interrupt
* After enabling interrupts, need to probe the hardware

# Code notes
* Need to add ethernet to the devsw array, this seems to add it to something that allows the fs to read and write from the device as if it were a unix file
* Probe to find the address the device is attached to and then init afterwards

# Useful references
* https://github.com/s-shin/xv6-network - Driver for the NE2000 (documented mainly in Japanese) and somewhat integrated into the xv6 system
* https://github.com/phulin/xv6-lwip-capabilities - Builds on the NE2000 driver and adds the lwIP library as a TCP/IP stack
* http://www.ti.com/general/docs/lit/getliterature.tsp?genericPartNumber=dp8390d&fileType=pdf - The NE2000 is a standard that several implementors follow - National Semiconductors (acquired by TI) produced the initial prototype design that the NE2000 is based on and this document is a good description of the initial standard and has explanation around how to interface with the card. Necessary for writing drivers for the card.
* http://wiki.osdev.org/Ne2000 - Information about writing drivers for the NE2000
* http://www.osdever.net/documents/WritingDriversForTheDP8390.pdf - More information about writing NE2000 drivers
* http://wiki.osdev.org/Network_Stack - High level information about implementing a network stack, from drivers to higher level protocols
* http://savannah.nongnu.org/projects/lwip/ - LwIP provides a TCP/IP stack, providing (among others) TCPIP, UDP, TCP, DNS, DHCP, ARP
* http://lwip.wikia.com/wiki/LwIP_Wiki - LwIP information and guides about using the library
* http://antoinealb.net/programming/2013/12/06/lwip-ucos2.html - Article about porting LWiP for a platform, including writing a custom sys_arch.c

# Steps
A rough plan of how to proceed

## Adding an ethernet interface  
1. Write NE2000 driver code for probing and initialising (look at disk.c for how to read/write from registers)
2. Integrate the driver code into xv6
  1. Add ethernet code to trap.c for handling ethernet interrupts
  2. Init the eth file descriptor (init.c) and define the file descriptor number (file.h)
  3. Dubious that the ioctl functionality needs to be implemented - this only seems to be necessary for be able to issue extra commands. Need to check this.
3. Test to make sure the mac address can come from the card
4. Write driver code for sending and receiving packets
5. Should hopefully be able to make a DHCP request and get an IP address back now by manually constructing an ethernet packet

## Adding lwip
1. Write xv6 sys_arch.c (and cc.h etc) for lwip
2. Write projecthif.c?

## Ring buffers
* Two ring buffers to handle packets - each made up of 256 byte pages. 
* Packets always start on page boundaries
* Two registers:
  * PSTART/PSTOP define the physical boundaries of the ring buffer
  * Whenever the DMA address reaches stop, it resets to start
* Ring buffer is in physical continuous memory
* Data comes into receive FIFO then moved to receive buffer when the early receive threshold is met
* Current Page Register acts as a Write Pointer and the Boundary Pointer acts as a Read Pointer 
* The addressable DMA space seems to be 0x4000 to 0x7fff for a NE2000 card operating in 16 bit mode but this doesn't appear to really be documented in the datasheet. It is in the datasheets for some NE2000 'clones' (http://www.davicom.com.tw/userfile/24247/DM9008-DS-F02-930914.pdf) and is widely used in driver code. 
  * PSTART and PSTOP only have the most significant 8 bits of the 16 bit address written to so this equates to 0x40 to 0x80 (as the stop address)
  * Seems usual to give a few pages (~6?) for the transmit buffer and to save the rest as a receive buffer