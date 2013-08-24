#include <avr/io.h>
//TODO: adjust these as needed
FUSES = 
    {
        .low = LFUSE_DEFAULT | ~FUSE_CKDIV8,
        .high = HFUSE_DEFAULT & FUSE_EESAVE,
        .extended = EFUSE_DEFAULT,
    };