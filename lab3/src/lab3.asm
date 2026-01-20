%include "./utils.inc"
%include "./constatnts.inc"


global InputGraphAndCheck

section .data
    vprompt_string          db      "Please enter number of nodes n, followed by a nxn adjacency matrix representing the graph: ",0
    vprompt_string_size     dd      $ - vprompt_string
    vsuccess_string         db      "Graph is Connected",0
    vsuccess_string_size    dd      $ - vsuccess_string
    vfail_string            db      "Graph is not Connected",0
    vfail_string_size       dd      $ - vfail_string

section .text

InputGraphAndCheck:
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
    ret

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
