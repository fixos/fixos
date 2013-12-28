	.section ".text"
	.global _disp_mono_set_pixel
	.type _disp_mono_set_pixel, @function
	.align 2

! setPixel() function, C prototype : void set_pixel(int x, int y, int color, unsigned char *vram)
! color is 0 for white, 1 for black, 2 for XOR
_disp_mono_set_pixel:
	! Clipping : (x>=0 && y>=0 && 63>=y && 127>=x)
	mov	#0, r1
	cmp/ge	r1, r4
	movt	r0
	cmp/ge	r1, r5
	movt	r2
	and	r2, r0
	mov	#63, r1
	cmp/ge	r5, r1
	movt	r2
	and	r2, r0
	mov	#127, r1
	cmp/ge	r4, r1
	movt	r2
	and	r2, r0
	tst	r0, r0
	bt	End		! Out of screen boudaries.

				! VRAM adress is in r7
	mov	r4, r2
	mov	r5, r3
	mov	#4, r1
	shld	r1, r3
	mov	#-3, r1
	shld	r1, r2
	add	r2, r3		! VRAM offset in r3

	mov	#7, r1
	and	r1, r4		! Equivalent to x%8
	sub	r4, r1
	mov	#1, r4
	shld	r1, r4		! Pixel position in byte in r4

	mov	r6, r0
	cmp/eq	#0, r0
	bt	ColorWhite
	cmp/eq	#1, r0
	bt	ColorBlack	
	cmp/eq	#2, r0
	bt	ColorXor
	bra	End
	nop

ColorWhite:
	mov	r7, r0
	mov.b	@(r0, r3), r1
	not	r4, r4
	and	r4, r1
	mov.b	r1, @(r0, r3)
	bra	End
	nop

ColorBlack:
	mov	r7, r0
	mov.b	@(r0, r3), r1
	or	r4, r1
	mov.b	r1, @(r0, r3)
	bra	End
	nop

ColorXor:
	mov	r7, r0
	mov.b	@(r0, r3), r1
	xor	r4, r1
	mov.b	r1, @(r0, r3)

End:
	rts
	nop

