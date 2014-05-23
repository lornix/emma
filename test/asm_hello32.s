#
	.text
	.global	_start
#
	.func	_start
_start:
	mov	$1,%eax			# SC write
	mov	$1,%ebx			# fd=1, stdout
	mov	$msgstart,%ecx
	mov	$msglen,%edx
	int	$0x80
#
	mov	$60,%eax		# SC exit
	mov	$0,%ebx			# exit 0
	int	$0x80
	hlt
#
	.endfunc
#	==============================
	.data
	.align 4
#
msgstart:
	.ascii	"Hello World!"
	.byte	0x0a
msglen = . - msgstart
