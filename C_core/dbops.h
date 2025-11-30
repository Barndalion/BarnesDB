#ifndef DB_OPS_H
#define DB_OPS_H
#define SLOT_SIZE 65

#include "parser.h"
// dynamic typing system
typedef enum{
    RETURN_TYPE_TABLE,
    RETURN_TYPE_COLUMN,
    RETURN_TYPE_ARRAY,
}return_type;
// helper dynamic typing
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


Table* get_table(Database *db, char *tablename);
Column* get_column(Table *table, char *columnname);
/* NOTE: RETRIEVE may return pointers into dynamically-allocated data (e.g.
    Table*, Column* or char**). Depending on how parse_file and get_index_data
    allocate memory, the caller may be responsible for freeing those objects.
    Make sure to use a deallocation helper for any returned heap memory. */
retrieve_data RETRIEVE(char *filename,char *select_data, char *From_table, char *Field_name);
void INSERT(char *Insert_Data,const char *filename,char *From_table, char *Field_name);
void DELETE(const char* filename, char* data, char* table, char *column);

#endif