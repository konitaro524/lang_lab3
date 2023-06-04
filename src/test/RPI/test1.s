	.text
	.global	main
main:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 32
	mov	w9, 5
	str	w9, [x29, -4]
	ldr	w8, [x29, -4]
	mov	w9, 1
	add	w8, w8, w9
	str	w8, [x29, -8]
	sub	sp, sp, #16
	str	w9, [sp, 4]
	str	w10, [sp, 0]
	ldr	w8, [x29, -8]
	mov	w0, w8
	bl	put_int
	ldr	w9, [sp, 4]
	ldr	w10, [sp, 0]
	add	sp, sp, 16
_END_main:
	ldp	x29, x30, [sp], 32
	ret

	.section	.rodata
.LC0:
	.string "%d\n"
	.text
put_int:
	stp	x29, x30, [sp, -32]!
	mov	x29, sp
	str	w0, [sp, 28]
	ldr	w1, [sp, 28]
	adrp	x0, .LC0
	add	x0, x0, :lo12:.LC0
	bl	printf
	nop
	ldp	x29, x30, [sp], 32
	ret
