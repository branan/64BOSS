.global loader                          # making entry point visible to linker
.code32 

# setting up the Multiboot header - see GRUB docs for details
.set MAGIC,    0xe85250d6               # 'magic number' lets bootloader find the header
.set MBSIZE, (mbhdr_end - mbhdr_start)
.set CHECKSUM, -(MAGIC + MBSIZE)         # checksum required

.set KERNEL_VMA_BASE, 0xffffffff80000000
.set KERNEL_LMA_BASE, 0x100000
.set CS_KERNEL, 0x08
.set IDT_LOCATION, 0x106000

.align 4
mbhdr_start:
.long MAGIC
.long 0
.long MBSIZE
.long CHECKSUM

.long 2
.long 24
.long mbhdr_start
.long loader
.long (edata - KERNEL_VMA_BASE)
.long (bootheap_end - KERNEL_VMA_BASE)

.long 3
.long 12
.long loader

# re-enable this once there's an FB console
#.long 5
#.long 20
#.long 0
#.long 0
#.long 24

.short 6
.short 0
.long 8

.long 0
.long 8
mbhdr_end:

loader:
    cli
    movl %ebx, %esi
    movl %eax, %edi

    # enable 64-bit page table with CR4.PAE
    movl %cr4, %eax
    bts $5, %eax
    movl %eax, %cr4

    # fill just enough of the page table 

    # create a long mode page table
    movl $(pml4), %eax
    movl %eax, %cr3

    # enable long mode and SYSCALL
    movl $0xc0000080, %ecx
    rdmsr
    bts $8, %eax
    bts $0, %eax
    wrmsr

    lgdt (pGDT)
    lidt (pIDT)

    # setup a basic stack
    mov $(bootheap_end - KERNEL_VMA_BASE), %esp

    movl %cr0, %eax
    bts $31, %eax
    movl %eax, %cr0
    # enable paging and jump to long mode
    ljmp $CS_KERNEL, $(start64-KERNEL_VMA_BASE)

# GDT and IDT
# User CS and DS are backwards from the kernel because SYSRET is stupid
# and assumes descriptors in C-32, D, C-64 order.
.align 4096
GDT_START:
    .quad   0x0000000000000000      # NULL descriptor
    .quad   0x00af9a000000ffff      # Kernel CS
    .quad   0x00cf92000000ffff      # Kernel DS
    .quad   0x0000000000000000      # User DS
    .quad   0x0000000000000000      # User CS
GDT_END:

    .word 0 # padding
    
pGDT:
    .word   GDT_END - GDT_START - 1
    .quad   GDT_START

    .word 0 #padding

pIDT:
    .word 0
    .quad 0

.align 4096 # tables MUST be aligned

# PML4
pml4:
    .quad   (pml3a + 0x0000000000000003)
    .fill   255, 8, 0
    .quad   (pml4 + 0x0000000000000003)
    .fill   254, 8, 0
    .quad   (pml3b + 0x0000000000000003)


# PML3
pml3a:
    .quad (pml2 + 0x0000000000000003)
    .fill 511, 8, 0

pml3b:
    .fill 510, 8, 0
    .quad (pml2 + 0x0000000000000003)
    .quad 0x0000000000000000

# identity-map lower 2MB
pml2:
    .quad (pml1 + 0x000000000000003)
    .fill 511, 8, 0

pml1:
    i = 0
    .rept 512
    .quad ( i + 0x0000000000000003)
    i = i + 4096
    .endr