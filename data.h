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
} Concat_mode;

typedef struct symbol symbol;

typedef struct {
    char* binary_src;
    char* binary_opcode;
    char* binary_dest;
    char* binary_a_r_e;
    char* base64_word;

    Directive directive;
    Concat_mode concat;
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

char* decimal_to_binary12(int decimal);
char* binary12_to_base64(const char* binary);
char* truncate_string(const char* input, int length);

void free_symbol(symbol** symbol_t);
void free_data_image(data_image** data);
void free_strings(int num_strings, ...);
void free_symbol_table(symbol ***p_symbol_table, size_t *size);
void free_data_image_array(data_image ***data_array, size_t *size);

int is_legal_addressing(file_context *src, Command cmd, Adrs_mod src_op, Adrs_mod dest_op, status *report);

status create_base64_word(data_image* data);
status handle_address_reference(data_image *data,  symbol *sym);
status handle_register_data_img(data_image *data, Concat_mode con_act, char *reg, ...);
status get_concat_mode(Adrs_mod src_op, Adrs_mod dest_op, Concat_mode *cn1, Concat_mode *cn2);
status process_data_img_dec(data_image *data, Adrs_mod src_op, Command opcode, Adrs_mod dest_op, ARE are);

ARE get_are(symbol *sym);

data_image* create_data_image(int lc, int *address);
data_image *assemble_operand_data_img(file_context *src, Concat_mode con_md, Adrs_mod mode, char* word, ...);

Concat_mode get_concat_mode_one_op(Adrs_mod src_op, Adrs_mod dest_op);

Adrs_mod get_addressing_mode(file_context *src, char *word, size_t word_len, status *report);

#endif
