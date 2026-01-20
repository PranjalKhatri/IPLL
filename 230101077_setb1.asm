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

extern scanf
extern printf

global main

section .data
    vprompt_string          db  "Input n and k, followed by n floats: "
    vprompt_string_size     dd  $ - vprompt_string
    verror_string           db  "Invalid Input"
    verror_string_size      dd  $ - verror_string
    vfloat_fmt_str          db  "%f",0

section .text
    
main:
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
    test    eax,    eax
    jz      .InvalidInputPrint

    mov     eax,    dword [ebp-8]
    test    eax,    eax
    jz      .InvalidInputPrint

    mov     eax,    dword [ebp-4]    ; eax = n
    cmp     eax,    dword [ebp-8]    ; n ? k
    jge     .Transformk           ; n >= k → valid

.InvalidInputPrint:
    push    dword [verror_string_size]
    push    verror_string
    call    PrintString
    add     esp,    8
    mov     eax,    NR_exit
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

; ReadFloat(float *ptr)
; do 16-byte alignment for the System V ABI
ReadFloat:
    push    ebp
    mov     ebp, esp

    push    dword [ebp+8]      ; push ptr to float
    push    vfloat_fmt_str   ; push address of string
    ;alignment=eip+ebp+8params=16 
    call    scanf
    add     esp, 8             ; clear params

    mov     esp, ebp
    pop     ebp
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
;CPrintFloat(float f)
CPrintFloat:
    push    ebp
    mov     ebp, esp

    fld     dword [ebp+8]       ;fld->load on float stack in 80 bit precision
    sub     esp, 8
    fstp    qword [esp]         ; push to stack from flt stack
    push    vfloat_fmt_str
    call    printf

    add     esp, 12             ; 8 double +4 char*
    mov     esp, ebp
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

;FloatListInput(float*ptr,unsigned int sz)
FloatListInput:
    push    ebp
    mov     ebp, esp
    push    esi
    push    ebx

    mov     ebx, [ebp+8]    ; pointer to current element
    mov     esi, [ebp+12]   ; count remaining elements

.Loop:
    test    esi, esi
    jz      .Done

    push    ebx
    call    ReadFloat
    add     esp, 4

    add     ebx, 4
    dec     esi
    jmp     .Loop

.Done:
    pop     ebx
    pop     esi
    mov     esp, ebp
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

;FloatSelectionSort(float*ptr,unsigned int sz)
FloatSelectionSort:
    push    ebp
    mov     ebp,    esp
    push    ebx
    push    esi
    push    edi

    mov     esi,    0               ; i = outer loop index
    mov     ebx,    [ebp+12]        ; ebx = size
    dec     ebx                     ; go to size-1 max

.OuterLoop:
    cmp     esi,    ebx
    jge     .Done

    mov     eax,    [ebp+8]
    lea     edi,    [eax + esi*4]   ; edi = start of unsorted portion

    mov     ecx,    [ebp+12]
    sub     ecx,    esi; remaining size

    sub     esp,    4
    mov     edx,    esp             ; edx points to local variable

    push    ecx                     ; sz
    push    edx                     ; &minIdx
    push    edi                     ; current ptr
    call    FloatListFindMinIdx
    add     esp,    12

    pop     eax                     ; eax = min idx, relative to current pos
    add     eax,    esi             ; make absolute

    ; ecx = ptr + (esi * 4)  [current position]
    ; edx = ptr + (eax * 4)  [min position found]
    mov     ebx,    [ebp+8]         ; base ptr
    lea     ecx,    [ebx + esi*4]
    lea     edx,    [ebx + eax*4]

    mov     eax,    [ecx]           ; temp = current
    mov     ebx,    [edx]           ; temp2 = min
    mov     [ecx],  ebx             ; current = min
    mov     [edx],  eax             ; min = temp

    inc     esi
    mov     ebx,    [ebp+12]        ; restore size
    dec     ebx
    jmp     .OuterLoop

.Done:
    pop     edi
    pop     esi
    pop     ebx
    mov     esp,    ebp
    pop     ebp
    ret

; FloatListFindMinIdx(float* ptr, unsigned int* idx, unsigned int sz)
FloatListFindMinIdx:
    push    ebp
    mov     ebp, esp
    push    ebx
    push    edi

    mov     eax, [ebp+8]      ; eax = ptr
    mov     edx, [ebp+12]     ; edx = ptr to idx
    mov     ecx, [ebp+16]     ; ecx = sz

    test    ecx, ecx
    jz      .Done             ; empty array

    fld     dword [eax]       ; initial min
    xor     edi, edi          ; bestidx
    mov     ebx, 1            ; curidx=1

    cmp     ecx, 1
    jbe     .WriteResult

.Loop:
    fld     dword [eax + ebx*4]
    fcomi   st0, st1
    
    jae     .NotSmaller

    fstp    st1             ; top to 1st and pop top so first at top.confusing shit
    mov     edi, ebx          ; Update best index
    jmp     .Next

.NotSmaller:
    fstp    st0             ; pop new element

.Next:
    inc     ebx
    cmp     ebx, ecx
    jl      .Loop

.WriteResult:
    mov     dword [edx], edi
    fstp    st0             ;cleanup

.Done:
    pop     edi
    pop     ebx
    mov     esp, ebp
    pop     ebp
    ret


