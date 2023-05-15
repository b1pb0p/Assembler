#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

#define FILE_MODE_READ "r"
#define FILE_MODE_WRITE "w"
#define ASSEMBLY_EXT ".as"
#define PREPROCESSOR_EXT  ".am"
#define OBJECT_EXT  ".ob"
#define ENTRY_EXT  ".ent"
#define EXTERNAL_EXT ".ext"

status process_file(const char* file_name, int index, int max);
void evaluate_and_proceed(status* code, file_context* src);
#endif
