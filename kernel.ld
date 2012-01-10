ENTRY (loader)

kernel_VMA = 0xffffffff80000000;
kernel_LMA = 0x00100000;

SECTIONS
{
    . = kernel_LMA;

    kernel_start = .;

    .text_boot :
    {
        CMakeFiles/kernel.dir/loader.S.o (.text)
    }

    . = ALIGN(0x1000);
    . += kernel_VMA;

    .text : AT(ADDR(.text) - kernel_VMA)
    {
        *(.text)
        *(.gnu.linkonce.t*)
    }

    . = ALIGN(0x1000);

    .rodata : AT(ADDR(.rodata) - kernel_VMA)
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

    .data : AT(ADDR(.data) - kernel_VMA)
    {
        *(.data)
        *(.gnu.linkonce.d*)
        edata = .;
    }

    . = ALIGN(0x1000);

    .bss : AT(ADDR(.bss) - kernel_VMA)
    {
        sbss = .;
        *(COMMON)
        *(.bss)
        *(.gnu.linkonce.b*)
        ebss = .;
    }

    . = ALIGN(0x1000);
    .bootheap : AT(ADDR(.bootheap) - kernel_VMA)
    {
        bootheap_start  = .;
        . += 0x8000;
        bootheap_end = .;
    }

    . = ALIGN(0x1000);
    kernel_end = . - kernel_VMA;

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame) 
    }
}
