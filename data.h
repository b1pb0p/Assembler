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
    ADDRESS
} concat_actions;

typedef struct {
    char *label;
    char *address_binary;
    int address_decimal;
    int is_missing_info;
} symbol;

typedef struct {
    char* binary_src;
    char* binary_opcode;
    char* binary_dest;
    char* binary_a_r_e;
    char* base64_word;

    directive type; /* TODO: check if necessary */
    concat_actions concat;
    symbol *symbol_t;

    int missing_info;
    int lc;

} data_image;

char* convert_bin_to_base64(const char* binary);
char* truncate_string(const char* input, int length);
char* decimal_to_binary12(int decimal);

void free_symbol(symbol** symbol_t);
void free_symbol_table(symbol ***p_symbol_table, size_t *size);
void free_data_image(data_image** data);
void free_data_image_array(data_image ***data_array, size_t *size);
void free_strings(int num_strings, ...);

status create_base64_word(data_image* data);
status is_legal_addressing(command cmd, addressing_modes src, addressing_modes dest);
symbol* create_symbol(const char* label, int address);

addressing_modes get_addressing_mode(const char *src);

#endif
