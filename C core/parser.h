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
    union{
        char **data;
        double *int_data;
    }data;
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

QueryToken *tokens;
extern int token_count;

QueryToken *parse_query(char *query_input);
Database parse_file(FILE *fp);
void print_tokens(QueryToken *tokens, int token_count);
void print_database(Database db);

#endif