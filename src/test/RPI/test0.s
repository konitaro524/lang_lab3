	.text
	.global	main
main:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 16
_END_main:
	ldp	x29, x30, [sp], 16
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
