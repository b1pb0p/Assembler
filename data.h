/* data.h
 * data image and base conversions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#ifndef ASSEMBLER_DATA_H
#define ASSEMBLER_DATA_H
#include "passes.h"

#define REGISTER_CH '@'

typedef struct {
    char *name;
    char *address;
    char **values; /* can be multiple (comma seperated/ strings) or singular */

    directive type; /* TODO: check if necessary */
} symbol;

typedef struct {
    char* binary_src;
    char* binary_opcode;
    char* binary_dest;
    char* binary_a_r_e;
    char* binary_word;
    char* base64_word;

    concat_actions concat;
    symbol *symbol_t;

    int missing_info;
    int lc;

} data_image;


char* convert_bin_to_base64(const char* binary);
char* truncate_string(const char* input, int length);
char* decimal_to_binary12(int decimal);

void free_symbol(symbol** symbol_t);
void free_data_image(data_image** data);
void free_data_image_array(data_image*** data_array, int size);

status concatenate_and_convert_to_base64(data_image* data);
status is_legal_addressing(command cmd, addressing_modes src, addressing_modes dest);
addressing_modes get_addressing_mode(const char *src);

#endif //ASSEMBLER_DATA_H
