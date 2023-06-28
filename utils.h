/* utils.h
 * declaration for utilities functions, managing input and globals.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

#include "errors.h"

#define FILE_EXT_LEN 3 /* .as */
#define FILE_EXT_LEN_OUT 4 /* .obj */
#define MAX_LINE_LENGTH 80 /* 80 - Using strlen */
#define MAX_LABEL_LENGTH 32 /*  31 + '\0' */
#define DIRECTIVE_LEN 4
#define COMMANDS_LEN 16
#define MAX_BUFFER_LENGTH 256

#define FILE_MODE_READ "r"
#define FILE_MODE_WRITE "w"
#define ASSEMBLY_EXT ".as"
#define PREPROCESSOR_EXT  ".am"
#define OBJECT_EXT  ".ob"
#define ENTRY_EXT  ".ent"
#define EXTERNAL_EXT ".ext"

extern const char *directives[DIRECTIVE_LEN];
extern const char *commands[COMMANDS_LEN];

typedef enum {
    LBL,
    NUM,
    STR,
    INV
} Value;

typedef enum {
    DATA = 1,
    STRING,
    ENTRY,
    EXTERN,
    DEFAULT /* Added field to indicate it belongs to the current file */
} Directive;

typedef enum {
    DIRECT = 1,
    INDIRECT = 3,
    REGISTER = 5,
    INVALID = -1
} Addressing_modes;

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
    STOP,
    INV_CMD
} Command;

typedef enum {
    SPACE,
    COMMA,
    COLON
} Delimiter;

typedef struct {
    FILE* file_ptr;
    char* file_name;
    char* file_name_wout_ext;
    int lc; /* Line counter */
} file_context;



char *strdup(const char *s);

size_t get_word_length(char **ptr);
size_t get_word(char **ptr, char *word, Delimiter delimiter);

status skip_white_spaces(char *line);
status copy_string(char** target, const char* source);
status copy_n_string(char** target, const char* source, size_t count);

Command is_command(const char* src);
Directive is_directive(const char* src);

void free_file_context(file_context** context);
void unget_word(char **ptr, size_t word_length, char *line);
void free_file_context_array(file_context** contexts, int size);

file_context* create_file_context(const char* file_name, char* ext, size_t ext_len, char* mode, status *report);
#endif
