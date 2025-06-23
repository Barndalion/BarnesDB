#ifndef DB_OPS_H
#define DB_OPS_H
#define SLOT_SIZE 65

#include "parser.h"
typedef enum{
    RETURN_TYPE_TABLE,
    RETURN_TYPE_COLUMN,
    RETURN_TYPE_ARRAY,
}return_type;

typedef struct{
    return_type type;
    union {
        Table *table;
        Column *column;
        char **array;
    }data;
}retrieve_data;

int return_index(char *annotation);
int get_index(char *data, Table *t, Column *c);
char* remove_index_tag_copy(const char* data);
char** get_index_data(int index, char *table_name);
Table* get_table(Database *db, char *tablename);
Column* get_column(Table *table, char *columnname);
retrieve_data RETRIEVE(char *filename,char *select_data, char *From_table, char *Field_name);
void INSERT(char *Insert_Data, char *filename,char *From_table, char *Field_name);
void update_metadatafile_inplace(const char *tablename, const char *fieldname, int new_index);
int read_index_from_metadata(const char *tablename, const char *fieldname);
int get_capacity();
void update_capacity(int new_capacity);

#endif