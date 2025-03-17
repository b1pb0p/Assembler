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
#define FILE_MODE_WRITE_PLUS "w+"
#define ASSEMBLY_EXT ".as"
#define PREPROCESSOR_EXT  ".am"
#define OBJECT_EXT  ".ob"
#define ENTRY_EXT  ".ent"
#define EXTERNAL_EXT ".ext"

extern const char *directives[DIRECTIVE_LEN];
extern const char *commands[COMMANDS_LEN];

typedef enum {
    ABSOLUTE,
    EXTERNAL,
    RELOCATABLE
} ARE;

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
    DEFAULT /* for .obj */
} Directive;

typedef enum {
    INVALID_MD = 0,
    IMMEDIATE = 1,
    DIRECT = 3,
    REGISTER = 5
} Adrs_mod;

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
    COLON,
    QUOTE
} Delimiter;

typedef struct {
    FILE* file_ptr;
    char* file_name;
    char* file_name_wout_ext;
    int lc; /* Line counter */
    int tc; /* total num of files counter */
    int fc; /* file counter (x out of tc) */
} file_context;


char *strdup(const char *s);
char* has_spaces_string(char **line, size_t *word_len, status *report);

int safe_atoi(const char *str);
int is_valid_register(file_context *src, const char* str, status *report);

void free_file_context(file_context** context);

size_t get_word_length(char **ptr);
size_t get_word(char **ptr, char *word, Delimiter delimiter);
size_t is_valid_string(char **line, char **word, status *report);

status is_valid_label(const char *label);
status copy_string(char** target, const char* source);
status copy_n_string(char** target, const char* source, size_t count);

Value validate_string(file_context *src, char **line ,char **p_word, size_t length, int *DC, status *report);
Value validate_data(file_context *src, char *word, size_t length, status *report);
Value concat_and_validate_string(file_context *src, char **line, char **word, size_t *length, int *DC, status *report);

Command is_command(const char* src);
Directive is_directive(const char* src);

file_context* create_file_context(const char* file_name, char* ext, size_t ext_len, char* mode, status *report);
#endif
