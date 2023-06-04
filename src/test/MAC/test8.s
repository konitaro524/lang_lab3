	.section	__TEXT,__text
	.globl	_main
_main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	-4(%rbp), %eax
	movl	-8(%rbp), %r10d
	addl	%r10d, %eax
	jmp	_END_main
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
