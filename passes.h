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
#define DEFAULT_DATA_IMAGE_CAP 5
#define BINARY_BITS 12
#define AMT_WORD_4 4
#define AMT_WORD_2 2
#define AMT_WORD_1 1
#define ADDRESS_START 100

status is_data_valid(const char *word);
status is_valid_label(const char *label);
status assembler_first_pass(file_context **src);
status assembler_second_pass(file_context **src);
status process_line(file_context *src, char *p_line);
status process_data(file_context *src, char *label, char *line);
status update_symbol_info(symbol* existing_symbol, int address);
status generate_obj_output(const char *file_name, size_t ic, size_t dc);
status generate_directive_output(const char *file_name, char *ext, Directive target);
status process_command_word(file_context *src, char *p_line, char *first_param, char *second_param);

symbol* find_symbol(const char* label);
symbol *declare_label(file_context *src, const char *label, size_t label_len);
symbol* add_symbol(file_context *src, const char* label, int address, status *report);

data_image* add_data_image_default(file_context *src, const char* label, int address, status *report);

void cleanup(file_context **src);
void free_global_data_and_symbol();
void process_line_w_label(file_context *src,char *line, symbol *sym, status *report);

int is_valid_register(const char* str);
int is_dot_directive(const char *word, const char *directive);
int is_label(file_context *src, const char *label, status *report);


char* create_dummy_label();
#endif
