#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>

typedef struct{
    char *keyword;
    char **data;
    int data_count;
}QueryToken;

typedef struct{
    char *columnname;
    char **data;
    int data_count;
    char *datatype;
}Column;

typedef struct{
    char *tablename;
    Column *columns;
    int column_count;
}Table;

typedef struct{
   Table *tables;
   int table_count;
}Database;

typedef struct{
    QueryToken *tokens;
    int token_count;
}token_lib;

/* parse_query returns a pointer to a heap-allocated token_lib structure with
    heap-allocated inner strings (keyword, data entries). Callers are
    responsible for freeing all nested allocations. */
token_lib *parse_query(char *query_input);
/* parse_file returns a Database structure that contains heap-allocated
    tables, columns, and strings; callers must free the returned Database
    and its inner objects when done to avoid memory leaks. */
Database parse_file(FILE *fp);
#endif