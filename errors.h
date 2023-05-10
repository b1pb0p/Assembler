/* errors.h
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/

#ifndef ASSEMBLER_ERRORS_H
#define ASSEMBLER_ERRORS_H

#define MSG_LEN 30
extern const char *MSG[MSG_LEN];

typedef enum {
    NO_ERROR,
    FAILURE,
    PRE_DONE,
    FIRST_PASS_DONE,
    SECOND_PASS_DONE,
    ERR_MEM_ALLOC,
    ERR_OPEN_FILE,
    OPEN_FILE_OK,
    PRE_FILE_OK,
    ERR_PRE_FILE,
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
    ERR_INVAL_MACRO_NAME,
    ERR_DUP_MACRO,
    ERR_MISSING_ENDMACRO
} status;

void handle_error(int code, ...);
void handle_progress(int code, ...);

#endif
