	.text
	.global	main
main:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 32
	ldr	w8, [x29, -8]
	ldr	w9, [x29, -4]
	add	w8, w8, w9
	ldr	w9, [x29, -12]
	add	w9, w9, w8
	ldr	w8, [x29, -16]
	add	w8, w8, w9
	str	w8, [x29, -16]
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
