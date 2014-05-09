#
	.text
	.globl	_start
#
_start:
	push	%rbp
	mov	%rsp,%rbp
	sub	$16,%rsp
#	==============================
	mov	$12,%rax	# brk
	xor	%rdi,%rdi	#request 0 bytes
	syscall
	movl	$0x2d6b7262,msgstart
	call	tohex
#	==============================
	mov	$handle_alarm,%rax
	mov	%rax,_handler
	movl	$0x6c646e68,msgstart
	call	tohex
	mov	$sigstruct,%rax
	movl	$0x63727473,msgstart
	call	tohex
#	==============================
	mov	$13,%rax	#sigaction
	mov	$14,%rdi
	mov	$sigstruct,%rsi
	mov	$0,%rdx
	mov	$8,%r10
	syscall
	movl	$0x61676973,msgstart
	call	tohex
#	==============================
	mov	$37,%rax	#alarm
	mov	$1,%rdi		#delay
	syscall
	movl	$0x6d726c61,msgstart
	call	tohex
#	==============================
	mov	$34,%rax	#pause
	syscall
#	==============================
	movl	$0x73756170,msgstart
	call	tohex
#	==============================
	mov	$23,%rax
	movl	$0x2d646e72,msgstart
	call	tohex
#	==============================
	pop	%rsp
	mov	$60,%rax		#exit
	mov	$0,%rdi
	syscall
	hlt
#	==============================
handle_alarm:
	movl	$0x52544E49,msgstart+4
	mov	%rdi,%rax
	call	tohex
	ret
#	==============================
tohex:
	push	%rax
	mov	$msgstart,%rbx
	movl	$0x2b2b2b2b, 4(%rbx,1)
	movl	$0x2b2b2b2b, 8(%rbx,1)
	movl	$0x2b2b2b2b,12(%rbx,1)
	movl	$0x2b2b2b2b,16(%rbx,1)
	mov	$msgend,%rbx
_l1:
	push	%rax
	and	$0x0f,%al
	or	$0x30,%al
	cmp	$0x3a,%al
	jl	_l2
	add	$39,%al
_l2:
	dec	%rbx
	mov	%al,(%rbx)
	pop	%rax
	shr	$4,%rax
	jnz	_l1
#
	mov	$1,%rax		#write
	mov	$1,%rdi		#fd=1, stdout
	mov	$msgstart,%rsi
	mov	$msgend-msgstart+1,%rdx
	syscall
#
	pop	%rax
	ret
#
#
	.data
	.align 4
#
msgstart:
	.ascii	"----0000000000000000"
msgend:
	.byte	0x0a
#
	.align 16
sigstruct:
_handler:
	.quad	0
_mask:
	.quad	0,0,0,0,0,0,0,0
	.quad	0,0,0,0,0,0,0,0
_flags:
	.long	0
_other:
	.long	0
	.long	0
	.long	0
_strend	= .
#
