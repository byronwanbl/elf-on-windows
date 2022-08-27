_start:
    pop %rax # Save return address.
    push %r9 # Push 5th and 6th argument.
    push %r8
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    pop %rcx
    pop %rdx
    pop %r8
    pop %r9

    movq %rax, %rdi

    movq %rsp, %rsi      # Put return address to another place on stack.
    addq $0x40, %rsi     # Before: [%rip] [arg 7] [arg 8] ...
.loop0:                  #           |
    movq (%rsp), %rax    #           + >---------------> +
    movq %rax, -8(%rsp)  #                               |
    addq $0x08, %rsp     # After:  [arg 5] [arg 6] ... [%rip]
    cmp %rsp, %rsi       #         | <---   6    ---> |        (So it support 12 arguments at most.)
    jne .loop0 # Label uses relative address.

    subq $0x48, %rsp      # According to microsoft document, 
                          # there should be a "shadow store" on stack.
                          # But I test on mingw and msvc, compiler does not prepare "shadown store".
                          # And it dose not align %rsp to 16-bit.

    movq %rdi, 0x40(%rsp) # Store %rip
    movabs $0x00000000, %rax # Inject target function address
    callq *%rax

    addq $0x10, %rsp # Recover stack
    movq 0x30(%rsp) , %rcx # Put %rip back.

    movq %rsp, %rsi
    addq $0x30, %rsp
.loop1:
    movq -8(%rsp), %rax
    movq %rax, (%rsp)
    subq $0x08, %rsp
    cmp %rsp, %rsi
    jne .loop1

    movq %rcx, (%rsp)
    ret
