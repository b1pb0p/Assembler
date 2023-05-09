/* errors.h
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/

#ifndef ASSEMBLER_ERRORS_H
#define ASSEMBLER_ERRORS_H

#define ERR_MSG_LEN 20
extern const char *ERR_MSG[ERR_MSG_LEN];

typedef enum {
    NO_ERROR,
    FAILURE,
    ERR_MEM_ALLOC,
    ERR_OPEN_FILE,
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
    ERR_INVAl_MACRO_NAME,
}status;

#endif
