	.section	__TEXT,__text
	.globl	_main
_main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
	movl	$1, %r10d
	movl	%r10d, -4(%rbp)
	movl	$2, %r10d
	movl	%r10d, -8(%rbp)
	movl	$3, %r10d
	movl	%r10d, -12(%rbp)
	movl	$4, %r10d
	movl	%r10d, -16(%rbp)
	movl	$5, %r10d
	movl	%r10d, -20(%rbp)
	movl	$6, %r10d
	movl	%r10d, -24(%rbp)
	movl	-4(%rbp), %eax
	movl	-8(%rbp), %r10d
	addl	%r10d, %eax
	movl	-12(%rbp), %r10d
	movl	-16(%rbp), %r11d
	addl	%r11d, %r10d
	addl	%r10d, %eax
	movl	-20(%rbp), %r10d
	movl	-24(%rbp), %r11d
	addl	%r11d, %r10d
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

	.section	__TEXT,__cstring
.LC0:
	.string "%d\n"
	.section	__TEXT,__text
put_int:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16,%rsp
	leaq	.LC0(%rip), %rax
	movl	%edi, -4(%rbp)
	movl	-4(%rbp), %esi
	movq	%rax, %rdi
	movb	$0, %al
	callq	_printf
	movl	%eax, -8(%rbp)
	addq	$16, %rsp
	popq	%rbp
	retq
