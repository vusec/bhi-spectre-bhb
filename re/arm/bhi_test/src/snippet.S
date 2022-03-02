.text
.global fill_bhb


.altmacro
.macro custom_beq val
    beq     label_\val
    label_\val:
.endm

//x0 = ptr history
//x1 = syscall number
//x2 = syscall argument
fill_bhb:
    stp     x29, x30, [sp, #-16]! 
.set i,0
.rept 256
    ldrb    w3, [x0]
    add     x0, x0, 1
    cmp     w3, 0
    custom_beq %i
.set i,i+1
.endr

    mov     x8, x1 
    mov     x0, x2
    svc     0

    ldp     x29, x30, [sp], #16
    ret

