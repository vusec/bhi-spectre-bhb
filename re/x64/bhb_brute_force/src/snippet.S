; Thursday, September 9th 2021
;
; Enrico Barberis - e.barberis@vu.nl
; Pietro Frigo - p.frigo@vu.nl
; Marius Muench - m.muench@vu.nl
; Herbert Bos - herbertb@cs.vu.nl
; Cristiano Giuffrida - giuffrida@cs.vu.nl
;
; Vrije Universiteit Amsterdam - Amsterdam, The Netherlands

section .text

align 64
global ret_gadget
ret_gadget:
    nop
    ret

align 64
global hit_gadget
hit_gadget:
    mov     rax, [rsi]
    ret
