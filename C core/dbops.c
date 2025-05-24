#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
void get_index_data(int index, char *table_name){
    
    FILE *fp = fopen("MyDB.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }
    Database db = parse_file(fp);
    Table *t = get_table(&db, table_name);
    for(int i = 0;i < t->column_count;i++){
        Column *c = &t[i];
        for(int j = 0;j < c->data_count;j++){
            if(extract_index(c->data.data[j]) == index){
              
            }
        } 
    }
}
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

void RETRIEVE(char *fp,char *select_data, char *From_table, char *Field_name){
    FILE *fp = fopen("MyDB.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }
    Database db = parse_file(fp);
    char *endptr;
    if(From_table == NULL && Field_name == NULL){
        Table *t = get_table(&db, select_data);
        return t;
    }
    if(From_table != NULL && Field_name == NULL){
        Table *t = get_table(&db, From_table);
        Column *c = get_column(t,select_data);
        return c;
    }
    if(From_table != NULL && Field_name != NULL){
        Table *t = get_table(&db, From_table);
        Column *c = get_column(t, Field_name);
        
        for(int i = 0; i < c->data_count;i++){
            if(strcmp(c->datatype,"int")==0 || strcmp(c->datatype,"float")==0){
                int val = strtod(select_data, &endptr);
                if(*endptr == '\0'){ // Check if conversion was successful
                    if(c->data.int_data[i] == val){
                        
                        printf("Match found: %g\n", c->data.int_data[i]);
                    }
                }
                if(c->data.int_data[i] == )
            }
        }
    }
 
}

