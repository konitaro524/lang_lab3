	.text
	.p2align 2
	.global	_main
_main:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 32
	ldr	w8, [x29, -8]
	ldr	w9, [x29, -4]
	add	w8, w8, w9
	mov	w0, w8
	b	_END_main
_END_main:
	ldp	x29, x30, [sp], 32
	ret

	.text
	.p2align 2
.LC0:
	.string "%d\n"
	.text
	.p2align 2
put_int:
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]
	add	x29, sp, #16
	stur	w0, [x29, #-4]
	ldur	w9, [x29, #-4]
	mov	x8, x9
	adrp	x0, .LC0@PAGE
	add	x0, x0, .LC0@PAGEOFF
	mov	x9, sp
	str	x8, [x9]
	bl	_printf
	ldp	x29, x30, [sp, #16]
	add	sp, sp, #32
	ret
