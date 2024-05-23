global _start
extern main

section .text
_start:
	call main
    mov ebx, eax
	mov eax, 9   ; Assuming syscall exit is 9
	int 0x30
