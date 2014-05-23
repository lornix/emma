#
	.text
	.global	_start
#
	.func	_start
_start:
#	push	%rbp
#	mov	%rsp,%rbp
#
	mov	$1,%al			# SC write
	mov	$1,%di			# fd=1, stdout
	mov	$msgstart,%esi
	mov	$msglen,%dl
	syscall
#
#	mov	%rbp,%rsp
#	pop	%rbp
#
	movb	$60,%al			# SC exit
	mov	$0,%di			# exit 0
	syscall
	hlt
#
	.endfunc
#	==============================
	.data
	.align 4
#
msgstart:
	.ascii	"Hello World!"
	.byte	0x0a,0
msglen = . - msgstart
