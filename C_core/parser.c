#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "dbops.h"
#include "utils.h"

token_lib *parse_query(char *query){

    token_lib *token_arr = malloc(sizeof(token_lib));

    token_arr->tokens = malloc(sizeof(QueryToken));
    token_arr->token_count = 0;

    printf("Query after strdup: %s\n", query);
    char *saveptr2;
    char *token = strtok_r(query, " ", &saveptr2);
    while(token != NULL){
        char *startbracket = strchr(token,'(');
        char *endbracket = strchr(token,')');

        int keyword_len = startbracket - token;
        char *keyword = malloc(keyword_len + 1);
        strncpy(keyword,token,keyword_len);
        keyword[keyword_len] = '\0';

        if (!startbracket || !endbracket || startbracket >= endbracket) {
            token = strtok_r(NULL, " ", &saveptr2);
            continue;
        }

        int data_len = endbracket - startbracket ;
        char *data = malloc(data_len + 1);
        strncpy(data,startbracket + 1, data_len - 1);
        data[data_len - 1] = '\0';

        int data_count = 0;

        QueryToken qt;
        int capacity = 2;
        qt.keyword = keyword;
        qt.data = malloc(sizeof(char*));
        char *saveptr;
        char *data_token = strtok_r(data, " ", &saveptr);
        while(data_token != NULL){
            if (data_count + 1 > capacity) {;
                    qt.data = realloc(qt.data, sizeof(char*) * capacity);
                    capacity *= 2;
                }
            qt.data[data_count++] = strdup(data_token);
            data_token = strtok_r(NULL, ",", &saveptr);
        }
        qt.data_count = data_count;

        token_arr->tokens = realloc(token_arr->tokens, sizeof(QueryToken) * (token_arr->token_count + 1));
        token_arr->tokens[token_arr->token_count++] = qt;

        token = strtok_r(NULL, " ", &saveptr2);
    }
    
    return token_arr;
}

Database parse_file(char *filename){
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    Database db = {NULL,0};
    Table current_table = {0};
    char line[1024];

    while(fgets(line, sizeof(line),fp)){
        line[strcspn(line, "\n")] = 0;
        if(line[0] == '#'){
            if (current_table.tablename != NULL) {
                        db.tables = realloc(db.tables, sizeof(Table) * (db.table_count + 1));
                        db.tables[db.table_count++] = current_table;
                        current_table = (Table){0};  
                    }
            current_table.tablename = strdup(line + 1);
                }

        else if(line[0]=='-'){

            char *colon = strchr(line, ':');
            if(!colon){
                continue;
            }

            char *datatypestart = strchr(line,'{');
            char *datatypeend = strchr(line,'}');

            if(!datatypestart || !datatypeend){
                continue;
            }

            int datatype_len = datatypeend - datatypestart;
            char *datatype = malloc(datatype_len + 1);
            strncpy(datatype,datatypestart+1, datatype_len-1);
            datatype[datatype_len-1] = '\0';

            int name_len = datatypestart - (line+1);
            char *column_name = malloc(name_len + 1);
            strncpy(column_name,line + 1, name_len);
            column_name[name_len] = '\0';

            char *data = colon +1;
            int data_capacity = 4;
            char **data_array = malloc(data_capacity * sizeof(char*));
            int data_count = 0;

            char *token = strtok(data,",");

            while(token != NULL){
                if(data_count>=data_capacity){
                    data_capacity *= 2;
                    data_array = realloc(data_array, sizeof(char*)*data_capacity);
                }
                char* cleaned_token = trim(token);
                if(!cleaned_token || strlen(cleaned_token) == 0){
                    free(cleaned_token);
                    token = strtok(NULL, ",");
                    continue;
                }
                data_array[data_count++] = strdup(cleaned_token);
                token = strtok(NULL, ",");
            }
            Column col;
           
            col.columnname = column_name;
            
            col.datatype = datatype;
            col.data = malloc(sizeof(char*) * data_count);

            for (int i = 0; i < data_count;i++){
                col.data[i] = strdup(data_array[i]);
            }
            col.data_count = data_count;

            current_table.columns = realloc(current_table.columns,sizeof(Column) * (current_table.column_count + 1));
            current_table.columns[current_table.column_count++] = col;
        }
        
    }
     if (current_table.tablename != NULL) {
        db.tables = realloc(db.tables, sizeof(Table) * (db.table_count + 1));
        db.tables[db.table_count++] = current_table;
    }
    return db;
}
