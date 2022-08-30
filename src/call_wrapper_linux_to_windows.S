_start:
    movq (%rsp), %rax
    push %rbp
    movq %rsp, %rbp
    subq $0x100, %rsp
    andq $~0x0f, %rsp
    push %r9 # Push 5th and 6th argument.
    push %r8
    push %rcx
    push %rdx
    push %rsi
    push %rdi

    movabs $0x0101010101010101, %rcx # Check if call by linux
    movabs $0x0202020202020202, %rdx # %rcx <= %rax < %rdx => Call by linux
    cmp %rax, %rcx
    ja .run_directly
    cmp %rax, %rdx
    jbe .run_directly
    jmp .covert

.run_directly:
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %r8
    pop %r9
    movabs $0x0303030303030303, %rax # Inject target function address
    callq *%rax
    jmp .ret

.covert:
    pop %rcx
    pop %rdx
    pop %r8
    pop %r9
    
    movabs $0x0000000000000000, %rax # Inject target function address
    callq *%rax

.ret:
    movq %rbp, %rsp
    popq %rbp
    ret
