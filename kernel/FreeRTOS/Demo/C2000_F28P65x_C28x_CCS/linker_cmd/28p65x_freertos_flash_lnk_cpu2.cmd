MEMORY
{
   BEGIN            : origin = 0x0E0000, length = 0x000002  // Update the codestart location as needed

   BOOT_RSVD        : origin = 0x000002, length = 0x0001AF     /* Part of M0, BOOT rom will use this for stack */
   RAMM0            : origin = 0x0001B1, length = 0x00024F
   RAMM1            : origin = 0x000400, length = 0x000400

   RAMD2            : origin = 0x008000, length = 0x002000  // Can be mapped to either CPU1 or CPU2. When configured to CPU1, use the address 0x01A000. User should comment/uncomment based on core selection
   RAMD3            : origin = 0x00A000, length = 0x002000  // Can be mapped to either CPU1 or CPU2. When configured to CPU1, use the address 0x01C000. User should comment/uncomment based on core selection
   // RAMD4         : origin = 0x00C000, length = 0x002000  // Can be mapped to either CPU1 or CPU2. When configured to CPU1, use the address 0x01E000. User should comment/uncomment based on core selection
   // RAMD5         : origin = 0x00E000, length = 0x002000  // Can be mapped to either CPU1 or CPU2. When configured to CPU1, use the address 0x020000. User should comment/uncomment based on core selection

   RAMGS0           : origin = 0x010000, length = 0x002000
   RAMGS1           : origin = 0x012000, length = 0x002000
   RAMGS2           : origin = 0x014000, length = 0x002000
   RAMGS3           : origin = 0x016000, length = 0x002000
   RAMGS4           : origin = 0x018000, length = 0x002000

   /* Flash Banks (128 sectors each) */
   // FLASH_BANK0     : origin = 0x080000, length = 0x20000  // Can be mapped to either CPU1 or CPU2. User should comment/uncomment based on core selection
   // FLASH_BANK1     : origin = 0x0A0000, length = 0x20000  // Can be mapped to either CPU1 or CPU2. User should comment/uncomment based on core selection
   // FLASH_BANK2     : origin = 0x0C0000, length = 0x20000  // Can be mapped to either CPU1 or CPU2. User should comment/uncomment based on core selection
   FLASH_BANK3     : origin = 0x0E0002, length = 0x1FFFE  // Can be mapped to either CPU1 or CPU2. User should comment/uncomment based on core selection
   FLASH_BANK4     : origin = 0x100000, length = 0x20000  // Can be mapped to either CPU1 or CPU2. User should comment/uncomment based on core selection


   CPU1TOCPU2RAM    : origin = 0x03A000, length = 0x000800
   CPU2TOCPU1RAM    : origin = 0x03B000, length = 0x000800

   CLATOCPURAM      : origin = 0x001480,   length = 0x000080
   CPUTOCLARAM      : origin = 0x001500,   length = 0x000080
   CLATODMARAM      : origin = 0x001680,   length = 0x000080
   DMATOCLARAM      : origin = 0x001700,   length = 0x000080

   CANA_MSG_RAM     : origin = 0x049000, length = 0x000800
   CANB_MSG_RAM     : origin = 0x04B000, length = 0x000800
   RESET            : origin = 0x3FFFC0, length = 0x000002
}


SECTIONS
{
   codestart        : > BEGIN
   .text            : >> FLASH_BANK3 | FLASH_BANK4, ALIGN(8)
   .cinit           : > FLASH_BANK3, ALIGN(8)
   .switch          : > FLASH_BANK3, ALIGN(8)
   .reset           : > RESET, TYPE = DSECT /* not used, */

   .stack           : > RAMM1
#if defined(__TI_EABI__)
   .bss             : > RAMGS4
   .bss:output      : > RAMGS4
   .init_array      : > FLASH_BANK3, ALIGN(8)
   .const           : > FLASH_BANK3, ALIGN(8)
   .data            : > RAMGS4
   .sysmem          : > RAMGS4
#else
   .pinit           : > FLASH_BANK3, ALIGN(8)
   .ebss            : > RAMGS4
   .econst          : > FLASH_BANK3, ALIGN(8)
   .esysmem         : > RAMGS4
#endif

   ramgs0 : > RAMGS0, type=NOINIT
   ramgs1 : > RAMGS1, type=NOINIT
   ramgs2 : > RAMGS2, type=NOINIT

   MSGRAM_CPU1_TO_CPU2 > CPU1TOCPU2RAM, type=NOINIT
   MSGRAM_CPU2_TO_CPU1 > CPU2TOCPU1RAM, type=NOINIT

  #if defined(__TI_EABI__)
       .TI.ramfunc : {} LOAD = FLASH_BANK3,  // Use flash that is mapped to CPU2
                        RUN = RAMGS4,
                        LOAD_START(RamfuncsLoadStart),
                        LOAD_SIZE(RamfuncsLoadSize),
                        LOAD_END(RamfuncsLoadEnd),
                        RUN_START(RamfuncsRunStart),
                        RUN_SIZE(RamfuncsRunSize),
                        RUN_END(RamfuncsRunEnd),
                        ALIGN(8)
   #else
       .TI.ramfunc : {} LOAD = FLASH_BANK3,  // Use flash that is mapped to CPU2
                        RUN = RAMGS4,
                        LOAD_START(_RamfuncsLoadStart),
                        LOAD_SIZE(_RamfuncsLoadSize),
                        LOAD_END(_RamfuncsLoadEnd),
                        RUN_START(_RamfuncsRunStart),
                        RUN_SIZE(_RamfuncsRunSize),
                        RUN_END(_RamfuncsRunEnd),
                        ALIGN(8)
   #endif

    /* IMPORTANT: The FreeRTOS statically allocated stack should be allocated to this section only */
   .freertosStaticStack  : >> RAMD2 | RAMD3

    /* IMPORTANT: The FreeRTOS heap should be allocated to this section only as the C28x stack
      memory can be allocated in the lower 64k RAM memory only. */
   .freertosHeap         : > RAMD2 | RAMD3
}

/*
//===========================================================================
// End of file.
//===========================================================================
*/