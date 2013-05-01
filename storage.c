#include <avr/io.h>
#include <stdbool.h>
#include "uart.h"
#include "storage.h"

int card_detect(void)
{
    static bool card_inserted = false;

    if ((PINJ & (1<<PJ3))) {
       if (card_inserted)
           printk("µSD card ejected\n");
       card_inserted = false;
    }
    if (!(PINJ & (1<<PJ3))) {
        if (!card_inserted)
            printk("µSD card inserted\n");
        card_inserted = true;
    }
    return 0;
}
