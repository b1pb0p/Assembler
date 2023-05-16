/* errors.c
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/
#include <stdio.h>
#include <stdarg.h>
#include "errors.h"
#include "utils.h"

/* Status messages */
const char *MSG[MSG_LEN] = {
        "Assembly completed without errors. Output files generated.",
        "Assembly terminated with errors. No output files generated.",
        "Assembler - Memory allocation error.",
        "First Pass - The first pass stage is finished without errors.",
        "Second Pass - The second pass stage is finished without errors.",
        "Preprocessor - The preprocessing stage is finished without errors.",
        "First Pass - The first pass stage is finished with errors.",
        "Second Pass - The second pass stage is finished with errors.",
        "Preprocessor - The preprocessing stage is finished with errors.",
        "Assembler - Unable to open file - %s",
        "Assembler - File opened successfully - %s.",
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
        "%s - Line too long on line %d. Mustn't exceed 80 characters.",
        "%s - Invalid macro name on line %d.",
        "%s - Duplicate macro name on line %d.",
        "%s - Missing 'endmcro' on line %d.",
        "Preprocessor (%d/%d) - No output file generated - %s.",
        "Preprocessor (%d/%d) - Output file successfully generated - %s.",
        "First Pass (%d/%d) - Output file successfully generated - %s.",
        "Second Pass (%d/%d) - Output file successfully generated - %s.",
        "First Pass (%d/%d) - No output file generated - %s.",
        "Second Pass (%d/%d) - No output file generated - %s."
};


void handle_error(status code, ...) {
    va_list args;
    file_context* fc;
    int num, tot;
    if (code == NO_ERROR) {
        printf("TERMINATED ->\t");
        printf("%s\n", MSG[code]);
    }
    else if (code == FAILURE) {
        fprintf(stderr, "TERMINATED ->\t");
        fprintf(stderr, "%s",MSG[code]);
    }
    else if (code <= ERR_PRE_DONE) {
        fprintf(stderr, "ERROR ->\t");
        fprintf(stderr, "%s",MSG[code]);
    }
    else {
        fprintf(stderr, "ERROR ->\t");
        /* Error messages that require additional arguments */
        va_start(args, code);
        fc = va_arg(args, file_context*);
        if (!fc)
            fprintf(stderr, "%s", MSG[FAILURE]);
        else if (code < OPEN_FILE)
            fprintf(stderr, MSG[code], fc->file_name);
        else if (code <= ERR_MISSING_ENDMACRO)
            fprintf(stderr, MSG[code],fc->file_name, fc->lc);
        else {
            num = va_arg(args, int);
            tot = va_arg(args, int);
            fprintf(stderr, MSG[code], num, tot, fc->file_name);
        }
        va_end(args);
    }
    fprintf(stderr, "\n");
}

void handle_progress(status code, ...) {
    va_list args;
    file_context *fc;
    int num, tot;

    if (code <= PRE_DONE) {
        printf("PROGRESS ->\t");
        printf("%s", MSG[code]);
    }
    else {
        /* Error messages that require additional arguments */
        va_start(args, code);
        fc = va_arg(args, file_context*);
        if (!fc)
            printf("%s", MSG[FAILURE]);
        else if (code <= OPEN_FILE)
            printf(MSG[code], fc->file_name);
        else {
            num = va_arg(args, int);
            tot = va_arg(args, int);
            printf( MSG[code], num, tot, fc->file_name);
        }
        va_end(args);
    }
    printf("\n");
}
