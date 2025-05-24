#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

QueryToken *parse_query(char *query_input){

    int token_count = 0;
    free(tokens);
    tokens = NULL;

    if (query_input == NULL) {
    fprintf(stderr, "Query is NULL!\n");
    return NULL;
    }
    printf("Parsing query: %s\n", query_input);

    char *query = strdup(query_input);
    if (!query) {
        perror("strdup");
        return NULL;
    }

    printf("Query after strdup: %s\n", query);
    char *saveptr2;
    char *token = strtok_r(query, " ", &saveptr2);
    while(token != NULL){
        char *startbracket = strchr(token,'(');
        char *endbracket = strchr(token,')');

        if (!startbracket || !endbracket || startbracket >= endbracket) {
            token = strtok_r(NULL, " ", &saveptr2);
            continue;
        }

        int keyword_len = startbracket - token;
        char *keyword = malloc(keyword_len + 1);
        strncpy(keyword,token,keyword_len);
        keyword[keyword_len] = '\0';

        int data_len = endbracket - startbracket ;
        char *data = malloc(data_len + 1);
        strncpy(data,startbracket + 1, data_len - 1);
        data[data_len - 1] = '\0';

        int data_count = 0;

        QueryToken qt;
        qt.keyword = keyword;
        qt.data = NULL;
        char *saveptr;
        char *data_token = strtok_r(data, ",", &saveptr);
        while(data_token != NULL){
            qt.data = realloc(qt.data, sizeof(char*) * (data_count + 1));
            qt.data[data_count++] = strdup(data_token);
            data_token = strtok_r(NULL, ",", &saveptr);
        }
        qt.data_count = data_count;

        tokens = realloc(tokens, sizeof(QueryToken) * (token_count + 1));
        tokens[token_count++] = qt;

        token = strtok_r(NULL, " ", &saveptr2);
    }
    
    return tokens;
}

Database parse_file(FILE *fp){
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

                data_array[data_count++] = strdup(token);
                token = strtok(NULL, ",");
            }
            Column col;
            col.columnname = column_name;
            col.data_count = data_count;
            col.datatype = datatype;

            if(strcmp(datatype,"int") == 0 || strcmp(datatype, "float") == 0){
                col.data.int_data = malloc(sizeof(double) * data_count);
                for(int i = 0;i < data_count;i++){
                    col.data.int_data[i] = strtod(data_array[i],NULL);
                }
            }else{
                col.data.data = data_array;
            }

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

// void RETRIEVE(char *table_name, char *column_name, char *select_data){
//     if(table_name != NULL){

//     }
// }

void print_database(Database db) {
    for (int i = 0; i < db.table_count; i++) {
        Table t = db.tables[i];
        printf("Table: %s\n", t.tablename);
        for (int j = 0; j < t.column_count; j++) {
            Column c = t.columns[j];
            printf("  Column: %s | Datatype: %s => ", c.columnname, c.datatype);
            if (strcmp(c.datatype, "int") == 0 || strcmp(c.datatype, "float") == 0) {
                for (int k = 0; k < c.data_count; k++) {
                    printf("%g", c.data.int_data[k]);
                    if (k < c.data_count - 1) printf(", ");
                }
            } else {
                for (int k = 0; k < c.data_count; k++) {
                    printf("%s", c.data.data[k]);
                    if (k < c.data_count - 1) printf(", ");
                }
            }
            printf("\n");
        }
        printf("\n");
    }
}

void print_tokens(QueryToken *tokens, int token_count) {
    for (int i = 0; i < token_count; i++) {
        printf("Keyword: %s\n", tokens[i].keyword);
        printf("Data: ");
        for (int j = 0; j < tokens[i].data_count; j++) {
            printf("%s, ", tokens[i].data[j]);
        }
        printf("\n");
    }
}
