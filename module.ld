ENTRY (loader)

SECTIONS
{
    . = ALIGN(0x1000);

    .text :
    {
        *(.text)
        *(.gnu.linkonce.t*)
    }

    . = ALIGN(0x1000);

    .rodata :
    {
        start_ctors = .;
        *(.ctor*)
        end_ctors = .;

        start_dtors = .;
        *(.dtor*)
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
    }

    . = ALIGN(0x1000);

    .data :
    {
        *(.data)
        *(.gnu.linkonce.d*)
        edata = .;
    }

    . = ALIGN(0x1000);

    .bss :
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        *(.gnu.linkonce.b*)
        ebss = .;
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame) 
    }
}
