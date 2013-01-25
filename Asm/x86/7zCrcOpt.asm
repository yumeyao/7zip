; 7zCrcOpt.asm -- CRC32 calculation : optimized version
; 2009-12-12 : Igor Pavlov : Public domain

include 7zAsm.asm

ifndef x64
    .386
endif
MY_ASM_START

rD   equ  r2
rN   equ  r7

ifndef x64
g_CrcTable EQU _g_CrcTable
endif

extrn g_CrcTable :near

ifdef x64
    num_VAR     equ r8
    table_VAR   equ r5
else
    data_size   equ (REG_SIZE * 5)
    num_VAR     equ r5
    num_VAR_Arg equ [r4 + data_size]
    table_VAR   equ offset g_CrcTable
endif

SRCDAT  equ  rN + rD + 4 *

CRC macro op:req, dest:req, src:req, t:req
    op      dest, DWORD PTR [src * 4 + 0400h * t + table_VAR]
endm

CRC_XOR macro dest:req, src:req, t:req
    CRC xor, dest, src, t
endm

CRC_MOV macro dest:req, src:req, t:req
    CRC mov, dest, src, t
endm

CRC1b macro
    movzx   x6, BYTE PTR [rD]
    inc     rD
    movzx   x3, x0_L
    xor     x6, x3
    shr     x0, 8
    CRC     xor, x0, r6, 0
    sub     rN, 1
endm

MY_PROLOG macro crc_end:req, crc_end_early:req
    MY_PUSH_4_REGS
    
    mov     x0, x1
ifndef x64
    mov     rN, num_VAR_Arg
    mov     num_VAR, rN
    test    rN, rN
else
    lea     r5, g_CrcTable
    mov     rN, num_VAR
    test    num_VAR, num_VAR
endif
    jz      crc_end_early
  @@:
    test    rD, 7
    jz      @F
    CRC1b
    jnz     @B
  @@:
    cmp     rN, 16
    jb      crc_end
    add     rN, rD
    mov     num_VAR, rN
    sub     rN, 8
    and     rN, NOT 7
    sub     rD, rN
    xor     x0, [SRCDAT 0]
endm

MY_EPILOG macro crc_end:req, crc_end_early:req
    xor     x0, [SRCDAT 0]
    mov     rD, rN
    mov     rN, num_VAR
    sub     rN, rD
  crc_end:
    test    rN, rN
    jz      crc_end_early
  @@:
    CRC1b
    jnz     @B
  crc_end_early:
    MY_POP_4_REGS
endm

ifndef x64
MY_PROC CrcUpdateT8, 3
    MY_PROLOG crc_end_8, crc_end_8_early
    mov     x1, [SRCDAT 1]
ifndef x64
    align 16
endif
  main_loop_8:
    mov     x6, [SRCDAT 2]
    movzx   x3, x1_L
    CRC_XOR x6, r3, 3
    movzx   x3, x1_H
    CRC_XOR x6, r3, 2
    shr     x1, 16
    movzx   x3, x1_L
    movzx   x1, x1_H
    CRC_XOR x6, r3, 1
    movzx   x3, x0_L
    CRC_XOR x6, r1, 0

    mov     x1, [SRCDAT 3]
    CRC_XOR x6, r3, 7
    movzx   x3, x0_H
    shr     x0, 16
    CRC_XOR x6, r3, 6
    movzx   x3, x0_L
    CRC_XOR x6, r3, 5
    movzx   x3, x0_H
    CRC_MOV x0, r3, 4
    xor     x0, x6
    add     rD, 8
    jnz     main_loop_8

    MY_EPILOG crc_end_8, crc_end_8_early
MY_ENDP

db 'Y', 'a', 'o'
endif

MY_PROC CrcUpdateT4, 3
    MY_PROLOG crc_end_4, crc_end_4_early
ifndef x64
    align 16
endif
  main_loop_4:
    movzx   x1, x0_L
    movzx   x3, x0_H
    shr     x0, 16
    movzx   x6, x0_H
    and     x0, 0FFh
    CRC_MOV x1, r1, 3
    xor     x1, [SRCDAT 1]
    CRC_XOR x1, r3, 2
    CRC_XOR x1, r6, 0
    CRC_XOR x1, r0, 1
 
    movzx   x0, x1_L
    movzx   x3, x1_H
    shr     x1, 16
    movzx   x6, x1_H
    and     x1, 0FFh
    CRC_MOV x0, r0, 3
    xor     x0, [SRCDAT 2]
    CRC_XOR x0, r3, 2
    CRC_XOR x0, r6, 0
    CRC_XOR x0, r1, 1
    add     rD, 8
    jnz     main_loop_4

    MY_EPILOG crc_end_4, crc_end_4_early
MY_ENDP

end
