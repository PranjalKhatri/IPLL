%include "./syscalls.inc"
%include "./constatnts.inc"

extern scanf
extern printf

global ListFindMax
global ReadUInt
global ReadFloat
global PrintString
global ListInput
global FloatListInput
global FloatListFindMinIdx
global FloatSelectionSort
global ZeroMemory
global PrintUInt
global CPrintFloat

section .data
    vfloat_fmt_str    db  "%f",0

section .text

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
; xmm regs not allowed in lab
; CPrintFloat:
;     push    ebp
;     mov     ebp,    esp
;
;     movss   xmm0, dword [ebp+8]
;     cvtss2sd xmm0, xmm0
;     sub     esp, 8
;     movsd   qword [esp], xmm0
;
;     push    vfloat_fmt_str
;     call    printf
;
;     add     esp, 12
;     mov     esp,    ebp
;     pop     ebp
;     ret
    
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

; FloatListFindMinIdx:
;     push    ebp
;     mov     ebp,    esp
;     push    ebx
;
;     mov     eax,    [ebp+8]    ; eax = ptr
;     mov     edx,    [ebp+12]   ; edx = ptr to idx
;     mov     ecx,    [ebp+16]   ; ecx = sz
;
;     test    ecx,    ecx
;     jz      .Done
;
;     movss   xmm1,   dword [eax] ; xmm1 = current min value
;     xor     ebx,    ebx         ; ebx = current index (0)
;     xor     edi,    edi         ; edi = best index found so far (0)
;
; .Loop:
;     movss   xmm0,   dword [eax + ebx*4] 
;
;     ucomiss xmm0,   xmm1
;     jae     .SkipMin
;
;     movss   xmm1,   xmm0        ; update min value
;     mov     edi,    ebx         ; update best index
;
; .SkipMin:
;     inc     ebx
;     cmp     ebx,    ecx
;     jl      .Loop
;
;     mov     [edx],  edi
;
; .Done:
;     pop     ebx
;     mov     esp,    ebp
;     pop     ebp
;     ret
