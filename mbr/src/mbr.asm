BITS 16

ORG 0x600

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

    sti
    ;DI is supposed to store some plug & play data ptr, save for later use
    push di

    mov es, ax

    cld
    mov di, 0x0600
    mov cx, 256
    rep movsw
    jmp 0x0000:.relocated
.relocated:

    mov bx, partition_table - 16
    mov cx, 4

read_loop:
    dec cx
    jz fail
    add bx, 16
    mov al, byte [bx]
    test al, al
    jns read_loop
    
    lea si, [bx + 8]
    mov di, lba.lba
    times 2 movsw

    mov si, lba
    mov ah, 0x42
    int 0x13
    jc read_error
    test ah, ah
    jnz read_error

    pop di ;PnP ptr
    mov bp, bx
    mov si, bx
    xor ax, ax
    mov bx, ax
    mov cx, ax
    jmp 0x7C00

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
halt:
    hlt
    jmp halt


lba:
	db 16
	db 0
.size:
	dw 1
.dst_off:
	dw 0x7C00
.dst_seg:
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
