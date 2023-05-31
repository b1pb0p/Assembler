/* first_pass.h
 * Assembler's first pass process.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#ifndef ASSEMBLER_FIRST_PASS_H
#define ASSEMBLER_FIRST_PASS_H

#define BINARY_BITS 12
#define BINARY_BASE64_BITS 6
#define BASE64_CHARS 2
#define SRC_DEST_OP_BINARY_LEN 3
#define A_R_E_BINARY_LEN 2
#define OPCODE_BINARY_LEN 4

char* convert_bin_to_base54(const char* binary);
char* truncate_string(const char* input, int length);
char* decimal_to_binary12(int decimal);

#endif
