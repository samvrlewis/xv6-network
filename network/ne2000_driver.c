#include "../types.h"
#include "../x86.h"
#include "../defs.h"
#include "ne2000_driver.h"
 
// Look to see if there's an NE2000 adapter at this address
 
//https://github.com/swetland/openblt/blob/master/netboot/ne2000.c
 
 
// To check if there's a NE2000 (or compatible card) attached on this address,
// use the multi-page command property of the device to verify. This is done by
// changing to command page 1, writing to the MAR5 register, changing to page 0 
// and then reading from the CNTR0 register (same address as MAR5) twice. 
//
// A NE2000 card should have an empty CNTR0 register as reading it clears it, 
// but any other device hopefully should have something in the register as we
// wrote to the same address before changing command pages.
int ne2k_probe(int address) {
    if(inb(address) == 0xFF)
    {
        return 0;
    }

    // Save the command register
    int command_state = inb(address);
 
    // Stop the card, set no DMA and choose page 1 commands
    outb(address + DP_CR, CR_STP | CR_NO_DMA | CR_PS_P1);
 
    // Save the mar5 register and then set it to 0xFF
    int mar5_state = inb(address + DP_MAR5); 
    outb(address + DP_MAR5, 0xFF); 
 
    // Go back to page 0
    outb(address + DP_CR, CR_NO_DMA | CR_PS_P0); 
     
    // Read CNTR0, which clears the register
    inb(address + DP_CNTR0);
 
    // Read CNTR0 again, if there's something in it then the device at this 
    // address isn't behaving as a NE2000 should
    if (inb(address + DP_CNTR0) != 0) {
        // Restore the registers that as been modified
        outb(address, command_state);
        outb(address + DP_MAR5, mar5_state);
        return 0;
    }
 
    return 1;
}
 
int ne2k_init(adapter_t* adapter) {
     cprintf("\n processing %x ",  adapter->base);
    // Read reset register to reset, then write to it to clear it
    // https://www.kernel.org/pub/linux/kernel/people/marcelo/linux-2.4/drivers/net/ariadne2.c
    //outb(adapter->base + DP_CR, 0x00); 

    // 1) Set to 0x21 as per the TI init seq
    outb(adapter->base + DP_CR, CR_STP | CR_NO_DMA | CR_PS_P0); 
    
    // 2) Init DCR, todo: check if it's 16 bit?
    outb(adapter->base + DP_DCR, DCR_BYTEWIDE | DCR_LTLENDIAN | DCR_BMS | DCR_2BYTES);
    
    // 3) Clear the remote byte registers
    outb(adapter->base + DP_RBCR0, 0x00);
    outb(adapter->base + DP_RBCR1, 0x00);
    
    // 4) Initialize receive config register
    outb(adapter->base + DP_RCR, RCR_SEP | RCR_AR | RCR_AB | RCR_AM | RCR_PRO);
   
    // 5) put nic loopback mode 1 (internal loopback)
    outb(adapter->base + DP_TCR, TCR_INTERNAL);

    // 6) init receive buffer ring (BNDRY), Page Start (PSTART), and Page Stop (PSTOP)
    outb(adapter->base + DP_PSTART, 0x40);
    outb(adapter->base + DP_PSTOP, 0x80);
    outb(adapter->base + DP_BNRY, 0x40);

    // 7) clear ISR
    outb(adapter->base + DP_ISR, 0xFF);

    // 8) initialize interrupt mask register
    outb(adapter->base + DP_IMR, IMR_PRXE);

    // 9) swap command register for page 1
    outb(adapter->base + DP_CR, CR_STP | CR_NO_DMA | CR_PS_P1);

    // 9i) Init physical address registers
    //outb(adapter->base + DP_PAR0, )
    
    return 0;
 
}

void ne2k_readmem(adapter_t* adapter, int address, int size) {
    outb(adapter->base + DP_CR, inb(adapter->base + DP_CR) | CR_PS_P0); 

    outb(adapter->base + DP_RSAR0, address);
    outb(adapter->base + DP_RSAR1, address >> 8);

    outb(adapter->base + DP_RBCR0, size);
    outb(adapter->base + DP_RBCR1, size >> 8);

    outb(adapter->base + DP_CR, CR_PS_P0 | CR_DM_RR | CR_STA);

    for (int i = 0; i < size; i++) {
        // If we are operating in byte mode (in the DCR register)
        // we need to read each byte that comes back twice as it is
        // duplicated. However, if in word mode - only need to read
        // once. There's some info on page 11 about this. '
        
        cprintf("%x", inb(adapter->base + 0x10));
        cprintf("%x", inb(adapter->base + 0x10));
    }
}
 
int send_data(adapter_t* adapter) {
    return 0;
}