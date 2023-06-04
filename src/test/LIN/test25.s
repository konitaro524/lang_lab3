	.text
	.globl	func
func:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%edi, -16(%rbp)
	movl	%esi, -20(%rbp)
	movl	%edx, -24(%rbp)
	movl	%ecx, -28(%rbp)
	movl	%r8d, -32(%rbp)
	movl	%r9d, -36(%rbp)
	subq	$48, %rsp
	movl	-16(%rbp), %eax
	movl	-20(%rbp), %r10d
	addl	%r10d, %eax
	movl	-24(%rbp), %r10d
	addl	%r10d, %eax
	movl	-28(%rbp), %r10d
	addl	%r10d, %eax
	movl	-32(%rbp), %r10d
	addl	%r10d, %eax
	movl	-36(%rbp), %r10d
	addl	%r10d, %eax
	movl	16(%rbp), %r10d
	addl	%r10d, %eax
	movl	24(%rbp), %r10d
	addl	%r10d, %eax
	movl	%eax, -4(%rbp)
	movl	$9, %r10d
	movl	%r10d, -8(%rbp)
	movl	$10, %r10d
	movl	%r10d, -12(%rbp)
	movl	-4(%rbp), %eax
	movl	-8(%rbp), %r10d
	addl	%r10d, %eax
	movl	-12(%rbp), %r10d
	addl	%r10d, %eax
	movl	%eax, -4(%rbp)
	subq	$16, %rsp
	movl	%r10d, 4(%rsp)
	movl	%r11d, 0(%rsp)
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	put_int
	movl	4(%rsp), %r10d
	movl	0(%rsp), %r11d
	addq	$16, %rsp
	movl	-4(%rbp), %eax
	jmp	_END_func
_END_func:
	leave
	ret

	.globl	main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	subq	$32, %rsp
	movl	%eax, 24(%rsp)
	movl	%r11d, 16(%rsp)
	movl	$1, %eax
	movl	%eax, %edi
	movl	$2, %eax
	movl	%eax, %esi
	movl	$3, %eax
	movl	%eax, %edx
	movl	$4, %eax
	movl	%eax, %ecx
	movl	$5, %eax
	movl	%eax, %r8d
	movl	$6, %eax
	movl	%eax, %r9d
	movl	$7, %eax
	movl	%eax, 0(%rsp)
	movl	$8, %eax
	movl	%eax, 8(%rsp)
	call	func
	movl	%eax, %r10d
	movl	24(%rsp), %eax
	movl	16(%rsp), %r11d
	addq	$32, %rsp
	movl	%r10d, -4(%rbp)
	subq	$16, %rsp
	movl	%r10d, 4(%rsp)
	movl	%r11d, 0(%rsp)
	movl	-4(%rbp), %eax
	movl	%eax, %edi
	call	put_int
	movl	4(%rsp), %r10d
	movl	0(%rsp), %r11d
	addq	$16, %rsp
_END_main:
	leave
	ret

	.section	.rodata
.LC0:
	.string "%d\n"
	.text
put_int:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16,%rsp
	movl	%edi, -4(%rbp)
	movl	-4(%rbp), %esi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	leave
	ret
