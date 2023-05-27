/* errors.h
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/

#ifndef ASSEMBLER_ERRORS_H
#define ASSEMBLER_ERRORS_H

#define MSG_LEN 37
extern const char *msg[MSG_LEN];

typedef enum {
    NO_ERROR,
    FAILURE,
    TERMINATE,
    ERR_MEM_ALLOC,
    ERR_OPEN_FILE,
    OPEN_FILE,
    ERR_INVAL_OPCODE,
    ERR_INVAL_OPERAND,
    ERR_MISS_OPERAND,
    ERR_TOO_MANY_OPERANDS,
    ERR_ILLEGAL_OPERAND,
    ERR_UNDEF_LABEL,
    ERR_MISS_LABEL,
    ERR_DUP_LABEL,
    ERR_INVAL_REGISTER,
    ERR_EXTRA_TEXT,
    ERR_MISS_ADDRESS_MARK,
    ERR_MISS_SEMICOLON,
    ERR_MISS_COMMA,
    ERR_LINE_TOO_LONG,
    ERR_MACRO_TOO_LONG,
    ERR_INVAL_MACRO_NAME,
    ERR_DUP_MACRO,
    ERR_MISSING_MACRO,
    ERR_MISSING_ENDMACRO,
    ERR_PRE,
    PRE_FILE_OK,
    FIRST_PASS_OK,
    SECOND_PASS_OK,
    ERR_FIRST_PASS,
    ERR_SECOND_PASS,
    ERR_FOUND_ASSEMBLER
} status;

void handle_error(status code, ...);
void handle_progress(status code, ...);

#endif
