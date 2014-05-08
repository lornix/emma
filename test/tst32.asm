;
	bits	32
;
	section	.text
;
	global	_start
;
_start:
	push	ebp
	mov	ebp,esp
	sub	esp,192
;
	mov	eax,45		;brk (32bit)
	xor	ebx,ebx		;request 0 bytes
	int	0x80
	call	tohex		;convert hex to ascii
;
;	==============================
	mov	eax,67		;sigaction (32bit)
	mov	ebx,14
	mov	ecx,sigstruct
	mov	edx,0
	mov	esi,8
	int	0x80
	call	tohex
;
;	==============================
;
	mov	eax,27		;alarm (32bit)
	mov	ebx,1		;delay
	int	0x80
;
;
	mov	eax,29		;pause (32bit)
	int	0x80
	nop
	call	tohex
;
.done:
	mov	eax,1		;exit (32bit)
	mov	ebx,0
	int	0x80
	hlt
;
handle_alarm:
	push	ebp
	mov	ebp,esp
	mov	dword [msgstart],'INTR'
	pop	ebp
	ret
;
; convert least nybble to ascii/hex, store [rbx]
; loop until rax==0
tohex:
	push	eax
	mov	dword [msgstart+ 4],'++++'
	mov	dword [msgstart+ 8],'++++'
	mov	dword [msgstart+12],'++++'
	mov	dword [msgstart+16],'++++'
	mov	ebx,msgend
.l1:
	push	eax
	and	al,0x0f
	or	al,0x30
	cmp	al,0x3a
	jl	.l2
	add	al,7
.l2:
	dec	ebx
	mov	[ebx],al
	pop	eax
	shr	eax,4
	jnz	.l1
;
	pop	eax
show:
	push	eax
;
	mov	eax,4		;write
	mov	ebx,1		;fd=1, stdout
	mov	ecx,msgstart
	mov	edx,msgend-msgstart+1
	int	0x80
;
	pop	eax
	ret
;
	section	.data align=8
;
msgstart:
	db	'----0000000000000000'
msgend:
	db	0x0a
;
SA_RESETHAND	equ	0x80000000
SA_SIGINFO	equ	0x00000004
;
sigstruct:
	dd	handle_alarm	;handler
	dd	0               ;flags
	dd	0,0,0,0,0,0,0,0	;mask
	dd	0		;not used
;
zzzend:	equ	$

