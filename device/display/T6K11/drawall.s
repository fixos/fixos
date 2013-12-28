!
!	Revolution-FX - Hardware Library for the fx-9860SD, fx-9860G, fx-9860AU, Graph85 and Graph85SD
!	Written by kucalc
!	Modified by Kristaba
!	
!	Current Version: v1.0 GNU Toolchain
!	Contact: kucalc@gmail.com
!	Homepage: http://revolution-fx.sourceforge.net
!	Official Support: http://www.casiocalc.org
!	Wiki: http://revolution-fx.sourceforge.net/wiki
!	
!	This work is licensed under the Creative Commons Attribution-Noncommercial-Share Alike 3.0 License. 
!	To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a 
!	letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
!

	.section ".text"
	.global _disp_mono_copy_to_dd
	.type _disp_mono_copy_to_dd, @function

_disp_mono_copy_to_dd:
	mov.l	r12, @-r15
	mov.l	r11, @-r15
	mov.l	r10, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15

	mov	r4, r10

	mov	#4, r0
	mov.l	DrawAllValue1, r4
	mov.l	DrawAllValue2, r5
	mov	#1, r7

	mov	#0x40, r8
	mov	#0x10, r9

	mov	#7, r11
	mov	#0, r12
	mov	r12, r1

   SelectLine:
	mov	r9, r6
	mov.b	r0, @r4
	mov.w	SelectLineValue1, r3
	or	r1, r3
	mov.b	r3, @r5
	mov.b	r7, @r4
	mov.b	r7, @r5
	mov.b	r0, @r4
	mov.b	r12, @r5

   WriteLine:
	mov.b	r11, @r4
	dt	r6
	mov.b	@r10+, r3
	bf/s	WriteLine
	mov.b	r3, @r5
	add	#1, r1
	cmp/ge	r8, r1
	bf	SelectLine

	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r10
	mov.l	@r15+, r11
	mov.l	@r15+, r12
	rts
	nop

	.align 2
SelectLineValue1:
	.word 0xC0


	.align 4
DrawAllValue1:
	.long 0xB4000000
DrawAllValue2:
	.long 0xB4010000

	.end
