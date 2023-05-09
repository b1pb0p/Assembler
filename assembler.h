#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

#define FILE_MODE_READ "r"
#define FILE_MODE_WRITE "w"
#define ASSEMBLY_EXT ".as"
#define AUTOMAKE_EXT  ".am"
#define OBJECT_EXT  ".ob"
#define ENTRY_EXT  ".ent"
#define EXTERNAL_EXT ".ext"


#define HANDLE_STATUS(file) if (code != NO_ERROR) { \
    handle_error(code, (file));                      \
    evaluate_and_proceed(code, (file)); \
}

void evaluate_and_proceed(int code, file_context* src);
void process_file(const char* file_name);
#endif
