	.section	__TEXT,__text
	.globl	_main
_main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$10, %r10d
	movl	%r10d, -4(%rbp)
.L0:
	movl	-4(%rbp), %eax
	cmpl	$0,%eax
	je	.L1
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
	movl	$1, %r10d
	subl	%r10d, %eax
	movl	%eax, -4(%rbp)
	jmp	.L0
.L1:
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
