.global start64

.set KERNEL_VMA_BASE, 0xffffffff80000000
.set KERNEL_LMA_BASE, 0x100000
.set CS_KERNEL, 0x08
.set CS_DATA, 0x10

start64:
    movq $(bootheap_end - KERNEL_VMA_BASE), %rsp

    movq $long_entry, %rax
    pushq $CS_KERNEL
    pushq $long_entry
    lretq

long_entry:
    movq $(bootheap_end), %rsp

    # Load the address of the syscall entrypoint into edx:eax
    movq $SysCall, %rax
    movq %rax, %rdx
    shrq $32, %rdx
    # Push that address into IA32_LSTAR
    movq $0xC0000082, %rcx
    wrmsr
    # Load our segments into edx:eax
    movq $0, %rax
    movq $0x1008, %rdx
    #Push those segments into IA32_STAR
    movq $0xC0000081, %rcx
    wrmsr

    pushq %rsi
    pushq %rdi

    call kinit
    movq $KERNEL_VMA_BASE, %rsp
    call kmain

    hlt #if we ever return, just halt

SysCall:
    cmp %rax, (numsyscalls)
    jge SysCallCancel
    mov %rsp, %r11
    mov $KERNEL_VMA_BASE, %rsp
    push %r11
    push %rcx
    mov %r10, %rcx
    mov $ksyscall, %r11
    call *(%r11, %rax, 8)
    pop %rcx
    pop %rsp
    sysret

SysCallCancel:
    mov $0xffffffffffffffff, %rax
    sysret

ksyscall:
    ret
endksyscall:

numsyscalls:
    .quad (endksyscall-ksyscall)/8