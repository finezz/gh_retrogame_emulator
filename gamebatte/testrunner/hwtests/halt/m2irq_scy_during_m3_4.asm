.size 8000

.code@100
	jp l150

.code@150
l150:
	ld c, 44
	ld b, 90
l154:
	ldff a, (c)
	cmp a, b
	jpnz l154
	ld hl, 8010
	ld a, ff
	ld(hl++), a
	ld(hl++), a
	ld a, 01
	ld b, 32
	ld hl, 9a40
l167:
	ld(hl++), a
	dec b
	jpnz l167
	ld a, c0
	ldff(47), a
	nop
	ld c, 41
	ld b, 03
l175:
	ldff a, (c)
	and a, b
	cmp a, b
	jpnz l175
	ld a, 20
	ldff(c), a
	ld a, 02
	ldff(ff), a
	ld c, 42
	ld b, 90
l186:
	halt
	xor a, a
	ldff(0f), a
	jp l1000

.code@1000
l1000:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	ldff a, (44)
	ld d, a
	ld a, b
	sub a, d
	ldff(c), a
	xor a, a

.code@102e
	ldff(c), a
	jp l186

