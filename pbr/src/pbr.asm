
ORG 0x7C00
BITS 16



entry:
	jmp short start1
	nop

%if $ - entry != 3
%error Invalid jump sequence
%endif



bpb:
	times 90 - ($ - entry) db 0



start1:
	jmp start



lba:
.sz: db 16
db 0
.size: dw 1
.offset: dw 0x7E00
.segment: dw 0x0000
.first_sector: dq 0

loader_relative_sector: dd 0

%if loader_relative_sector - entry != 108
%error Imported data misplacement
%endif


start:
;Setup memory layout
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax

	mov ss, ax
	mov sp, 0x7C00
	sti

;Save partition beginning for the loader
	cld
	mov di, 0x5F8
	add si, 8
	times 2 movsw
	xor ax, ax
	times 2 stosw

;Read loader sector listing into 0x7E00 (next "memory sector")
	mov bx, loader_relative_sector
	call lba_store_relative_sector_number

	mov ah, 0x42
	mov si, lba
	int 0x13
	jc read_error
	test ah, ah
	jnz read_error
	inc byte [read_error.read_num]

	mov bx, 0x7E00 ;sector number ptr
	mov bp, 0x7E00 + 101 * 4 ;sector count ptr
	mov word [lba.offset], 0x600

.read_loop:
	times 2 nop
	mov al, [bp] ;load entry's sectors count
	test al, al
	jz .read_end
	
	mov [lba.size], al

;copy partition sector number into lba

	call lba_store_relative_sector_number

	mov si, lba
	mov ah, 0x42
	int 0x13
	jc read_error
	test ah, ah
	jnz read_error

;advance to next entry
	movzx ax, byte [bp]
	shl ax, 9
	add [lba.offset], ax

	inc bp
	add bx, 4
	jmp .read_loop

.read_end:
	jmp 0x0000:0x0600



;bx = ptr to 4 byte partition relative sector number
lba_store_relative_sector_number:
	mov si, 0x5F8
	mov di, lba.first_sector
	mov cx, 4
	rep movsw

	mov di, lba.first_sector

	mov ax, [bx]
	add word [di], ax

	mov ax, [bx + 2]
	adc word [di + 2], ax

	mov ax, 0
	adc word [di + 4], ax
	adc word [di + 6], ax
	ret



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
.str: db "Read error "
.read_num: db "1"
.str_end:



pad:
times 512 - 2 - 64 - 6 - ($ - entry) db 0xCC

disk_signature:
times 6 db 0

partition_table:
times 64 db 0

boot_signature:
db 0x55, 0xAA

%if $ - entry != 512
%error File size is not 512
%endif
