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

token_lib *parse_query(char *query_input);
Database parse_file(char* filename);
#endif