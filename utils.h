/* utils.h
 * declaration for utilities functions, managing input and globals.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

#include "errors.h"

#define FILENAME_EXT_LEN 3 /* .as */
#define MAX_LINE_LENGTH 82 /* 80 + '\0' */
#define MAX_LABEL_LENGTH 32 /*  31 + '\0' */
#define DIRECTIVE_LEN 4
#define COMMANDS_LEN 16

extern const char *directives[DIRECTIVE_LEN];
extern const char *commands[COMMANDS_LEN];

typedef enum {
    A,
    R,
    E
} ARE;

typedef enum {
   DATA,
   STRING,
   ENTRY,
   EXTERN
} directive;

typedef enum {
    MOV,
    CMP,
    ADD,
    SUB,
    NOT,
    CLR,
    LEA,
    INC,
    DEC,
    JMP,
    BNE,
    RED,
    PRN,
    JSR,
    RTS,
    STOP
} command;


typedef struct {
    FILE* file_ptr;
    char* file_name;
    int ic; /* Instructions counter */
    int dc; /* Data counter */
    int lc; /* Line counter */
} file_context;




file_context* create_file_context(const char* file_name, char* ext, char* mode, status * report);
size_t get_word(char **ptr);
status copy_string(char** target, const char* source);
status copy_n_string(char** target, const char* source, size_t count);
directive is_directive(const char* src);
command is_command(const char* src);
void free_file_context(file_context** context);
#endif
