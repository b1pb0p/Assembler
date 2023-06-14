/* errors.c
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/

#include <stdio.h>
#include <stdarg.h>
#include "errors.h"
#include "utils.h"

/* Status messages */
const char *msg[MSG_LEN] = {
        "Assembly completed without errors. Output file(s) generated.",
        "Assembly terminated with errors. No output file(s) generated.",
        "Internal Error - Invalid function call - %s.",
        "Assembler - Memory allocation error.",
        "Assembler - Unable to open file - %s",
        "Assembler - File opened successfully - %s.",
        "%s - Line cannot start with a number on line %d",
        "%s - Invalid opcode on line %d.",
        "%s - Invalid operand on line %d.",
        "%s - Missing operand on line %d.",
        "%s - Too many operands on line %d.",
        "%s - Illegal use of operand on line %d.",
        "%s - Invalid data call: Invalid value on line %d.",
        "%s - Duplicate label declaration on line %d.",
        "%s - Label defined at the beginning of the extern line is meaningless and will be ignored.",
        "%s - Invalid register used on line %d.",
        "%s - Extraneous text on line %d.",
        "%s - Missing '@' symbol on line %d.",
        "%s - Missing ':' symbol after label declaration on line %d.",
        "%s - Missing ',' symbol on line %d.",
        "%s - Line too long on line %d. Cannot exceed 80 characters.",
        "%s - Macro too long on line %d. Cannot exceed 31 characters.",
        "%s - Invalid macro name (%s) on line %d.",
        "%s - Label (%s) cannot start with a digit on line %d",
        "%s - Label (%s) does not exist on line %d.",
        "%s - Invalid label name (%s) on line %d.",
        "%s - Invalid Command or Directive after label declaration, (%s) on line %d.",
        "%s - Duplicate macro name on line %d.",
        "%s - Missing opening 'mcro' on line %d.",
        "%s - Missing closing 'endmcro' on line %d.",
        "Preprocessor (%d/%d) - No output file(s) generated - %s.as.",
        "Preprocessor (%d/%d) - Output file(s) successfully generated - %s.",
        "First Pass (%d/%d) - Output file(s) successfully generated - %s.as.",
        "First Pass (%d/%d) - No output file(s) generated - %s.as.",
        "Assembler process for - %s.as terminated with errors. No output file(s) generated.",
};

/**
 * Handles and reports errors during the assembly process.
 *
 * Handles different error codes and formats the error messages accordingly.
 * Additional arguments may be required for specific error messages.
 *
 * @param code      The error code indicating the type of error.
 * @param ...       Additional arguments depending on the error code.
 */
void handle_error(status code, ...) {
    va_list args;
    file_context* fc;
    int num, tot;
    char *fncall;

    va_start(args, code);

    if (code == NO_ERROR)
        printf("TERMINATED ->\t%s\n", msg[code]);
    else if (code == FAILURE)
        fprintf(stderr, "TERMINATED ->\t%s", msg[code]);
    else if (code == TERMINATE || code == ERR_FOUND_ASSEMBLER) {
        fncall =  va_arg(args, char *);
        fprintf(stderr, "TERMINATED ->\t");
        fprintf(stderr, msg[code], fncall);
    }
    else if (code == ERR_MEANINGLESS_LABEL) {
        fc = va_arg(args, file_context*);
        fprintf(stderr, "WARNING ->\t");
        fprintf(stderr, msg[code], fc->file_name);
    }
    else if (code == ERR_PRE || code == ERR_FIRST_PASS) {
        fprintf(stderr, "ERROR ->\t");
        num = va_arg(args, int);
        tot = va_arg(args, int);
        fncall = va_arg(args, char*);
        fprintf(stderr, msg[code], num, tot, fncall);
    }
    else {
        fprintf(stderr, "ERROR ->\t");
        fc = va_arg(args, file_context*);

        if (code < OPEN_FILE)
            fprintf(stderr, msg[code], fc->file_name);
        else if (code >= ERR_INVAL_MACRO_NAME && code <= ERR_INVAL_LABEL) {
            fncall = va_arg(args, char*);
            fprintf(stderr, msg[code], fc->file_name, fncall ,fc->lc);
        }
        else if (code <= ERR_MISSING_ENDMACRO)
            fprintf(stderr, msg[code], fc->file_name, fc->lc);
    }
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * Handles and reports progress messages during the assembly process.
 *
 * Handles different progress codes and formats the progress messages accordingly.
 * Additional arguments may be required for specific progress messages.
 *
 * @param code      The progress code indicating the type of progress.
 * @param ...       Additional arguments depending on the progress code.
 */
void handle_progress(status code, ...) {
    va_list args;
    file_context *fc;
    int num, tot;

    if (code == NO_ERROR) {
        printf("PROGRESS ->\t");
        printf("%s", msg[code]);
    }
    else {
        va_start(args, code);
        fc = va_arg(args, file_context*);
        if (!fc)
            printf("%s", msg[FAILURE]);
        else if (code <= OPEN_FILE)
            printf(msg[code], fc->file_name);
        else if (code == ERR_FIRST_PASS || code == FIRST_PASS_OK) {
            num = va_arg(args, int);
            tot = va_arg(args, int);
            fprintf(stderr, msg[code], num, tot, fc->file_name);
        }
        else {
            num = va_arg(args, int);
            tot = va_arg(args, int);
            printf(msg[code], num, tot, fc->file_name);
        }
        va_end(args);
    }
    printf("\n");
}
