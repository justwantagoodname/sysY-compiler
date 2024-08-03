.global main
.text
.align 2
.type main, %function
main:
    addi sp, sp, -48
    sd ra, 40(sp)
    sd s0, 32(sp)
    addi s0, sp, 48
    li a5, 3
    sw a5, -16(s0)
    lui a5, %hi(.LC3)
    flw fa5, %lo(.LC3)(a5)
    fsw fa5, -20(s0)
    lui a5, %hi(.LC4)
    flw fa5, %lo(.LC4)(a5)
    fsw fa5, -24(s0)
    lui a5, %hi(.LC5)
    flw fa5, %lo(.LC5)(a5)
    fsw fa5, -28(s0)
    lw a5, -20(s0)
    sw a5, -36(s0)
    lw a5, -24(s0)
    sw a5, -40(s0)
    flw fa3, -36(s0)
    flw fa4, -40(s0)
    fadd.s fa5, fa3, fa4
    fsw fa5, -36(s0)
    lw a5, -36(s0)
    sw a5, -28(s0)
    lw a5, -16(s0)
    sw a5, -44(s0)
    lw a5, -28(s0)
    sw a5, -48(s0)
    lui a5, %hi(STR0)
    addi a0, a5, %lo(STR0)
    lw a1, -44(s0)
    flw fa5, -48(s0)
    fcvt.d.s fa5, fa5
    fmv.x.d a2, fa5
    call putf
    li a0, 12345
    call putint
    li a0, 10
    call putch
    lui a5, %hi(.LC0)
    flw fa5, %lo(.LC0)(a5)
    fmv.s fa0, fa5
    call putfloat
    li a0, 10
    call putch
    lui a5, %hi(STR1)
    addi a0, a5, %lo(STR1)
    lui a5, %hi(.LC1)
    fld fa5, %lo(.LC1)(a5)
    fmv.x.d a1, fa5
    call putf
    lui a5, %hi(STR1)
    addi a0, a5, %lo(STR1)
    lui a5, %hi(.LC2)
    fld fa5, %lo(.LC2)(a5)
    fmv.x.d a1, fa5
    call putf
    lui a5, %hi(STR2)
    addi a0, a5, %lo(STR2)
    li a1, 12345
    lui a5, %hi(.LC1)
    fld fa5, %lo(.LC1)(a5)
    fmv.x.d a2, fa5
    call putf
    lui a5, %hi(STR3)
    addi a0, a5, %lo(STR3)
    call putf
    mv a0, a5
    ld ra, 40(sp)
    ld s0, 32(sp)
    addi sp, sp, 48
    jr ra
.LC5:
    .word 0
.LC3:
    .word 1082130432
.LC4:
    .word 1086324736
.LC0:
    .word 1205840153
STR1:
    .string "%f\n"
STR2:
    .string "Complex string: %d, %f\n"
STR3:
    .string "Goodbye, world!\n"
STR0:
    .string "var arg: %d %f\n"
.LC1:
    .word 1610612736
    .word 1074339512
.LC2:
    .word 1610612736
    .word 1075388088

.section	.note.GNU-stack,"",%progbits
.ident	"SysY-Compiler"

