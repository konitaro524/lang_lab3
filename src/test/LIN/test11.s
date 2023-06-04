	.text
	.globl	main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	-12(%rbp), %eax
	movl	-16(%rbp), %r10d
	addl	%r10d, %eax
	movl	-8(%rbp), %r10d
	addl	%eax, %r10d
	movl	-4(%rbp), %eax
	addl	%r10d, %eax
	movl	%eax, -4(%rbp)
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
