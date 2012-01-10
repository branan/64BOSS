.global _Z6memcpyPvPKvm
.global _Z6memsetPvhm
.global _Z6memsetPvtm
.global _Z6memsetPvjm
.global _Z6memsetPvmm
.global _Z5khaltv

_Z6memcpyPvPKvm:
    test %rdx, %rdx
    jz do_return
    mov %rdx, %rcx
    rep movsb
do_return:
    ret

_Z6memsetPvhm:
    mov %rsi, %rax
    mov %rdx, %rcx
    rep stosb
    ret

_Z6memsetPvtm:
    mov %rsi, %rax
    mov %rdx, %rcx
    rep stosw
    ret

_Z6memsetPvjm:
    mov %rsi, %rax
    mov %rdx, %rcx
    rep stosl
    ret

_Z6memsetPvmm:
    mov %rsi, %rax
    mov %rdx, %rcx
    rep stosq
    ret

_Z5khaltv:
    cli
    hlt