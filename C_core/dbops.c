#include "parser.h"
#include "dbops.h"
#include "utils.h"
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

retrieve_data RETRIEVE(char *select_data, char *filename, char *From_table, char *Field_name){
    retrieve_data result;

    FILE *fp = fopen(filename,"r");
    if(!fp){
        perror("FILE NOT FOUND");
    }
   
    Database db = parse_file(fp);
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
        int index = get_index(select_data, t, c);
        char** temp_holder =  get_index_data(filename, index, t->tablename);

        
        result.type = RETURN_TYPE_ARRAY;
        result.data.array = malloc(t->column_count * sizeof(char*));
        for(int i = 0; i < t->column_count; i++){
            result.data.array[i] = temp_holder[i];
        }
        result.data.array[t->column_count] = NULL; // Null-terminate the array
        return result;
    }
}

/* insert function: it calculates the precise offset in the file using the index which is stored in
metadata file. it also updates the index and dynamically grows the textfile if space runs out */

void INSERT(char *Insert_Data, char *filename,char *From_table, char *Field_name){
    FILE *fp = fopen(filename, "r+");
    if(!fp) {
        perror("Failed to open file");
        return; 
    } // standard file safety
    printf("hi");

    char buffer[512];// holds 
    int inline_pos; // Position of the inline data in the line
    int in_table = 0; // Flag to check if we are in the correct table

    long in_file_pos;
    while(1){
        printf("hi while loop");
        in_file_pos = ftell(fp);
        if(!fgets(buffer,sizeof(buffer),fp)){
            break;
        }
        buffer[strcspn(buffer, "\n")] = 0;
        if(buffer[0] == '#'){
            if(strcmp(buffer + 1,From_table)==0){
                in_table = 1;
            }else{
                in_table = 0;
            }
            continue;
        }
        if(in_table && buffer[0]=='-'){
            printf("in first check");
            char *start = buffer + 1; 
            char *end = strchr(buffer, '{');
            char length = end - start;
            char column_name[65];
            strncpy(column_name, start, length); 
            column_name[length] = '\0';
            printf("\n%s, %s\n", column_name, Field_name);
            
            if(strcmp(column_name,Field_name)==0){
                printf("in second check");
                char *data_start = strchr(buffer,':');
                if(data_start != NULL){
                    inline_pos = (data_start - buffer) + 1;
                    int index = get_index_from_metadata(From_table, Field_name);
                    int old_capacity = get_capacity();
                    update_metadatafile_inplace(From_table,Field_name,(index + 1));
                    if(old_capacity <= index){
                        update_capacity(old_capacity * 2);
                        // TODO: ADD EXPONENTIAL GROWTH
                        fprintf(stderr, "Capacity exceeded. Cannot insert more data.\n");
                        break;
                    }
                    printf("in final if check");
                    // write_metadata_bin(filename);
                    long targetted_offset = in_file_pos + (long)inline_pos + (long)((index) * (SLOT_SIZE));
                    printf("%d\n", targetted_offset);

                    char slot_buffer[SLOT_SIZE];
                    memset(slot_buffer, '-', sizeof(slot_buffer));
                    char *modified_data = indexify(Insert_Data, index);
                    memcpy(slot_buffer, modified_data,strlen(modified_data));
                
                    fseek(fp, targetted_offset, SEEK_SET);
                    fwrite(slot_buffer, sizeof(char), SLOT_SIZE-1, fp);
                    fputc(',',fp);
                    break;
                }
            }
        }
    }
    fclose(fp);
} 

// Finish later

void DELETE(char* filename, char* data, char* table, char *column){
    // open and close for db
    FILE *fp = fopen(filename,"r+");
    Database db = parse_file(fp);
    fclose(fp);

    Table *t = get_table(&db,table);
    Column *c = get_column(t,column);

    int index = get_index(data,t,c);
    if(index == -1){
        fprintf(stderr, "NO ITEMS FOUND");
        return;
    }

    char buffer[512];// holds 
    int inline_pos; // Position of the inline data in the line
    int in_table = 0; // Flag to check if we are in the correct table
    FILE *fa = fopen(filename,"r+");
    if(!fa){
        perror("file not found");
        return;
    }
    long in_file_pos;
    printf("%d", index);
    while(1){
        printf("start of while loop \n");
        in_file_pos = ftell(fa);
        if(!fgets(buffer,sizeof(buffer),fa)){
            printf("ends here \n");
            break;
        }
        buffer[strcspn(buffer, "\n")] = 0;
        if(buffer[0] == '#'){
            if(strcmp(buffer + 1,table)==0){
                in_table = 1;
            }else{
                in_table = 0;
            }
            continue;
        }
        printf("\n below tabletest \n");
        if(in_table && buffer[0]=='-'){
            printf("entered in table block test \n");
            char *start = buffer + 1; 
            char *end = strchr(buffer, '{');
            char length = end - start;
            char column_name[65];
            strncpy(column_name, start, length); 
            column_name[length] = '\0';
            printf("comparing current column(%s) to %s", column_name,column);
            if(strcmp(column_name,column)==0){
                char *data_start = strchr(buffer,':');
                if(data_start != NULL){
                    if(index == get_index_from_metadata(table,column) ){
                        update_metadatafile_inplace(table,column,(index - 1));
                    }
                    inline_pos = (data_start - buffer) + 1;
                
                    long targetted_offset = in_file_pos + (long)inline_pos + (long)(index * (SLOT_SIZE));

                    char slot_buffer[SLOT_SIZE];
                    memset(slot_buffer, '-', sizeof(slot_buffer));
                
                    fseek(fa, targetted_offset, SEEK_SET);
                    fwrite(slot_buffer, sizeof(char), SLOT_SIZE-1, fa);
                    fputc(',',fa);

                    
                    break;
                }
            }
            else{
                continue;
            }
        }
    }
    fclose(fa);
}