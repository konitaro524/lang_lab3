	.text
	.global	func
func:
	stp	x29, x30, [sp, -80]!
	add	x29, sp, 80
	str	w0, [x29, -32]
	str	w1, [x29, -36]
	str	w2, [x29, -40]
	str	w3, [x29, -44]
	str	w4, [x29, -48]
	str	w5, [x29, -52]
	str	w6, [x29, -56]
	str	w7, [x29, -60]
	ldr	w8, [x29, -32]
	ldr	w9, [x29, -36]
	add	w8, w8, w9
	ldr	w9, [x29, -40]
	add	w8, w8, w9
	ldr	w9, [x29, -44]
	add	w8, w8, w9
	ldr	w9, [x29, -48]
	add	w8, w8, w9
	ldr	w9, [x29, -52]
	add	w8, w8, w9
	ldr	w9, [x29, -56]
	add	w8, w8, w9
	ldr	w9, [x29, -60]
	add	w8, w8, w9
	ldr	w9, [x29, -16]
	add	w8, w8, w9
	ldr	w9, [x29, -8]
	add	w8, w8, w9
	str	w8, [x29, -28]
	mov	w9, 9
	str	w9, [x29, -24]
	mov	w9, 10
	str	w9, [x29, -20]
	ldr	w8, [x29, -28]
	ldr	w9, [x29, -24]
	add	w8, w8, w9
	ldr	w9, [x29, -20]
	add	w8, w8, w9
	str	w8, [x29, -28]
	sub	sp, sp, #16
	str	w9, [sp, 4]
	str	w10, [sp, 0]
	ldr	w8, [x29, -28]
	mov	w0, w8
	bl	put_int
	ldr	w9, [sp, 4]
	ldr	w10, [sp, 0]
	add	sp, sp, 16
	ldr	w8, [x29, -28]
	b	_END_func
_END_func:
	ldp	x29, x30, [sp], 80
	ret

	.global	main
main:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 32
	sub	sp, sp, #16
	str	w8, [sp, 8]
	str	w10, [sp, 0]
	mov	w8, 1
	mov	w0, w8
	mov	w8, 2
	mov	w1, w8
	mov	w8, 3
	mov	w2, w8
	mov	w8, 4
	mov	w3, w8
	mov	w8, 5
	mov	w4, w8
	mov	w8, 6
	mov	w5, w8
	mov	w8, 7
	mov	w6, w8
	mov	w8, 8
	mov	w7, w8
	mov	w8, 100
	str	w8, [sp, -16]
	mov	w8, 1000
	str	w8, [sp, -8]
	bl	func
	mov	w9, w8
	ldr	w8, [sp, 8]
	ldr	w10, [sp, 0]
	add	sp, sp, 16
	str	w9, [x29, -4]
	sub	sp, sp, #16
	str	w9, [sp, 4]
	str	w10, [sp, 0]
	ldr	w8, [x29, -4]
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
