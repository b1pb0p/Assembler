/* preprocessor.c
 * preprocessor functions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#ifndef ASSEMBLER_PREPROCESSOR_H
#define ASSEMBLER_PREPROCESSOR_H

#include <stdio.h>
#include "utils.h"
#include "errors.h"

#define MAX_MACRO_NAME_LENGTH 31 /* 30 + '\0' */
#define MACRO_START "mcro"
#define MACRO_END "endmcro"
#define SKIP_MCRO 4 /* mcro length */
#define SKIP_MCR0_END 7 /* endmcro length */
#define MAX_BUFFER_LENGTH 256

typedef struct macro_node{
    char* name;
    char* body;
    struct macro_node* next;
} macro_node;


status assembler_preprocessor(file_context *src, file_context *dest);

status handle_macro_start(file_context *src, char *line, int *found_macro, char **macro_name, char **macro_body);
status handle_macro_body(char *line, int found_macro, char **macro_body);
status handle_macro_end(char *line, int *found_macro, char **macro_name, char **macro_body);
status write_to_file(file_context *src, file_context *dest, char *line, int found_macro, int found_error);
status add_macro(char* name, char* body);

macro_node* is_macro_exists(char* name);

void free_macros();

#endif
