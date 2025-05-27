#include "parser.h"
#include "dbops.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int extract_index(char *data){
    char* index_start = strchr(data, '(');
    char* index_end = strchr(data, ')');

    int index_len = index_end - index_start - 1;
    if (index_start && index_end && index_len > 0) {
        char *index = malloc(index_len + 1);
        strncpy(index, index_start + 1, index_len);
        index[index_len] = '\0';

    return atoi(index);
    } else {
        fprintf(stderr, "Invalid index format in data: %s\n", data);
        return -1; \
    } 
}

Table* get_table(Database *db, char *tablename){
    for(int i = 0; i < db->table_count; i++){
        if(strcmp(db->tables[i].tablename, tablename) == 0){
            return &db->tables[i];
        }
    }
    return NULL;
}
Column* get_column(Table *table, char *columnname){
    for(int i = 0; i < table->column_count; i++){
        if(strcmp(table->columns[i].columnname, columnname) == 0){
            return &table->columns[i];
        }
    }
    return NULL;
}
char** get_index_data(int index, char *table_name){
    
    FILE *fp = fopen("MyDB.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }
    
    Database db = parse_file(fp);
    Table *t = get_table(&db, table_name);
    char **data = malloc((t->column_count +1) * sizeof(char*));
    for(int i = 0;i < t->column_count;i++){
        data[i] = NULL; // Initialize to NULL
        Column *c = &t->columns[i];
        for(int j = 0;j < c->data_count;j++){
            if(extract_index(c->data[j]) == index){
                data[i] = c->data[j];
            }
        } 
    }
    data[t->column_count] = NULL; 
    fclose(fp);
    return data;
}


retrieve_data RETRIEVE(char *filename,char *select_data, char *From_table, char *Field_name){
    retrieve_data result;

    FILE *pp = fopen(filename, "r");
    if (!pp) {
        perror("Failed to open file");
        result.type = RETURN_TYPE_TABLE; // Default to RETURN_TYPE_TABLE for error handling
        result.data.table = NULL; // Set to NULL to indicate failure
        return result; // Return an empty result on failure
    }
    Database db = parse_file(pp);
    char *endptr;
    if(From_table == NULL && Field_name == NULL){
        Table *t = get_table(&db, select_data);
        result.type = RETURN_TYPE_TABLE;
        result.data.table = t;
        return result;
    }
    if(From_table != NULL && Field_name == NULL){
        Table *t = get_table(&db, From_table);
        Column *c = get_column(t,select_data);
        result.type = RETURN_TYPE_COLUMN;
        result.data.column = c;
        return result;
    }
    if(From_table != NULL && Field_name != NULL){
        Table *t = get_table(&db, From_table);
        Column *c = get_column(t, Field_name);
        
        result.type = RETURN_TYPE_ARRAY;
        result.data.array = malloc(t->column_count * sizeof(char*));
    }
}

