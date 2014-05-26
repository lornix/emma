#
	.text
#
	.global	_start
#
	.func	_start,_start
_start:
	mov	$plswait.len,%rdx
	mov	$plswait,%rsi
	mov	$1,%rdi
	mov	$1,%rax
	syscall
#
	mov	$bucket,%r8
	xor	%rbx,%rbx
mainloop:
	call	countbits
	incl	4(%r8,%rax,8)
	inc	%ebx
	and	$0xFFFFFFFF,%ebx
	test	%ebx,%ebx
	jnz	mainloop
#
	mov	$0,%r9
showloop:
	mov	$4,%rdx
	lea	0(%r8,%r9,8),%rsi
	mov	$1,%rdi		# stdout
	mov	$1,%rax		# write
	syscall
#
	mov	4(%r8,%r9,8),%eax
#
	mov	$10,%rbx
	mov	$1,%rcx		# counts characters (+LF/0)
	mov	$(buffer+21),%rsi	# end of destination buffer
	movb	$10,(%rsi)
divide:
	xor	%rdx,%rdx	# clear upper bits
	div	%rbx
	or	$0x30,%dl
	dec	%rsi
	mov	%dl,(%rsi)
	inc	%rcx
	test	%rax,%rax	# is it zero?
	jnz	divide		# no? loop around again
#
	mov	%rcx,%rdx	# length
	mov	$1,%rdi		# stdout
	mov	$1,%rax		# write
	syscall
#
	inc	%r9
	cmp	$33,%r9
	jnz	showloop
#
showdone:
	xor	%rdi,%rdi	# no error
	mov	$60,%rax	# system exit
	syscall
#
	.endfunc
#
#
	.func	countbits,countbits
countbits:
	mov	%ebx,%eax
	mov	%eax,%edx
	shr	$1,%eax
	and	$0x55555555,%eax
	sub	%eax,%edx
	mov	%edx,%eax
	shr	$2,%edx
	and	$0x33333333,%eax
	and	$0x33333333,%edx
	add	%edx,%eax
	mov	%eax,%edx
	shr	$4,%eax
	add	%edx,%eax
	and	$0x0f0f0f0f,%eax
	imul	$0x01010101,%eax
	shr	$24,%eax
	ret
#
	.endfunc
#
#
	.data
#
plswait:
	.ascii	"Please wait, counting on my fingers"
	.byte	10
plswait.len = . - plswait
#
buffer:
	.ascii	"Twenty one characters"
bucket:
	.long	0x202d3030,0
	.long	0x202d3130,0
	.long	0x202d3230,0
	.long	0x202d3330,0
	.long	0x202d3430,0
	.long	0x202d3530,0
	.long	0x202d3630,0
	.long	0x202d3730,0
	.long	0x202d3830,0
	.long	0x202d3930,0
	.long	0x202d3031,0
	.long	0x202d3131,0
	.long	0x202d3231,0
	.long	0x202d3331,0
	.long	0x202d3431,0
	.long	0x202d3531,0
	.long	0x202d3631,0
	.long	0x202d3731,0
	.long	0x202d3831,0
	.long	0x202d3931,0
	.long	0x202d3032,0
	.long	0x202d3132,0
	.long	0x202d3232,0
	.long	0x202d3332,0
	.long	0x202d3432,0
	.long	0x202d3532,0
	.long	0x202d3632,0
	.long	0x202d3732,0
	.long	0x202d3832,0
	.long	0x202d3932,0
	.long	0x202d3033,0
	.long	0x202d3133,0
	.long	0x202d3233,0

