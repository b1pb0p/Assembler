/* data.h
 * data image and base conversions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#ifndef ASSEMBLER_DATA_H
#define ASSEMBLER_DATA_H

#include "utils.h"

#define REGISTER_CH '@'

typedef enum {
    DEFAULT_12BIT,
    REG_DEST,
    REG_SRC,
    REG_REG,
    ADDRESS,
    VALUE
} concat_actions;

typedef struct symbol symbol;

typedef struct {
    char* binary_src;
    char* binary_opcode;
    char* binary_dest;
    char* binary_a_r_e;
    char* base64_word;

    Directive directive;
    concat_actions concat;
    symbol *p_sym;

    int *value;
    int is_word_complete;
    int lc;
    int data_address;

} data_image;

struct symbol {
    char *label;
    char *address_binary;

    int address_decimal;
    int is_missing_info;
    int lc;

    Directive sym_dir;
    data_image *data;
};

char* convert_bin_to_base64(const char* binary);
char* truncate_string(const char* input, int length);
char* decimal_to_binary12(int decimal);

void free_symbol(symbol** symbol_t);
void free_symbol_table(symbol ***p_symbol_table, size_t *size);
void free_data_image(data_image** data);
void free_data_image_array(data_image ***data_array, size_t *size);
void free_strings(int num_strings, ...);

status create_base64_word(data_image* data);
status is_legal_addressing(Command cmd, Addressing_modes src, Addressing_modes dest);

data_image* create_data_image(int lc, int *address);

Addressing_modes get_addressing_mode(file_context *src, const char *word);

#endif
