/* Assembler Project in 20465 - System Programming Laboratory
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "utils.h"
#include "preprocessor.h"
#include "assembler.h"

#define HANDLE_STATUS(file) if (code != NO_ERROR) { \
    handle_error(code, (file));                      \
    evaluate_and_proceed(&code, (file)); \
}

/*
 *
status assembler_first_pass(file_context* file_c);
status assembler_second_pass(file_context* file_c);
FILE **assembler_generate_output();
*/

int main(int argc, char *argv[]) {
    int i;
    status report;

    if (argc == 1) {
        handle_error(FAILURE);
        exit(FAILURE);
    }

    for (i = 1; i < argc; i++)
      report = process_file(argv[i], i, argc - 1);
    if (report != NO_ERROR) handle_error(ERR_PRE_DONE);

    handle_error(report);
    atexit(free_macros);
    return 0;
}

status process_file(const char* file_name, int index, int max) {
    file_context *fc, *dest;
    status code = NO_ERROR;
    fc = create_file_context(file_name, ASSEMBLY_EXT, FILE_MODE_READ, &code);
    HANDLE_STATUS(fc)

    dest = create_file_context(file_name, PREPROCESSOR_EXT, FILE_MODE_WRITE, &code);
    HANDLE_STATUS(dest)

    code = assembler_preprocessor(fc, dest);
    if (code != NO_ERROR) {
        handle_error(ERR_PRE, dest, index, max);
        if (dest) {
            fclose(dest->file_ptr);
            remove(dest->file_name);
        }
        evaluate_and_proceed(&code, dest);
        return FAILURE;
    } else {
        handle_progress(PRE_FILE_OK, dest, index, max);
        return NO_ERROR;
    }
}

void evaluate_and_proceed(status* code, file_context* src) {
    if (*code == ERR_MEM_ALLOC || *code == TERMINATE) {
        /* Error Status that require to exit program */
        if (src) free_file_context(src);
        handle_error(FAILURE);
        exit(*code);
    } else
        *code = NO_ERROR;
}

