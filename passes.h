/* passes.h
 * Assembler's first and second pass processes.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include "utils.h"
#include "data.h"

#ifndef ASSEMBLER_PASSES_H
#define ASSEMBLER_PASSES_H

#define INVALID_ADDRESS -1
#define BASE64_CHARS 2
#define A_R_E_BINARY_LEN 2
#define SRC_DEST_OP_BINARY_LEN 3
#define OPCODE_BINARY_LEN 4
#define BINARY_BASE64_BITS 6
#define REGISTER_BINARY_LEN 5
#define ADDRESS_BINARY_LEN 10
#define BINARY_BITS 12
#define AMT_WORD_4 4
#define AMT_WORD_2 2
#define AMT_WORD_1 1

typedef enum {
    DEFAULT_12BIT,
    REG_DEST,
    REG_SRC,
    REG_REG,
    ADDRESS
} concat_actions;

status assembler_first_pass(file_context** contexts);
status update_symbol_info(symbol* existing_symbol, int address);

symbol* create_symbol(const char* label, int address);
symbol* add_symbol(file_context *src, const char* label, int address);
symbol* find_symbol(const char* label);

void free_symbol_table();
#endif
