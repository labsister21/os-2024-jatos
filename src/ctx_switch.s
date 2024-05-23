global process_context_switch

; Load struct Context (CPU GP-register) then jump
; Function Signature: void process_context_switch(struct Context ctx);
process_context_switch:
    ; Using iret (return instruction for interrupt) technique for privilege change
    lea  ecx, [esp+0x04] ; Save the base address for struct Context ctx


;1. Sebelum melakukan semuanya, simpan base address function argument ctx
;2. Lanjutkan dengan setup iret stack dengan push
    mov eax, 0x20 | 0x3
    push eax ;ss
    mov eax, [ecx + 12]
    push eax ;esp
    mov eax, [ecx + 52]
    push eax ;eflags
    mov eax, 0x18 | 0x3
    push eax ;cs
    mov eax, [ecx + 48]
    push eax ;eip

;3. Load semua register dari ctx
    mov edi, [ecx]
    mov esi, [ecx + 4]
    mov ebp, [ecx + 8]
    mov ebx, [ecx + 16]
    mov edx, [ecx + 20]
    mov eax, [ecx + 28]
    mov gs, [ecx + 32]
    mov fs, [ecx + 36]
    mov es, [ecx + 40]
    mov ds, [ecx + 44]
    mov ecx, [ecx + 24]

;4. Cleanup operasi register yang tersisa jika ada

;5. Lakukan jump ke process dengan iret
    iret