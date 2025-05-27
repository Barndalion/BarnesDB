#ifndef DB_OPS_H
#define DB_OPS_H

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

int extract_index(char *annotation);
char** get_index_data(int index, char *table_name);
Table* get_table(Database *db, char *tablename);
Column* get_column(Table *table, char *columnname);

#endif