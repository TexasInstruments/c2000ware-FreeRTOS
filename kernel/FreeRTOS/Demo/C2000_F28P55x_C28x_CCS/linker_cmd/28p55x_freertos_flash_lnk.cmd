MEMORY
{
   BEGIN            : origin = 0x080000, length = 0x000002  // Update the codestart location as needed

   BOOT_RSVD        : origin = 0x000002, length = 0x000126     /* Part of M0, BOOT rom will use this for stack */
   RAMM0            : origin = 0x000128, length = 0x0002D8
   RAMM1            : origin = 0x000400, length = 0x000400

   /*
   RAMLS0           : origin = 0x008000, length = 0x000800
   RAMLS1           : origin = 0x008800, length = 0x000800
   RAMLS2           : origin = 0x009000, length = 0x000800
   RAMLS3           : origin = 0x009800, length = 0x000800
   RAMLS4           : origin = 0x00A000, length = 0x000800
   RAMLS5           : origin = 0x00A800, length = 0x000800
   RAMLS6           : origin = 0x00B000, length = 0x000800
   RAMLS7           : origin = 0x00B800, length = 0x000800
   */
   RAMLS0_7         : origin = 0x008000, length = 0x004000

   RAMLS8           : origin = 0x014000, length = 0x002000  // When configured as CLA program use the address 0x4000
   RAMLS9           : origin = 0x016000, length = 0x002000  // When configured as CLA program use the address 0x6000

   // RAMLS8_CLA    : origin = 0x004000, length = 0x002000  // Use only if configured as CLA program memory
   // RAMLS9_CLA    : origin = 0x006000, length = 0x002000  // Use only if configured as CLA program memory

   RAMGS0           : origin = 0x00C000, length = 0x002000
   RAMGS1           : origin = 0x00E000, length = 0x002000
   RAMGS2           : origin = 0x010000, length = 0x002000
   RAMGS3           : origin = 0x012000, length = 0x002000

   /* Flash Banks (128 sectors each) */
   FLASH_BANK0     : origin = 0x080002, length = 0x1FFFE
   FLASH_BANK1     : origin = 0x0A0000, length = 0x20000
   FLASH_BANK2     : origin = 0x0C0000, length = 0x20000
   FLASH_BANK3     : origin = 0x0E0000, length = 0x20000
   FLASH_BANK4     : origin = 0x100000, length = 0x20000


   CLATOCPURAM      : origin = 0x001480,   length = 0x000080
   CPUTOCLARAM      : origin = 0x001500,   length = 0x000080
   CLATODMARAM      : origin = 0x001680,   length = 0x000080
   DMATOCLARAM      : origin = 0x001700,   length = 0x000080

   RESET            : origin = 0x3FFFC0, length = 0x000002
}



SECTIONS
{
   codestart        : > BEGIN, ALIGN(8)
   .text            : > FLASH_BANK0,   ALIGN(8)
   .cinit           : > FLASH_BANK0,  ALIGN(8)
   .switch          : > FLASH_BANK0,  ALIGN(8)
   .reset           : > RESET,                  TYPE = DSECT /* not used, */

   .stack           : > RAMM1
   /* IMPORTANT: The FreeRTOS statically allocated stack should be allocated to this section only */
   .freertosStaticStack  : >> RAMM1 | RAMM0 | RAMLS0_7
/* IMPORTANT: The FreeRTOS heap should be allocated to this section only as the C28x stack
      memory can be allocated in the lower 64k RAM memory only. */
   .freertosHeap         : > RAMLS0_7 | RAMGS0 | RAMGS1

#if defined(__TI_EABI__)
   .init_array      : > FLASH_BANK0,  ALIGN(8)
   .bss             : > RAMLS0_7
   .bss:output      : > RAMLS0_7
   .bss:cio         : > RAMLS0_7
   .data            : > RAMLS0_7
   .sysmem          : > RAMLS0_7
   .const           : > FLASH_BANK0,  ALIGN(8)
#else
   .pinit           : > FLASH_BANK0,  ALIGN(8)
   .ebss            : > RAMLS0_7
   .esysmem         : > RAMLS0_7
   .cio             : > RAMLS0_7
   .econst          : > FLASH_BANK0,  ALIGN(8)
#endif

    ramgs0 : > RAMGS0
    ramgs1 : > RAMGS0

    /*  Allocate IQ math areas: */
   IQmath           : > FLASH_BANK0, ALIGN(8)
   IQmathTables     : > FLASH_BANK0, ALIGN(8)

   .TI.ramfunc      : LOAD = FLASH_BANK0,
                      RUN = RAMLS0_7,
                      LOAD_START(RamfuncsLoadStart),
                      LOAD_SIZE(RamfuncsLoadSize),
                      LOAD_END(RamfuncsLoadEnd),
                      RUN_START(RamfuncsRunStart),
                      RUN_SIZE(RamfuncsRunSize),
                      RUN_END(RamfuncsRunEnd),
                      ALIGN(8)

}
