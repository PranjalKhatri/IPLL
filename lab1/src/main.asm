%include "./syscalls.asm"
%include "./utils.inc"

global ListMostRepeating

section .data
    vprompt_string          db  "Enter Number of Integers in the Array, followed by the actual array: "
    vprompt_string_size     dd  $ - vprompt_string

section .text

ListMostRepeating:
    push    ebp
    mov     ebp,    esp
;display prompt
    lea     eax,    dword [vprompt_string]
    push    dword   [vprompt_string_size]
    push    eax
    call    PrintString
    add     esp,    8
;read size
    sub     esp, 4          ; Reserve 4 bytes
    lea     eax, dword [esp]      ; Get the address of our reserved space
    push    eax             ; Push that address as the argument
    call    ReadUInt
    add     esp, 8

leave:
    mov     esp,    ebp
    pop     ebp
    ret
