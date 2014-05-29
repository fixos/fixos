/* Copyright (C) 1994, 1995, 1997, 1998, 1999, 2000, 2001, 2002, 2003,
   2004, 2005, 2006
   Free Software Foundation, Inc.

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs,
and to distribute those combinations without any restriction coming
from the use of this file.  (The General Public License restrictions
do apply in other respects; for example, they cover modification of
the file, and distribution when not linked into a combine
executable.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.  */

!! libgcc routines for the Renesas / SuperH SH CPUs.
!! Contributed by Steve Chamberlain.
!! sac@cygnus.com

!! ashiftrt_r4_x, ____ashrsi3, ____ashlsi3, ____lshrsi3 routines
!! recoded in assembly by Toshiyasu Morita
!! tm@netcom.com

/* SH2 optimizations for ____ashrsi3, ____ashlsi3, ____lshrsi3 and
   ELF local label prefixes by J"orn Rennecke
   amylaar@cygnus.com  */

	.text
	.balign	4
	.global	___movmem
	.global ___movstr
	.set ___movstr, ___movmem	
	/* This would be a lot simpler if r6 contained the byte count
	   minus 64, and we wouldn't be called here for a byte count of 64.  */
___movmem:
	sts.l	pr,@-r15
	shll2	r6
	bsr	___movmemSI52+2
	mov.l	@(48,r5),r0
	.balign	4
movmem_loop: /* Reached with rts */
	mov.l	@(60,r5),r0
	add	#-64,r6
	mov.l	r0,@(60,r4)
	tst	r6,r6
	mov.l	@(56,r5),r0
	bt	movmem_done
	mov.l	r0,@(56,r4)
	cmp/pl	r6
	mov.l	@(52,r5),r0
	add	#64,r5
	mov.l	r0,@(52,r4)
	add	#64,r4
	bt	___movmemSI52
! done all the large groups, do the remainder
! jump to movmem+
	mova	___movmemSI4+4,r0
	add	r6,r0
	jmp	@r0
movmem_done: ! share slot insn, works out aligned.
	lds.l	@r15+,pr
	mov.l	r0,@(56,r4)
	mov.l	@(52,r5),r0
	rts
	mov.l	r0,@(52,r4)
	.balign	4

	.global	___movmemSI64
	.global ___movstrSI64
	.set	___movstrSI64, ___movmemSI64
___movmemSI64:
	mov.l	@(60,r5),r0
	mov.l	r0,@(60,r4)
	.global	___movmemSI60
	.global ___movstrSI60
	.set	___movstrSI60, ___movmemSI60
___movmemSI60:
	mov.l	@(56,r5),r0
	mov.l	r0,@(56,r4)
	.global	___movmemSI56
	.global ___movstrSI56
	.set	___movstrSI56, ___movmemSI56
___movmemSI56:
	mov.l	@(52,r5),r0
	mov.l	r0,@(52,r4)
	.global	___movmemSI52
	.global ___movstrSI52
	.set	___movstrSI52, ___movmemSI52
___movmemSI52:
	mov.l	@(48,r5),r0
	mov.l	r0,@(48,r4)
	.global	___movmemSI48
	.global	___movstrSI48
	.set	___movstrSI48, ___movmemSI48
___movmemSI48:
	mov.l	@(44,r5),r0
	mov.l	r0,@(44,r4)
	.global	___movmemSI44
	.global	___movstrSI44
	.set	___movstrSI44, ___movmemSI44
___movmemSI44:
	mov.l	@(40,r5),r0
	mov.l	r0,@(40,r4)
	.global	___movmemSI40
	.global ___movstrSI40
	.set	___movstrSI40, ___movmemSI40
___movmemSI40:
	mov.l	@(36,r5),r0
	mov.l	r0,@(36,r4)
	.global	___movmemSI36
	.global	___movstrSI36
	.set	___movstrSI36, ___movmemSI36
___movmemSI36:
	mov.l	@(32,r5),r0
	mov.l	r0,@(32,r4)
	.global	___movmemSI32
	.global	___movstrSI32
	.set	___movstrSI32, ___movmemSI32
___movmemSI32:
	mov.l	@(28,r5),r0
	mov.l	r0,@(28,r4)
	.global	___movmemSI28
	.global	___movstrSI28
	.set	___movstrSI28, ___movmemSI28
___movmemSI28:
	mov.l	@(24,r5),r0
	mov.l	r0,@(24,r4)
	.global	___movmemSI24
	.global	___movstrSI24
	.set	___movstrSI24, ___movmemSI24
___movmemSI24:
	mov.l	@(20,r5),r0
	mov.l	r0,@(20,r4)
	.global	___movmemSI20
	.global	___movstrSI20
	.set	___movstrSI20, ___movmemSI20
___movmemSI20:
	mov.l	@(16,r5),r0
	mov.l	r0,@(16,r4)
	.global	___movmemSI16
	.global	___movstrSI16
	.set	___movstrSI16, ___movmemSI16
___movmemSI16:
	mov.l	@(12,r5),r0
	mov.l	r0,@(12,r4)
	.global	___movmemSI12
	.global	___movstrSI12
	.set	___movstrSI12, ___movmemSI12
___movmemSI12:
	mov.l	@(8,r5),r0
	mov.l	r0,@(8,r4)
	.global	___movmemSI8
	.global	___movstrSI8
	.set	___movstrSI8, ___movmemSI8
___movmemSI8:
	mov.l	@(4,r5),r0
	mov.l	r0,@(4,r4)
	.global	___movmemSI4
	.global	___movstrSI4
	.set	___movstrSI4, ___movmemSI4
___movmemSI4:
	mov.l	@(0,r5),r0
	rts
	mov.l	r0,@(0,r4)

	.global	___movmem_i4_even
	.global	___movstr_i4_even
	.set	___movstr_i4_even, ___movmem_i4_even

	.global	___movmem_i4_odd
	.global	___movstr_i4_odd
	.set	___movstr_i4_odd, ___movmem_i4_odd

	.global	___movmemSI12_i4
	.global	___movstrSI12_i4
	.set	___movstrSI12_i4, ___movmemSI12_i4

	.p2align	5
L_movmem_2mod4_end:
	mov.l	r0,@(16,r4)
	rts
	mov.l	r1,@(20,r4)

	.p2align	2

___movmem_i4_even:
	mov.l	@r5+,r0
	bra	L_movmem_start_even
	mov.l	@r5+,r1

___movmem_i4_odd:
	mov.l	@r5+,r1
	add	#-4,r4
	mov.l	@r5+,r2
	mov.l	@r5+,r3
	mov.l	r1,@(4,r4)
	mov.l	r2,@(8,r4)

L_movmem_loop:
	mov.l	r3,@(12,r4)
	dt	r6
	mov.l	@r5+,r0
	bt/s	L_movmem_2mod4_end
	mov.l	@r5+,r1
	add	#16,r4
L_movmem_start_even:
	mov.l	@r5+,r2
	mov.l	@r5+,r3
	mov.l	r0,@r4
	dt	r6
	mov.l	r1,@(4,r4)
	bf/s	L_movmem_loop
	mov.l	r2,@(8,r4)
	rts
	mov.l	r3,@(12,r4)

	.p2align	4
___movmemSI12_i4:
	mov.l	@r5,r0
	mov.l	@(4,r5),r1
	mov.l	@(8,r5),r2
	mov.l	r0,@r4
	mov.l	r1,@(4,r4)
	rts
	mov.l	r2,@(8,r4)
