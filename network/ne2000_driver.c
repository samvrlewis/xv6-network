#include "../types.h"
#include "../x86.h"
#include "../defs.h"
#include "ne2000_driver.h"

typedef struct {
  char name[8];
  int irq;
  int base;
  uchar address[6];
} adapter_t;
 
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
    outb(address, CR_STP | CR_NO_DMA | CR_PS_P1);
 
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
 
int init(adapter_t* adapter) {
    // Read reset register to reset, then write to it to clear it
    // https://www.kernel.org/pub/linux/kernel/people/marcelo/linux-2.4/drivers/net/ariadne2.c
    return 0;
 
}
 
int send_data(adapter_t* adapter) {
    return 0;
}
