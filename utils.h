/* utils.h
 * declaration for utilities functions, handling errors and managing input
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

#include "errors.h"

#define FILENAME_EXT_LEN 3 /* .as */

typedef struct {
    FILE* file_ptr;
    char* file_name;
    int ic; /* Instructions counter */
    int dc; /* Data counter */
    int lc; /* Line counter */
}file_context;




file_context* create_file_context(char* file_name, char* ext, char* mode, int* report);
size_t get_word(char **ptr);
status copy_string(char** target, const char* source);
status copy_n_string(char** target, const char* source, size_t count);
#endif
