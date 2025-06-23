#include "parser.h"
#include "dbops.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int return_index(char *data){
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
int get_index(char *data, Table *t, Column *c){
    if (t == NULL || c == NULL) {
        fprintf(stderr, "Table or column not found.\n");
        return -1; // Table or column not found
    }

    for(int i = 0; i < c->data_count; i++){
        char* comp_data = remove_index_tag_copy(c->data[i]);
        if (strcmp(comp_data, data) == 0) {
            return return_index(c->data[i]);
        }
    }

    fprintf(stderr, "Data not found in column.\n");
    return -1;   
}

char* remove_index_tag_copy(const char* data) {
    char* index_start = strchr(data, '(');
    if (!index_start) return strdup(data); // no tag found

    size_t prefix_len = index_start - data;
    char* index_end = strchr(index_start, ')');
    if (!index_end) return strdup(data); // unmatched '('

    index_end++; // move past ')'
    size_t suffix_len = strlen(index_end);

    // allocate new string: prefix + suffix + null terminator
    char* result = malloc(prefix_len + suffix_len + 1);
    if (!result) return NULL;

    strncpy(result, data, prefix_len);         
    strcpy(result + prefix_len, index_end);   

    return result;
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
        data[i] = NULL; 
        Column *c = &t->columns[i];
        for(int j = 0;j < c->data_count;j++){
            if(return_index(c->data[j]) == index){
                data[i] = remove_index_tag_copy(c->data[j]);
            }
        } 
    }
    data[t->column_count] = NULL; 
    fclose(fp);
    return data;
}

retrieve_data RETRIEVE(char *select_data, char *filename, char *From_table, char *Field_name){
    retrieve_data result;

    FILE *pp = fopen(filename, "r");
    if (!pp) {
        perror("Failed to open file");
        result.type = RETURN_TYPE_TABLE; // Default to RETURN_TYPE_TABLE for error handling
        result.data.table = NULL; // Set to NULL to indicate failure
        return result; // Return an empty result on failure
    }
    Database db = parse_file(pp);
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
        char** temp_holder =  get_index_data(index, t->tablename);

        
        result.type = RETURN_TYPE_ARRAY;
        result.data.array = malloc(t->column_count * sizeof(char*));
        for(int i = 0; i < t->column_count; i++){
            result.data.array[i] = temp_holder[i];
        }
        result.data.array[t->column_count] = NULL; // Null-terminate the array
        return result;
    }
}

void INSERT(char *Insert_Data, char *filename,char *From_table, char *Field_name){
    FILE *fp = fopen(filename, "r+");
    if(!fp) {
        perror("Failed to open file");
        return; 
    }

    char buffer[512];
    long in_file_pos;
    int inline_pos;
    int capacity = get_capacity("metadata.txt");

    while(fgets(buffer,sizeof(buffer),fp)){
        if(buffer[0] == '#'){
            printf("table was found\n");
            long raw_buffer_len = strlen(buffer);
            buffer[strcspn(buffer, "\n")] = 0; 
            if(strcmp(buffer + 1, From_table)==0){
                in_file_pos = ftell(fp);
                printf("From_table found at position: %ld\n raw_bufer:%d\n", ftell(fp), raw_buffer_len);
                printf("in_file_pos: %ld\n", in_file_pos);
            }
        }
        buffer[strcspn(buffer, "\n")] = 0;

        char *start = buffer + 1; 
        char *end = strchr(buffer, '{');
        char length = end - start;
        char column_name[256];

        strncpy(column_name, start, length); 
        column_name[length] = '\0';
        printf("column_name: %s\n", column_name);
        if(buffer[0] == '-' && strcmp(column_name,Field_name)==0){
            printf("column was found\n");
            char *data_start = strchr(buffer,':');
            if(data_start != NULL){
                inline_pos = (data_start - buffer) + 1;
                printf("inline_pos: %d\n", inline_pos);
                int index = read_index_from_metadata(From_table, Field_name);
                printf("index: %d\n", index);
                if(capacity <= index){
                    int old_capacity = get_capacity();
                    update_capacity(old_capacity * 2);
                    // TODO: ADD EXPONENTIAL GROWTH
                    fprintf(stderr, "Capacity exceeded. Cannot insert more data.\n");
                }
                long targetted_offset = in_file_pos + (long)inline_pos + (long)(index  * SLOT_SIZE);
                printf("targetted offset: %ld\n", targetted_offset);

                char slot_buffer[SLOT_SIZE];
                memset(slot_buffer, '-', sizeof(slot_buffer));
                memcpy(slot_buffer, Insert_Data,strlen(Insert_Data));
                printf("%s\n", slot_buffer);
               
                fseek(fp, targetted_offset, SEEK_SET);
                printf("writing to file at offset: %ld\n", ftell(fp));
                fwrite(slot_buffer, sizeof(char), SLOT_SIZE-1, fp);
                fputc(',',fp);
                
                update_metadatafile_inplace(From_table, Field_name, index + 1);
                return;
                
            }
        }

    }
    fclose(fp);
} 
void update_metadatafile_inplace(const char *tablename, const char *fieldname, int new_index){
    FILE *fp= fopen("metadata.txt", "r+");
    if(!fp) return;

    char line[256];
    long pos;

    while(fgets(line, sizeof(line), fp)) {
        char *token_tablename = strtok(line, " ");
        char *token_fieldname = strtok(NULL, " ");

        if(strcmp(token_fieldname, fieldname)==0 && strcmp(token_tablename, tablename)==0) {
            pos = ftell(fp);
            int index_pos = pos - 3;
            char new_index_str[3];
            sprintf(new_index_str, "%02d", new_index);
            fwrite(new_index_str, 1, 2, fp);
            break;
        }
    }
    fclose(fp);
}
int read_index_from_metadata(const char *tablename, const char *fieldname) {
    FILE *fp = fopen("metadata.txt", "r");
    if (!fp) return 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char *token_tablename = strtok(line, " ");
        char *token_fieldname = strtok(NULL, " ");
        char *token_index = strtok(NULL, " ");

        if (strcmp(token_tablename, tablename) == 0 && strcmp(token_fieldname, fieldname) == 0) {
            int index = atoi(token_index);
            fclose(fp);
            return index;
        }
    }
    fclose(fp);
}
int get_capacity(){
    FILE *fp = fopen("metadata.txt", "r+");
    if (!fp) return 0;

    char line[256];
    while(fgets(line,sizeof(line),fp)){
        char *word_token = strtok(line, " ");
        char *data_token = strtok(NULL, " ");

        int capacity = atoi(data_token);
        fclose(fp);
        return capacity;
    }
}

void update_capacity(int new_capacity) {
    FILE *fp = fopen("metadata.txt", "r+");
    if (!fp) return;

    char line[256];
    long pos;

    while(fgets(line, sizeof(line), fp)) {
        char *word_token = strtok(line, " ");
        if (strcmp(word_token, "capacity") == 0) {
            pos = ftell(fp);
            fseek(fp, pos - strlen(line) + 9, SEEK_SET); // Move to the start of the capacity value
            fprintf(fp, "%d", new_capacity);
            break;
        }
    }
    fclose(fp);
}
