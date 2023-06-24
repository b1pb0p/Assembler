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

status is_valid_label(const char *label);
status assembler_first_pass(file_context **src);
status assembler_second_pass(file_context **src);
status process_line(file_context *src, char *p_line);
status update_symbol_info(symbol* existing_symbol, int address);
status generate_obj_output(const char *file_name, size_t ic, size_t dc);
status string_parser(file_context *src, char **word, char *ch, status *report);
status generate_directive_output(const char *file_name, char *ext, Directive target);
status assert_value_to_data(file_context *src, Directive dir, Value val_type ,char *word, int **value,
                            data_image **p_data, status *report);

symbol* find_symbol(const char* label);
symbol* add_symbol(file_context *src, const char* label, int address, status *report);
symbol *declare_label(file_context *src, char *label, size_t label_len, status *report);

Value validate_data(file_context *src, char *word, size_t length, status *report);
Value validate_string(file_context *src, char *word, size_t length, status *report);
Value line_parser(file_context *src, Directive dir, char **line, char *word, status *report);

data_image* add_data_image(file_context *src, Directive dir, const char* label, status *report);

void cleanup(file_context **src);
void free_global_data_and_symbol();
void process_data(file_context *src, const char *label, char *line, status *report);
void process_string(file_context *src, const char *label, char *line, status *report);
void handle_processing_line(file_context *src, char *line, symbol *sym, status *report);
void assert_data_img_by_label(file_context *src, Directive dir, const char *label, int *flag,
                                                int **value, data_image **p_data, status *report);
int is_valid_register(const char* str);
int is_valid_string(char **line, char **word, status *report);
int is_label(file_context *src, const char *label, status *report);

#endif
