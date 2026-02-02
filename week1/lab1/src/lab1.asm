%include "./syscalls.inc"
%include "./utils.inc"

global ListMostRepeating

section .data
    vprompt_string          db  "Enter Number of Integers in the Array, followed by the actual array: "
    vprompt_string_size     dd  $ - vprompt_string

section .bss
    vcount_arr  resd    1000000

section .text

ListMostRepeating:
    push    ebp
    mov     ebp,    esp
    push    esi
    push    edi
    sub     esp,    4;local variable to hold list size
.Prompt:
    lea     eax,    dword [vprompt_string]
    push    dword [vprompt_string_size]
    push    eax
    call    PrintString
    add     esp,    8
.ReadSize:
    lea     eax,    dword [ebp-12]
    push    eax
    call    ReadUInt
    add     esp,    4
.ReadList:
    mov     eax,    dword [ebp-12]
    shl     eax,    2  ; sz to byte
    sub     esp,    eax; allocate size for sz list
    shr     eax,    2
    lea     eax,    dword [esp];start of list

    push    dword [ebp-12] ;param 2: sizeof array
    push    eax         ;param 1: start of array
    call    ListInput
    add     esp,    8 ; clean parameters
.ZeroFreq:
    lea     eax,    dword [esp]
    push    dword [ebp-12]
    push    eax
    call    ListFindMax;eax contain max of array
    add     esp,    8
    
    inc     eax     ;+1 for 0. #[0-max]=max+1
    mov     esi,    eax;save in esi to use in findmostRepeated
    shl     eax,    2
    lea     ecx,    dword [vcount_arr]
    push    eax
    push    ecx
    call    ZeroMemory;initialize frequencies with 0
    add     esp,    8

    mov     edi,    dword [ebp-12]
.PopulateFrequency:
    cmp     edi,    0 
    je      .FindMostRepeated
    dec     edi         ; subtract one before since list 1st element =esp+0
    mov     eax,    dword [esp+4*edi]
    inc     dword [vcount_arr+eax*4]
    jmp     .PopulateFrequency

.FindMostRepeated:
    mov     ecx,    0;max freq
    mov     eax,    0;element corresp. to max freq
    mov     edi,    0;idx
;esi: max+1
.Loop:
    cmp     esi,    edi
    je      .PrintMostRepeated
    cmp     ecx,    dword [vcount_arr+4*edi]
    jge     .SkipExchange
    mov     ecx,    dword [vcount_arr+4*edi]
    mov     eax,    edi
.SkipExchange:
    inc     edi
    jmp     .Loop
.PrintMostRepeated:
    push    eax 
    call    PrintUInt
    add     esp,    4
.Leave:
    mov     eax,    dword [ebp-12]
    shl     eax,    2
    add     esp,    eax; clear array 
    add     esp,    4 ; and its size

    pop     edi
    pop     esi
    mov     esp,    ebp
    pop     ebp
    ret
