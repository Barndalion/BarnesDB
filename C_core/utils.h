#ifndef UTILS_H
#define UTILS_H

#include "dbops.h"
#include "parser.h"



void print_tokens(QueryToken *tokens, int token_count);
void print_database(Database db);
char *trim(char *str);
char *indexify(char *data, int index);
int return_index(char *data);
int get_index(char *data, Table *t, Column *c);
char* remove_index_tag_copy(const char* data);
char** get_index_data(char* filename, int index, char *table_name);
int get_capacity();
void update_capacity(int new_capacity);
void write_metadata_bin(char* filename);
void insert_metadata_record(char *tablename, char *fieldname, int index_count);
int get_index_from_metadata(char *tablename, char* fieldname);
void update_metadatafile_inplace(const char *tablename, const char *fieldname, int new_index);
void print_metadata_bin();
void initialize_runtime_database(const char* database_name);
const char* active_database(void);
const char* metadata_file(void);
int file_exists_fopen(const char *filename);

#endif