     .globl   read_from_vector_64_bits
     .p2align 2
     .type    read_from_vector_64_bits,%function
read_from_vector_64_bits:               // Function "read_from_vector_64_bits" entry point.
     .cfi_startproc
read_from_vector_64_bits_loop:
     ldr x2, [x0], #8
     cmp x0, x1
     ble read_from_vector_64_bits_loop
     ret                     // Return by branching to the address in the link register.
     .cfi_endproc
