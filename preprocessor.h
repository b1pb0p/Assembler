/* preprocessor.c
 * preprocessor functions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#ifndef ASSEMBLER_PREPROCESSOR_H
#define ASSEMBLER_PREPROCESSOR_H

#include <stdio.h>
#include "utils.h"
#include "errors.h"

#define MAX_LINE_LENGTH 82 /* 80 + '\0' */
#define MAX_MACRO_NAME_LENGTH 31 /* 30 + '\0' */
#define MAX_LABEL_LENGTH 32 /*  31 + '\0' */
#define DEFAULT_MACROS 2
#define MACRO_START "mcro"
#define MACRO_END "endmcro"
#define SKIP_MCRO 4 /* mcro length */
#define SKIP_MCR0_END 7 /* endmcro length */
#define MAX_BUFFER_LENGTH 256


typedef struct {
    char* name;
    char* body;
} macro_s;


status handle_macro_start(file_context * src, char *line, int *found_macro,char *macro_name, char *macro_body);
status handle_macro_body(file_context * src, char *line, char *macro_body);
status handle_macro_end(file_context * src, char *line, int *found_macro,char *macro_name, char *macro_body);
status write_to_file(file_context *dest, char *line, int found_macro, status report);
status add_macro(char* name, char* body);
void free_unused_macros();
void free_macros();

#endif
