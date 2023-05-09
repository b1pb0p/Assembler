/* Assembler Project in 20465 - System Programming Laboratory
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "utils.h"
#include "preprocessor.h"
#include "assembler.h"

/*
 *
status assembler_first_pass(file_context* file_c);
status assembler_second_pass(file_context* file_c);
FILE **assembler_generate_output();
*/

int main(int argc, char *argv[]) {
    int i;

    if (argc == 1) {
        handle_error(FAILURE);
        exit(FAILURE);
    }

    for (i = 1; i < argc; i++) {
        process_file(argv[i]);
    }

    handle_error(NO_ERROR);
    atexit(free_macros);
    return 0;
}

void process_file(const char* file_name) {
    status code = NO_ERROR;
    file_context *fc = create_file_context(file_name, ASSEMBLY_EXT, FILE_MODE_READ, &code);
    HANDLE_STATUS(fc);

    file_context* dest = create_file_context(file_name, PREPROCESSOR_EXT, FILE_MODE_WRITE, &code);
    HANDLE_STATUS(dest);

    code = assembler_preprocessor(fc, dest);
    if (code != NO_ERROR) {
        handle_error(code, dest);
        handle_error(ERR_PRE, dest);
        if (dest) remove(dest->file_name);
        evaluate_and_proceed(code, dest);
    }

}

void evaluate_and_proceed(int code, file_context* src) {
    if (code <= ERR_PRE) {
        /* Error Status that require to exit program */
        if (src) free_file_context(src);
        exit(code);
    } else {
        code = NO_ERROR;
    }
}

