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

typedef struct{
    char tablename[64];
    char fieldname[64];
    int index_count;
}Metadata_records;

typedef struct{
    Metadata_records *records;
    int record_count;
}Metadata;

Table* get_table(Database *db, char *tablename);
Column* get_column(Table *table, char *columnname);
retrieve_data RETRIEVE(char *filename,char *select_data, char *From_table, char *Field_name);
void INSERT(char *Insert_Data, char *filename,char *From_table, char *Field_name);
void DELETE(char* filename, char* data, char* table, char *column);

#endif