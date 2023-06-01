/* first_pass.h
 * Assembler's first pass process.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#ifndef ASSEMBLER_FIRST_PASS_H
#define ASSEMBLER_FIRST_PASS_H

#define BASE64_CHARS 2
#define A_R_E_BINARY_LEN 2
#define SRC_DEST_OP_BINARY_LEN 3
#define OPCODE_BINARY_LEN 4
#define BINARY_BASE64_BITS 6
#define REGISTER_BINARY_LEN 5
#define ADDRESS_BINARY_LEN 10
#define BINARY_BITS 12

typedef enum {
    DEFAULT_12BIT,
    REG_DEST,
    REG_SRC,
    REG_REG,
    ADDRESS
} concat_actions;

char* convert_bin_to_base54(const char* binary);
char* truncate_string(const char* input, int length);
char* decimal_to_binary12(int decimal);
char* concat_12bits(const char* src_op, const char* opcode, const char* dest_op, const char* a_r_e);

#endif
