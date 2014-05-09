#
	.text
	.globl	_start
#
_start:
	push	%rbp
	mov	%rsp,%rbp
	sub	$192,%rsp
#
	mov	$12,%rax	# brk
	xor	%rdi,%rdi	#request 0 bytes
	syscall
	call	tohex		#convert hex to ascii
#	==============================
	mov	$13,%rax	#sigaction
	mov	$14,%rdi
	mov	$sigstruct,%rsi
	mov	$0,%rdx
	mov	$8,%r10
	syscall
	call	tohex
#	==============================
	mov	$37,%rax	#alarm
	mov	$1,%rdi		#delay
	syscall
#	==============================
	mov	$34,%rax	#pause
	syscall
#	==============================
	mov	$23,%rax
	call	tohex
#	==============================
	mov	$1,%rax		#exit
	mov	$0,%rdi
	syscall
	hlt
#	==============================
handle_alarm:
	push	%rbp
	mov	%rsp,%rbp
	movl	$0x494E5452,(msgstart)
	pop	%rbp
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
.l1:
	push	%rax
	and	$0x0f,%al
	or	$0x30,%al
	cmp	$0x3a,%al
	jl	.l2
	add	$7,%al
.l2:
	dec	%rbx
	mov	%al,(%rbx)
	pop	%rax
	shr	$4,%rax
	jnz	.l1
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
	.align 8
#
msgstart:
	.ascii	"----0000000000000000"
msgend:
	.byte	0x0a
#
sigstruct:
	.quad	handle_alarm	#handler
	.quad	0,0,0,0,0,0,0,0	#mask
	.quad	0               #flags
	.quad	0,0,0,0,0,0,0,0
	.quad	0,0,0,0,0,0,0,0
	.quad	0,0,0,0,0,0,0,0
	.quad	0,0,0,0,0,0,0,0
#
