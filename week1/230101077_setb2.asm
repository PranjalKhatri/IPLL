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
    vprompt_string          db      "Please enter number of nodes n, followed by a nxn adjacency matrix representing the graph: ",0
    vprompt_string_size     dd      $ - vprompt_string
    vsuccess_string         db      "Graph is Connected",0
    vsuccess_string_size    dd      $ - vsuccess_string
    vfail_string            db      "Graph is not Connected",0
    vfail_string_size       dd      $ - vfail_string

section .text

_start:
    push    ebp
    mov     ebp,    esp
    push    ebx
    push    esi
    sub     esp,    4   ;size of graph
.Prompt:
    push    dword [vprompt_string_size]
    push    vprompt_string
    call    PrintString
    add     esp,    8
.Inputn:
    lea     eax,    dword [ebp-12]
    push    eax
    call    ReadUInt
    add     esp,    4
.InputAdjMat:
    mov     eax,    dword [ebp-12]
    imul    eax,    eax
    shl     eax,    2
    sub     esp,    eax ; reserve space for adj mat
    lea     esi,    dword [esp]
    mov     ebx,    0
.Loop:
    mov     eax,    dword [ebp-12]
    mul     eax
    cmp     ebx,    eax
    je      .Check
    mov     eax,    ebx ;eax = base+ebx*4 = ptr to next input
    shl     eax,    2
    add     eax,    esi

    push    eax
    call    ReadUInt
    add     esp,    4

    inc     ebx
    jmp     .Loop
.Check:
    lea     eax,    dword [esp]
    push    dword [ebp-12]   ; param 2 = n
    push    esi
    call    CheckConnectivity
    add     esp,    8
.Done:
    lea     esp,    dword [ebp-8]
    pop     esi
    pop     ebx
    pop     ebp
    mov     eax,    NR_exit
    mov     ebx,    0
    int     0x80

; CheckConnectivity(int** adjMat,int nodes)
CheckConnectivity:
    push    ebp
    mov     ebp,    esp
    push    ebx 

    mov     eax,    dword [ebp+12]
    shl     eax,    2
    sub     esp,    eax     ; reserve space for visited array
    lea     ebx,    dword [esp] ; ebx = start of visited array
    push    eax             ; param 2: bytesz to zero
    push    ebx             ; param 1: ptr to visited array
    call    ZeroMemory
    add     esp,    8
.CallDfs:
    ; mark initial node
    mov     dword [ebx],    1
    push    ebx
    push    dword [ebp+12]
    push    0
    push    dword [ebp+8]
    call    Dfs
    add     esp,    16

    mov     eax,    0             ; eax = current index
    lea     ecx,    dword [ebx]   ; ecx = ptr to current element
.CheckAllVisited:
    cmp     eax,    dword [ebp+12]
    je      .Success
    mov     edx,    dword [ecx]
    test    edx,    edx
    jz      .Fail
    inc     eax                 ; idx++
    add     ecx,    4           ; point to next element
    jmp     .CheckAllVisited
.Fail:
    push    dword [vfail_string_size]
    push    vfail_string   
    call    PrintString
    add     esp,    8
    jmp     .Done
.Success:
    push    dword [vsuccess_string_size]
    push    vsuccess_string   
    call    PrintString
    add     esp,    8
    jmp     .Done
.Done:
    lea     esp,    dword [ebp-4]
    pop     ebx
    pop     ebp
    ret
    

; Dfs(int*adjMat,int current,int nodes, int* visited)
; mark the initial node as visited before calling
Dfs:
    push    ebp
    mov     ebp,    esp
    push    ebx
    push    esi
    
    mov     ebx,    0           ; current neighbor index
    mov     eax,    dword [ebp+12]  ; eax = current*nodes*4 = offset
    mul     dword [ebp+16]
    shl     eax,    2
    mov     esi,    dword [ebp+8]
    add     esi,    eax         ; esi = ptr to rows corresponding to current node

.Loop:
    ; done iterating
    cmp     ebx,    dword [ebp+16]
    je      .Done
    ; Check if ebx node is actually connectes
    mov     eax,    ebx
    shl     eax,    2
    mov     edx,    eax
    add     eax,    esi         ; eax = ptr to current neighbor
    mov     ecx,    dword [eax]
    test    ecx,    ecx    
    jz      .Next
    ;       Check already visited
    mov     eax,    dword [ebp+20]
    add     eax,    edx
    mov     ecx,    dword [eax]
    test    ecx,    ecx
    jnz     .Next
    ;       mark visited
    mov     dword [eax],    1
    push    dword [ebp+20]
    push    dword [ebp+16]
    push    ebx
    push    dword [ebp+8]
    call    Dfs
    add     esp,    16
.Next:
    inc     ebx
    jmp     .Loop
.Done:
    pop     esi
    pop     ebx
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
