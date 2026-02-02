STDIN                            equ  0
STDOUT                           equ  0

NR_restart_syscall               equ  0
NR_exit                          equ  1
NR_fork                          equ  2
NR_read                          equ  3
NR_write                         equ  4
NR_open                          equ  5

new_line         equ 10
carriage_return  equ 13
spacebar         equ 32
tab_char         equ 9
asci_zero        equ '0'

global _start

section .data
    vprompt_string          db  "Enter Number of Integers in the Array, followed by the actual array: "
    vprompt_string_size     dd  $ - vprompt_string

section .bss
    vcount_arr  resd    1000000

section .text

_start:
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
    mov     eax,    NR_exit
    mov     ebx,    0
    int     0x80


;ReadUInt(uint&num)
ReadUInt:
    push ebp
    mov ebp, esp
    push ebx
    sub esp, 8              ; [esp] = tmp char (1 byte), [esp+4] = result (4 bytes)
    mov dword [esp+4], 0    ; initialize result

.SkipLeading:
    mov eax, 3              ; sys_read
    mov ebx, 0              ; stdin
    lea ecx, [esp]          ; read 1 byte into tmp
    mov edx, 1
    int 0x80

    cmp byte [esp], new_line
    je  .SkipLeading
    cmp byte [esp], carriage_return
    je  .SkipLeading
    cmp byte [esp], spacebar
    je  .SkipLeading
    cmp byte [esp], tab_char
    je  .SkipLeading

.ReadChar:
    mov eax, dword [esp+4]  ; load result
    mov ebx, 10
    mul ebx

    movzx ecx, byte [esp]
    sub ecx, asci_zero       ; convert ASCII to integer
    add eax, ecx
    mov dword [esp+4], eax

    mov eax, 3              ; sys_read
    mov ebx, 0              ; stdin
    lea ecx, [esp]          ; read 1 byte into tmp
    mov edx, 1
    int 0x80

    cmp byte [esp], '0'
    jb .EndReadInt
    cmp byte [esp], '9'
    ja .EndReadInt

    jmp .ReadChar

.EndReadInt:
    mov eax, dword [esp+4]   ; final result
    mov ecx, [ebp+8]         ; pointer to caller variable
    mov [ecx], eax

    add esp,    8
    pop ebx
    mov esp, ebp
    pop ebp
    ret

;ListFindMax(int *start,uint cnt)
;INT32_MIN if  cnt =0
ListFindMax:
    push    ebp
    mov     ebp,    esp
    push    ebx

    mov     edx,    [ebp+12]   ; edx = cnt
    test    edx,    edx
    jz      .EmptyList

    mov     ecx,    [ebp+8]    ; ecx = list pointer
    mov     eax,    [ecx]      ; Initial max = first element
    
    cmp     edx,    1
    je      .EndListMax

.loop_start:
    dec     edx
    
    mov     ebx,    [ecx+4*edx]
    cmp     ebx,    eax
    jle     .skip_update
    mov     eax,    ebx

.skip_update:
    test    edx,    edx      ; Check if edx is 0
    jnz     .loop_start      ; If not 0, there are more elements to check

    jmp     .EndListMax

.EmptyList:
    mov     eax,    0x80000000

.EndListMax:
    pop     ebx
    mov     esp,    ebp
    pop     ebp
    ret

;PrintString(char *address,int len)
PrintString:
    push    ebp
    mov     ebp,    esp
    push    ebx

    mov     eax,    NR_write
    mov     ebx,    STDOUT
    mov     ecx,    [ebp+8];mov the address into ecx
    mov     edx,    [ebp+12];mov the length in edx
    int     0x80

    pop     ebx
    mov     esp,    ebp
    pop     ebp
    ret

;PrintUInt(unsigned int a)
PrintUInt:
    push    ebp
    mov     ebp,    esp
    push    ebx
    push    edi

    mov     eax,    [ebp+8]    ; Load the integer to print
    mov     ebx,    10         ; Constant divisor
    xor     edi,    edi        ; count digit pushed to stack

    ;num=0
    test    eax,    eax
    jnz     .div_loop
 
    push    asci_zero
    inc     edi
    jmp     .print_loop

.div_loop:
    test    eax,    eax        ; if quotient is 0
    jz      .print_loop

    xor     edx,    edx        ; clear for dword division
    div     ebx                ; eax = quotient, edx = remainder
 
    add     edx,    asci_zero  ; convert to ascii
    push    edx                ; save digit on stack
    inc     edi                ; digcnt++
    jmp     .div_loop

.print_loop:
    test    edi,    edi        ; check if any digits left
    jz      .done
    
    push    1                  ; param 2: length (1 character)
    lea     eax,    [esp+4]    ; param 1: address of character (offset by 4 due to push above)
    push    eax
    call    PrintString
    add     esp,    8          ; Clean parameters for PrintString
    
    pop     eax                ; Remove used digit
    dec     edi
    jmp     .print_loop

.done:
    pop     edi
    pop     ebx
    
    mov     esp,    ebp
    pop     ebp
    ret
;ListInput(int *first,unsigned int len)
ListInput:
    push    ebp
    mov     ebp,    esp
    push    ebx                     ;callee saved
    mov     ebx,    dword [ebp+12]   ;copy list length
    mov     eax,    dword [ebp+8]   ; address

    sub     esp,    4               ;local variable to keep the address of current input location
    mov     dword [esp],    eax     ;copy address to local variable on stack top
.InputLoop:
    cmp     ebx,    0
    je      .EndListInput
    call ReadUInt                   ;address already at stack top
    add     dword [esp],    4       ;move to next location
    dec     ebx
    jmp     .InputLoop
.EndListInput:
    mov     ebx,    dword [ebp-4]
    mov     esp,    ebp
    pop     ebp
    ret

;ZeroMemory(unsigned char *ptr, unsigned int bytesz)
ZeroMemory:
    push    ebp
    mov     ebp, esp
    push    edi

    mov     ecx, [ebp+12]     ; bytesz
    mov     edi, [ebp+8]      ; ptr
    xor     eax, eax
    cld
    rep     stosb

    pop     edi
    pop     ebp
    ret
