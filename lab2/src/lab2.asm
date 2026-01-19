%include "./syscalls.inc"
%include "./utils.inc"

global KeleSortFloatList

section .data
    vprompt_string          db  "Input n and k, followed by n floats: "
    vprompt_string_size     dd  $ - vprompt_string
    verror_string           db  "k should be smaller then n."
    verror_string_size      db  $ - verror_string

section .text
    
KeleSortFloatList:
    push    ebp
    mov     ebp,    esp
    push    edi

    sub     esp,    8   ;space for local variable n and k
.Prompt:
    push    dword [vprompt_string_size]
    push    vprompt_string
    call    PrintString
    add     esp,    8
.Readnk:
    lea     eax,    dword [ebp-4]
    push    eax
    call    ReadUInt
    add     esp,    4
    lea     eax,    dword [ebp-8]
    push    eax
    call    ReadUInt
    add     esp,    4

    mov     eax,    dword [ebp-4]
    cmp     eax,    dword [ebp-8]
    jge     .Transformk
    push    dword [verror_string_size]
    push    verror_string
    call    PrintString
    mov     eax,    1
    int     0x80
.Transformk:
    ;k->n-k+1 for k largest.
    mov     eax,    dword [ebp-4]
    sub     eax,    dword [ebp-8]
    add     eax,    1
    mov     dword [ebp-8], eax
.ReadArr:
    mov     eax,    dword [ebp-4]
    shl     eax,    2
    sub     esp,    eax ;reserve space for floats after n and k
    lea     edi,    dword [esp]
    shr     eax,    2

    push    eax
    push    edi
    call    FloatListInput
    add     esp,    8
.SortArr:
    mov     ecx,    dword [ebp-4]
    push    ecx
    push    edi
    call    FloatSelectionSort
    add     esp,    8
.FindKth:
    mov     eax,    dword [ebp-8]
    dec     eax;0 based
    shl     eax,    2
    add     edi,    eax
    mov     eax,    dword [edi]
    ;print it
    push    eax
    call    CPrintFloat 
    pop     eax
.Done:
    pop     edi
    mov     esp,    ebp
    pop     ebp
    ret
