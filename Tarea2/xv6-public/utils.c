#include "utils.h" 

unsigned long next=1;

        // Get a pseudo-random number in the range [0, max)
        int 
        random(int max)
        {
        if (max == 0) return 0;
        next = next * 1103515245 + 12345;
        int rand = ((unsigned)(next/65536) % 32768);
        return (rand % max+1) - 1;
        }