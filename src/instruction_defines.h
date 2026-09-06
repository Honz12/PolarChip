// This will have the ISA opcodes

/*
    Mísc
*/

#define INST_NOP    0x00 // No operation
#define INST_RET    0x01 // Return from subroutine
#define INST_CALL   0x02 // Call subroutine
#define INST_INT    0x03 // Call interrupt

/*
    Load
*/

#define INST_LDI8   0x04
#define INST_LDI16  0x05
#define INST_LDI24  0x06
#define INST_LDI32  0x07

#define INST_LD8    0x08
#define INST_LD16   0x09
#define INST_LD24   0x0a
#define INST_LD32   0x0b

#define INST_LDP8   0x0c
#define INST_LDP16  0x0d
#define INST_LDP24  0x0e
#define INST_LDP32  0x0f

/*
    Store
*/

#define INST_ST8    0x10
#define INST_ST16   0x11
#define INST_ST24   0x12
#define INST_ST32   0x13

#define INST_STP8   0x14
#define INST_STP16  0x15
#define INST_STP24  0x16
#define INST_STP32  0x17

/*
    Arithmetic
*/

/* Binary */

#define INST_ADD    0x18
#define INST_SUB    0x19

#define INST_MUL    0x1a
#define INST_MULS   0x1b

#define INST_DIV    0x1c
#define INST_DIVS   0x1d

#define INST_MOD   0x1e
#define INST_MODS  0x1f

#define INST_AND    0x20
#define INST_NAND   0x21

#define INST_OR    0x22
#define INST_NOR   0x23
#define INST_XOR   0x24

//#define _         0x25
//#define _         0x26
//#define _         0x27

/* Unary */

#define INST_INC    0x28 // Increment
#define INST_DEC    0x29 // Decrement

#define INST_INCM   0x2a // Increment and move
#define INST_DECM   0x2b // Decrement and move

//#define _         0x2c
//#define _         0x2d
//#define _         0x2e
//#define _         0x2f

/*
    QMEM and stack manipulation
*/

#define INST_MOV    0x30 // Move value
#define INST_NUL    0x31 // Nullout cell

#define INST_PUSH   0x32 // Push to stack (4 bytes)
#define INST_POP    0x33 // Push to stack (4 bytes)
