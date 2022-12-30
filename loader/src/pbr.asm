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

lba:
.sz: db 16
db 0
.loader_size: dw 0 ;offset 92
dd 0x00000600
.loader_sector: dq 0 ;offset 98

start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax

	mov ss, ax
	mov sp, 0x7C00
	sti

	cld
	mov di, 0x5F8
	add si, 8
	times 2 movsw
	xor ax, ax
	times 2 stosw

	mov ah, 0x42
	mov si, lba
	int 0x13
	jc read_error
	test ah, ah
	jnz read_error

	jmp 0x0000:0x0600


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
.hlt:
    hlt
    jmp .hlt
.str: db "Read error"
.str_end:


pad:
times 512 - 2 - 64 - 6 - 8 - ($ - entry) db 0xCC

boot_hash: ;to be filled by loader at runtime
dq 0

disk_signature:
times 6 db 0

partition_table:
times 64 db 0

mbr_signature:
db 0x55, 0xAA

%if $ - entry != 512
%error File size is not 512
%endif
