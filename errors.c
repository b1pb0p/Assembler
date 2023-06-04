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
        "Assembly completed without errors. Output files generated.",
        "Assembly terminated with errors. No output files generated.",
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
        "%s - Label used before definition on line %d.",
        "%s - Missing label on line %d.",
        "%s - Duplicate label on line %d.",
        "%s - Invalid register used on line %d.",
        "%s - Extraneous text on line %d.",
        "%s - Missing '@' symbol on line %d.",
        "%s - Missing ';' symbol after label declaration on line %d.",
        "%s - Missing ',' symbol on line %d.",
        "%s - Line too long on line %d. Cannot exceed 80 characters.",
        "%s - Macro too long on line %d. Cannot exceed 31 characters.",
        "%s - Invalid macro name (%s) on line %d.",
        "%s - Label (%s) does not exist on line %d.",
        "%s - Duplicate macro name on line %d.",
        "%s - Missing opening 'mcro' on line %d.",
        "%s - Missing closing 'endmcro' on line %d.",
        "Preprocessor (%d/%d) - No output file generated - %s.",
        "Preprocessor (%d/%d) - Output file successfully generated - %s.",
        "First Pass (%d/%d) - Output file successfully generated - %s.",
        "Second Pass (%d/%d) - Output file successfully generated - %s.",
        "First Pass (%d/%d) - No output file generated - %s.",
        "Second Pass (%d/%d) - No output file generated - %s.",
        "Assembler process for - %s.as terminated with errors. No output files generated.",
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

    if (code == NO_ERROR) {
        printf("TERMINATED ->\t");
        printf("%s\n", msg[code]);
    }
    else if (code == FAILURE) {
        fprintf(stderr, "TERMINATED ->\t");
        fprintf(stderr, "%s", msg[code]);
    }
    else if (code == TERMINATE || code == ERR_FOUND_ASSEMBLER) {
        va_start(args, code);
        fncall =  va_arg(args, char *);
        fprintf(stderr, "TERMINATED ->\t");
        fprintf(stderr, msg[code], fncall);
        va_end(args);
    }
    else {
        fprintf(stderr, "ERROR ->\t");
        /* Error messages that require additional arguments */
        va_start(args, code);
        fc = va_arg(args, file_context*);
        if (!fc)
            fprintf(stderr, "%s", msg[FAILURE]);
        else if (code < OPEN_FILE)
            fprintf(stderr, msg[code], fc->file_name);
        else if (code == ERR_INVAL_MACRO_NAME) {
            fncall = va_arg(args, char*);
            num = va_arg(args, int);
            fprintf(stderr, msg[code], fc->file_name, fncall ,num);
        }
        else if (code == ERR_LABEL_DOES_NOT_EXIST) {
            fncall = va_arg(args, char*);
            fprintf(stderr, msg[code], fc->file_name, fncall ,fc->lc);
        }
        else if (code <= ERR_MISSING_ENDMACRO)
            fprintf(stderr, msg[code], fc->file_name, fc->lc);
        else {
            num = va_arg(args, int);
            tot = va_arg(args, int);
            fprintf(stderr, msg[code], num, tot, fc->file_name);
        }
        va_end(args);
    }
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
        else {
            num = va_arg(args, int);
            tot = va_arg(args, int);
            printf(msg[code], num, tot, fc->file_name);
        }
        va_end(args);
    }
    printf("\n");
}
