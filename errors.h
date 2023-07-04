/* errors.h
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/

#ifndef ASSEMBLER_ERRORS_H
#define ASSEMBLER_ERRORS_H

#define MSG_LEN 42
extern const char *msg[MSG_LEN];

typedef enum {
    NO_ERROR,
    FAILURE,
    TERMINATE,
    ERR_MEM_ALLOC,
    ERR_OPEN_FILE,
    OPEN_FILE,
    ERR_LINE_START_DIGIT,
    ERR_INVALID_OPCODE,
    ERR_INVALID_OPERAND,
    ERR_MISS_OPERAND,
    ERR_TOO_MANY_OPERANDS,
    ERR_INVALID_SYNTAX,
    ERR_EXTRA_COMMA,
    ERR_DUP_LABEL,
    WARN_MEANINGLESS_LABEL,
    WARN_UNUSED_EXT,
    ERR_INVALID_REGISTER,
    ERR_EXTRA_TEXT,
    ERR_MISSING_QMARK,
    ERR_MISS_ADDRESS_MARK,
    ERR_MISSING_COLON,
    ERR_MISSING_COMMA,
    ERR_LINE_TOO_LONG,
    ERR_OPERAND_TOO_LONG,
    ERR_MACRO_TOO_LONG,
    ERR_INVAL_MACRO_NAME,
    ERR_LABEL_START_DIGIT,
    ERR_ILLEGAL_CHARS,
    ERR_DUPLICATE_DIR,
    ERR_LABEL_DOES_NOT_EXIST,
    ERR_INVALID_ACTION,
    ERR_BOTH_DIR,
    ERR_FORBIDDEN_LABEL_DECLARE,
    ERR_INVALID_LABEL,
    ERR_DUP_MACRO,
    ERR_MISSING_MACRO,
    ERR_MISSING_ENDMACRO,
    ERR_PRE,
    PRE_FILE_OK,
    FIRST_PASS_OK,
    ERR_FIRST_PASS,
    ERR_FOUND_ASSEMBLER
} status;

void handle_error(status code, ...);
void handle_progress(status code, ...);

#endif
