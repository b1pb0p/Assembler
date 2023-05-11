/* utils.h
 * declaration for utilities functions, handling errors and managing input
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

#include "errors.h"

#define FILENAME_EXT_LEN 3 /* .as */
#define MAX_LINE_LENGTH 82 /* 80 + '\0' */
#define MAX_LABEL_LENGTH 32 /*  31 + '\0' */

typedef struct {
    FILE* file_ptr;
    char* file_name;
    int ic; /* Instructions counter */
    int dc; /* Data counter */
    int lc; /* Line counter */
}file_context;




file_context* create_file_context(const char* file_name, char* ext, char* mode, status * report);
size_t get_word(char **ptr);
status copy_string(char** target, const char* source);
status copy_n_string(char** target, const char* source, size_t count);
void free_file_context(file_context* context);
#endif
