#
	.text
	.global	_start
#
	.func	_start
_start:
	push	%rbp
	mov	%rsp,%rbp
#	==============================
	mov	$12,%rax		# SC brk
	xor	%rdi,%rdi		# request 0 bytes
	syscall
	movl	$0x2d6b7262,(msgstart)	# brk-
	call	tohex
#	==============================
	mov	$13,%rax		# SC sigaction
	mov	$10,%rdi		# SIGUSR1
	mov	$sigstruct,%rsi		# new sigaction struct *
	mov	$0,%rdx			# old sigaction struct *
	mov	$8,%r10			# size of sigset in 64bit units?
	syscall
	movl	$0x61676973,(msgstart)	# siga
	call	tohex
#	==============================
	mov	$62,%rax		# SC kill
	mov	$0,%rdi			# my process group?
	mov	$10,%rsi		# signal == SIGUSR1
# disabled for now, causes termination
#	syscall
#	==============================
	mov	$23,%rax
	movl	$0x2d646e72,(msgstart)	# rnd-
	call	tohex
#	==============================
	pop	%rbp
	mov	$60,%rax		# SC exit
	mov	$13,%rdi
	syscall
	hlt
	.endfunc
#	==============================
	.func	handle_alarm
handle_alarm:
	push	%rbp
	mov	%rsp,%rbp
	movl	$0x52544E49,(msgstart+4)	# INTR
	pop	%rbp
	ret
	.endfunc
#	==============================
	.func	restorer
restorer:
	push	%rbp
	mov	%rsp,%rbp
	pop	%rbp
	ret
	.endfunc
#	==============================
	.func	tohex,tohex
tohex:
	push	%rbp
	mov	%rsp,%rbp
	push	%rbx
	push	%rax
	movl	$0x20202020,(msgval)
	movl	$0x20202020,(msgval+ 4)
	movl	$0x20202020,(msgval+ 8)
	movl	$0x20202020,(msgval+12)
	mov	$msgend,%rbx
1:
	push	%rax
	and	$0x0f,%al
	or	$0x30,%al
	cmp	$0x3a,%al
	jl	2f
	add	$39,%al
2:
	dec	%rbx
	mov	%al,(%rbx)
	pop	%rax
	shr	$4,%rax
	jnz	1b
#
	push	%rdi
	push	%rsi
	push	%rdx
	mov	$1,%rax			# SC write
	mov	$1,%rdi			# fd=1, stdout
	mov	$msgstart,%rsi
	mov	$(msgend-msgstart+1),%rdx
	syscall
	pop	%rdx
	pop	%rsi
	pop	%rdi
#
	pop	%rax
	pop	%rbx
	pop	%rbp
	ret
	.endfunc
#
#
	.data
	.align 4
#
msgstart:
	.ascii	"----"
	.ascii	" "
msgval:
	.ascii	"----------------"
msgend:
	.byte	0x0a
#
	.align 16
sigstruct:
sa_handler:	# +0, 8 bytes
	.quad	handle_alarm
sa_mask:	# +8, 128 bytes
	.quad	0x200,0,0,0,0,0,0,0
	.quad	00000,0,0,0,0,0,0,0
sa_flags:	# +136, 4 bytes
	.long	0
sa_restorer:	# +140, 8 bytes
	.quad	restorer
endofstat:
	.long	-1
#
# Addr of sa: 7fffb12d2f70
# Addr of sa.sa_handler:  0x00 (8 bytes)
# Addr of sa.sa_mask:     0x08 (128 bytes)
# Addr of sa.sa_flags:    0x88 (4 bytes)
# Addr of sa.sa_restorer: 0x90 (8 bytes)
#
