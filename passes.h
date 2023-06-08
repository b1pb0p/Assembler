/* passes.h
 * Assembler's first and second pass processes.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include "utils.h"
#include "data.h"

#ifndef ASSEMBLER_PASSES_H
#define ASSEMBLER_PASSES_H

#define INVALID_ADDRESS (-1)
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
#define ADDRESS_START 100


status assembler_first_pass(file_context ***contexts);
status assembler_second_pass(file_context ***contexts);
status update_symbol_info(symbol* existing_symbol, int address);
status write_symbol_info(file_context *ctx, symbol *sym);
status generate_obj_output(file_context *obj_file, int ic, int dc);
status process_line(char *p_line, int ic, int dc);
status process_label(const char *label, int ic);
status process_command_word(const char *command, char *p_line, char *first_param, char *second_param);
status skip_white_spaces(char *line);

symbol* add_symbol(file_context *src, const char* label, int address);
symbol* find_symbol(const char* label);

void cleanup_output_files(file_context ***contexts);

int is_label(const char *label);


#endif
