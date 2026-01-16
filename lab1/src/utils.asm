%include "./syscalls.asm"

global ListFindMax
global ReadUInt
global PrintString

;CONSTANTS
new_line    equ 10
asci_zero   equ '0'

section .text

;ReadUInt(uint&num)
ReadUInt:
    push ebp
    mov ebp, esp
    push ebx
    sub esp, 8              ; [esp] = tmp char (1 byte), [esp+4] = result (4 bytes)
    mov dword [esp+4], 0    ; initialize result

RUIReadChar:
    mov eax, 3              ; sys_read
    mov ebx, 0              ; stdin
    lea ecx, [esp]          ; read 1 byte into tmp
    mov edx, 1
    int 0x80

    cmp byte [esp], new_line
    je RUIEndReadInt

    mov eax, dword [esp+4]  ; load result
    mov ebx, 10
    mul ebx

    movzx ecx, byte [esp]
    sub ecx, asci_zero       ; convert ASCII to integer
    add eax, ecx
    mov dword [esp+4], eax
    jmp RUIReadChar

RUIEndReadInt:
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
    jz      LFMempty_list

    mov     ecx,    [ebp+8]    ; ecx = list pointer
    mov     eax,    [ecx]      ; Initial max = first element
    
    cmp     edx,    1
    je      LFMEndListMax

LFMloop_start:
    dec     edx
    
    mov     ebx,    [ecx+4*edx]
    cmp     ebx,    eax
    jle     LFMskip_update
    mov     eax,    ebx

LFMskip_update:
    cmp     edx,    1
    jg      LFMloop_start

    jmp     LFMEndListMax

LFMempty_list:
    mov     eax,    0x80000000

LFMEndListMax:
    pop     ebx
    mov     esp,    ebp
    pop     ebp
    ret

;PrintString(char *address,int len)
PrintString:
    mov     eax,    NR_write
    mov     ebx,    STDOUT
    mov     ecx,    [esp+4];mov the address into ecx
    mov     edx,    [esp+8];mov the length in edx
    int     0x80
