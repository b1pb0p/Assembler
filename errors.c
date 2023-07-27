/* errors.c
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "errors.h"
#include "utils.h"

/* Status messages */
const char *msg[MSG_LEN] = {
        "Assembly process for %s.as completed without errors. Output file(s) have been generated.",
        "Assembly terminated with errors. No output file(s) have been generated.",
        "Assembler - Memory allocation error.",
        "Invalid function call - %s.",
        "Assembler - Unable to open file - %s",
        "Assembler - File opened successfully - %s.",
        "Assembler process for %s.as terminated with errors. No output file(s) have been generated.",
        "%s - Line cannot start with a digit on line %d",
        "%s - Excessive comma on line %d.",
        "%s - Duplicate label declaration on line %d.",
        "%s - Invalid operand(s) on line %d.",
        "%s - Missing operand(s) on line %d.",
        "%s - Too many operands on line %d.",
        "%s - Invalid register used on line %d.",
        "%s - Extraneous text on line %d.",
        "%s - Missing '\"' symbol on line %d.",
        "%s - Missing '@' symbol before a register on line %d.",
        "%s - Missing ':' symbol after label declaration on line %d.",
        "%s - Missing ',' symbol on line %d.",
        "%s - Missing '.' symbol before a directive on line %d.",
        "%s - Line length exceeds the maximum limit on line %d. Maximum length is 80 characters.",
        "%s - Macro length exceeds the maximum limit on line %d. Maximum length is 31 characters.",
        "%s - Operand length exceeds the maximum limit on line %d. Maximum length is 31 characters.",
        "%s - Duplicate macro name on line %d.",
        "%s - Missing opening 'mcro' on line %d.",
        "%s - Missing closing 'endmcro' on line %d.",
        "%s - Invalid opcode (%s) on line %d.",
        "%s - %s action has an empty body on line %d",
        "%s - Unused extern label (%s) on line %d.",
        "%s - Invalid macro name (%s) on line %d.",
        "%s - Label (%s) cannot start with a digit on line %d",
        "%s - Label (%s) cannot act as both entry and extern on line %d",
        "%s - Label (%s) is being declared/used in a forbidden context on line %d",
        "%s - Invalid label name (%s) on line %d.",
        "%s - Label (%s) does not exist on line %d.",
        "%s - Meaningless label (%s) at the beginning of a %s line. It will be ignored. on line %d.",
        "%s - Invalid %s call (%s) on line %d.",
        "%s - Invalid %s (%s) contains illegal characters on line %d",
        "%s - Duplicate %s declaration (%s) on line %d.",
        "%s - Invalid Command or Directive after %s, (%s) on line %d.",
        "Preprocessor (%d/%d) - No output file(s) have been generated - %s.as.",
        "Preprocessor (%d/%d) - Output file(s) have been successfully generated - %s.",
        "First Pass (%d/%d) - Output file(s) have been successfully generated - %s.as.",
        "First Pass (%d/%d) - No output file(s) have been generated - %s.as."
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
    file_context *fc = NULL;
    int num, tot;
    Directive dir;
    char *fncall, *fncall_par;

    va_start(args, code);

    if (code == FAILURE || code == ERR_MEM_ALLOC)
        fprintf(stderr, code == ERR_MEM_ALLOC ? "ERROR ->\t%s" : "TERMINATED ->\t%s", msg[code]);
    else if (code == TERMINATE || code == ERR_FOUND_ASSEMBLER) {
        fncall =  va_arg(args, char *);
        fprintf(stderr, code == TERMINATE ? "INTERNAL ERROR ->\t" : "TERMINATED ->\t");
        fprintf(stderr, msg[code], fncall);
    }
    else if (code >= ERR_OPEN_FILE && code <= ERR_MISSING_ENDMACRO) {
        fprintf(stderr, "ERROR ->\t");
        fc = va_arg(args, file_context*);
        fprintf(stderr, msg[code], fc->file_name, fc->lc);
    }
    else if (code == ERR_INVALID_ACTION || code == ERR_ILLEGAL_CHARS || code == ERR_INVALID_SYNTAX) {
        fprintf(stderr, "ERROR ->\t");
        fc = va_arg(args, file_context*);
        fncall_par = va_arg(args, char*);
        fncall = va_arg(args, char*);
        fprintf(stderr, msg[code], fc->file_name, tolower(*fncall_par) == 'l' ? "label declaration"
        : *fncall_par == 'd' ? "data assigment" : "string assigment", fncall, fc->lc);
    }
    else if (code == WARN_EMPTY_DIR) {
        fc = va_arg(args, file_context*);
        dir = va_arg(args, Directive);
        fprintf(stderr, "WARNING ->\t");
        fprintf(stderr, msg[code], fc->file_name, dir == ENTRY ? "Entry" : dir == EXTERN ? "Extern"
             : dir == DATA ? "Data" : "String"   , fc->lc);
    }
    else if (code == WARN_UNUSED_EXT) {
        fc = va_arg(args, file_context*);
        fncall = va_arg(args, char *);
        num = va_arg(args, int);
        fprintf(stderr, "WARNING ->\t");
        fprintf(stderr, msg[code], fc->file_name, fncall, num);
    }
    else if (code == WARN_MEANINGLESS_LABEL) {
        fc = va_arg(args, file_context*);
        fncall =  va_arg(args, char *);
        dir = va_arg(args, Directive);
        fprintf(stderr, "WARNING ->\t");
        fprintf(stderr, msg[code], fc->file_name, fncall, dir == ENTRY ? "entry" : "extern", fc->lc);
    }
    else if (code ==  ERR_DUPLICATE_DIR) {
        fc = va_arg(args, file_context*);
        fncall =  va_arg(args, char *);
        dir = va_arg(args, Directive);
        fprintf(stderr, "ERROR ->\t");
        fprintf(stderr, msg[code], fc->file_name, dir == ENTRY ? "entry" : "extern", fncall, fc->lc);
    }
    else if (code >= ERR_INVALID_OPCODE && code < ERR_LABEL_DOES_NOT_EXIST) {
        fprintf(stderr, "ERROR ->\t");
        fc = va_arg(args, file_context*);
        fncall =  va_arg(args, char *);
        fprintf(stderr, msg[code], fc->file_name, fncall, fc->lc);
    }
    else if (code == ERR_LABEL_DOES_NOT_EXIST) {
        fprintf(stderr, "ERROR ->\t");
        fc = va_arg(args, file_context*);
        fncall = va_arg(args, char*);
        num = va_arg(args, int);
        fprintf(stderr, msg[code], fc->file_name, fncall, num);
    }
    else if (code == ERR_PRE || code == ERR_FIRST_PASS) {
        fprintf(stderr, "ERROR ->\t");
        num = va_arg(args, int);
        tot = va_arg(args, int);
        fncall = va_arg(args, char*);
        fprintf(stderr, msg[code], num, tot, fncall);
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
    char *fncall = NULL;

    va_start(args, code);
    if (code == NO_ERROR)
        printf(msg[code], va_arg(args, char*));
    else {
        if (code <= OPEN_FILE) {
            fc = va_arg(args, file_context*);
            printf(msg[code], fc->file_name);
        }
        else if (code == FIRST_PASS_OK) {
            num = va_arg(args, int);
            tot = va_arg(args, int);
            fncall = va_arg(args, char*);
            printf(msg[code], num, tot, fncall);
        }
        else if (code == PRE_FILE_OK) {
            fc = va_arg(args, file_context*);
            num = va_arg(args, int);
            tot = va_arg(args, int);
            printf(msg[code], num, tot, fc->file_name);
        }
        else
        fprintf(stderr, "INTERNAL ERROR ->\tInvalid function call - handle_progress()");
        va_end(args);
    }
    printf("\n");
}
