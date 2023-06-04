	.section	__TEXT,__text
	.globl	sum
sum:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	subq	$16, %rsp
	movl	-4(%rbp), %eax
	movl	-8(%rbp), %r10d
	subl	%r10d, %eax
	jmp	_END_sum
_END_sum:
	leave
	ret

	.globl	_main
_main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	subq	$16, %rsp
	movl	%eax, 8(%rsp)
	movl	%r11d, 0(%rsp)
	movl	$1, %eax
	movl	%eax, %edi
	movl	$2, %eax
	movl	%eax, %esi
	call	sum
	movl	%eax, %r10d
	movl	8(%rsp), %eax
	movl	0(%rsp), %r11d
	addq	$16, %rsp
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
