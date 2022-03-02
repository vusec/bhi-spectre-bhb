.text

.global ret_gadget
ret_gadget:
    nop
    ret

.global hit_gadget
.align 10
hit_gadget:
    ldr     x1, [x1]
    ret

.global ind_gadget
.align 10
ind_gadget:
    ldr     x0, [x0]
    br      x0
    
