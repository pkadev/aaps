#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memtest.h"
#include "uart.h"

#define REPEAT_NUM_TIMES 4

#define MEM_SIZE 32 * 1024

bool mem_test(void)
{
    uint8_t *mem;
    int i, j, n;
    for (n = 0; n < REPEAT_NUM_TIMES; n++)
    {
        i = 0;
        do
        {
            size_t size = (i + 2) * 4029;
            mem = malloc(size);
            if (mem == NULL) {
                //printk("Alloc %u bytes failed\n", size);
            }
            else
            {
                printk("Alloc %u bytes succeded\n", size);

                memset(mem, 0xAC, size);

                for(j = 0; j < size; j++) {
                    if (mem[j] != 0xAC) {
                        printk("Memory mismatch at %p\n", &(mem[j]));
                        break;
                    } else {
                        //printk("Mem at %p is %x\n", &(mem[j]), mem[j]);
                    }
                }
                memset(mem, 0x00, size);
            }
            i++;
            free(mem);
        } while (mem);
        printk("Done!\n");
    }
    return true;
}
