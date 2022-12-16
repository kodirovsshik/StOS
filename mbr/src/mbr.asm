BITS 16

ORG 0x7C00

entry:
    jmp short start
    nop

%if $ - entry != 3
%error Invalid jump signature
%endif

bpb:
    times 90 - ($ - entry) db 0

start:
    cli

    xor ax, ax
    mov ds, ax
    mov ss, ax

    mov sp, 0x7C00
    mov si, sp

    ;preserve for plug & play
    push dx
    push es
    push di

    mov es, ax

    cld
    mov di, 0x0600
    mov cx, 256
    rep movsw
    jmp 0x0000:adjust_cs_ip - (0x7C00 - 0x600)

adjust_cs_ip:
    sti

    mov bx, 0x7C00 + 512 - 2 - 64 - 16
    mov cx, 4

read_loop:
    dec cx
    jz fail
    add bx, 16
    mov al, byte [bx]
    test al, al
    jns read_loop
    mov si, bx

    ;mov ax, 0x0201
    ;mov dh, [si + 1]
    ;mov cl, [si + 2]
    ;mov ch, [si + 3]
    ;mov bx, 0x7C00

		mov word [lba.size], 1
		mov word [lba.dst_off], 0x7C00
		mov word
    mov ax, [

    mov ah, 0x42
    int 0x13
    jc read_error
    test ah, ah
    jnz read_error

    pop di
    pop es
    pop dx
    xor ax, ax
    mov bx, ax
    mov cx, ax
    mov bp, si
    jmp 0x0000:0x7C00

read_error:
    mov ax, 0x0002
    int 0x10

    mov cx, .str_end - .str
    mov si, .str
    mov ah, 0x0E
    mov bx, 0x0007
.loop:
    lodsb
    int 0x10
    loop .loop
    jmp halt
.str: db "Read error"
.str_end:

fail:
    int 0x18
    jmp halt

halt:
    cli
.hlt:
    hlt
    jmp .hlt

lba:
	db 16
	db 0
.size:
	dw 0
.dst_seg:
	dw 0
.dst_off:
	dw 0
.lba:
	dq 0

pad:
times 440 - ($ - entry) db 0xCC

disk_signature:
times 6 db 0

partition_table:
times 64 db 0

mbr_signature:
db 0x55, 0xAA

%if $ - entry != 512
%error File size is not 512
%endif
