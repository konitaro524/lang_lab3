	.text
	.p2align 2
	.global	_main
_main:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 32
	mov	w9, 1
	str	w9, [x29, -16]
	mov	w9, 2
	str	w9, [x29, -12]
	mov	w9, 3
	str	w9, [x29, -8]
	mov	w9, 4
	str	w9, [x29, -4]
	ldr	w8, [x29, -16]
	ldr	w9, [x29, -12]
	add	w8, w8, w9
	ldr	w9, [x29, -8]
	add	w8, w8, w9
	ldr	w9, [x29, -4]
	add	w8, w8, w9
	str	w8, [x29, -16]
	sub	sp, sp, #16
	str	w9, [sp, 4]
	str	w10, [sp, 0]
	ldr	w8, [x29, -16]
	mov	w0, w8
	bl	put_int
	ldr	w9, [sp, 4]
	ldr	w10, [sp, 0]
	add	sp, sp, 16
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
