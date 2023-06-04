	.text
	.globl	main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$2, %r10d
	movl	%r10d, -4(%rbp)
	movl	$3, %r10d
	movl	%r10d, -8(%rbp)
	movl	$4, %r10d
	movl	%r10d, -12(%rbp)
	movl	$5, %r10d
	movl	%r10d, -16(%rbp)
	movl	-4(%rbp), %eax
	movl	-8(%rbp), %r10d
	imull	%r10d, %eax
	movl	-12(%rbp), %r10d
	movl	-16(%rbp), %r11d
	imull	%r11d, %r10d
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
