/* first_pass.h
 * Assembler's first pass process.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include "utils.h"

#ifndef ASSEMBLER_PASSES_H
#define ASSEMBLER_PASSES_H

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

status assembler_first_pass(file_context** contexts);
#endif
