#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbops.h"
#include "parser.h"
#include "utils.h"

//TO MUCH RELIANCE ON AI LOOK OVER
char *trim(char *str){
    char *buffer = malloc(SLOT_SIZE);
    strncpy(buffer,str,SLOT_SIZE-1);
    buffer[SLOT_SIZE-1] = '\0';

    char* start = buffer;
    while (*start == ' ' || *start == '\t' || *start == '\n' || *start == '-') {
        start++;
    }

    // Shift trimmed string to the beginning of buffer
    memmove(buffer, start, strlen(start) + 1);

    for (int i = strlen(buffer) - 1; i >= 0; i--){
        if(buffer[i] == '-' || buffer[i] == '\t' || buffer[i] == '\n' || buffer[i] == ' '){
            buffer[i] = '\0';
        }else{
            break;
        }
    }

    return buffer;
}

void print_database(Database db) {
    for (int i = 0; i < db.table_count; i++) {
        Table t = db.tables[i];
        printf("Table: %s\n", t.tablename);
        for (int j = 0; j < t.column_count; j++) {
            Column c = t.columns[j];
            printf("  Column: %s | Datatype: %s => ", c.columnname, c.datatype);
            for (int k = 0; k < c.data_count; k++) {
                printf("%s,",c.data[k]);
            }
            printf("\n");
        }
    }
        printf("\n");
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

char *indexify(char *data, int index) {
    char *indexified_data = malloc(strlen(data) + 10); // Allocate enough space for index
    if (!indexified_data) {
        perror("Failed to allocate memory");
        return NULL; // Handle memory allocation failure
    }
    sprintf(indexified_data, "%s(%d)", data, index);
    return indexified_data;
}

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

char** get_index_data(char* filename, int index, char *table_name){
    FILE *fp = fopen(filename,"r");
    if(!fp){
        perror("FILE NOT FOUND");
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
    return data;
}

int get_capacity(){
    FILE *fp = fopen("metadata.bin", "r+");
    if (!fp) return 0;

    int capacity;
    fread(&capacity, sizeof(int),1,fp);
    if (capacity <= 0) {
        capacity = 8; // Default capacity if not set
        fseek(fp, 0, SEEK_SET);
        fwrite(&capacity, sizeof(int), 1, fp);
    }
    fclose(fp);
    return capacity;
}

void update_capacity(int new_capacity) {
    FILE *fp = fopen("metadata.bin", "r+");
    if (!fp) return;

    fseek(fp, 0, SEEK_SET); // Move to the beginning of the file
    fwrite(&new_capacity, sizeof(int), 1, fp); // Write the new capacity
    fclose(fp);
}

void write_metadata_bin(char* filename){
    FILE *fp = fopen("metadata.bin", "w+b");
    if(!fp) return;

    int size = 8;
    FILE *fr = fopen(filename,"r");
    if(!fp){
        perror("FILE NOT FOUND");
    }
    // Write capacity at the start
    fseek(fp, 0, SEEK_SET);
    fwrite(&size, sizeof(int), 1, fp);

    Database db = parse_file(fr);

    Metadata_records records;
    // Start writing records after the header
    fseek(fp, sizeof(int), SEEK_SET);
    for(int i = 0; i < db.table_count; i++){
        Table *t = &db.tables[i];
        strcpy(records.tablename, t->tablename);
        for(int j = 0; j < t->column_count; j++){
            Column *c = &t->columns[j];
            strcpy(records.fieldname, c->columnname);
            records.index_count = c->data_count;
            fwrite(&records, sizeof(Metadata_records), 1, fp);
        }
    }
    fclose(fp); 
}

//currently useless will be useful in the future when we add a insert column or so feature otherwise write_metadata_bin will be used  to update metadata
void insert_metadata_record(char *tablename, char *fieldname, int index_count){
    FILE *fp = fopen("metadata.bin","a+");
    if (!fp){
        perror("Failed to open metadata file");
        return;
    }

    Metadata_records record;
    strcpy(record.tablename, tablename);
    strcpy(record.fieldname, fieldname);
    record.index_count = index_count;

    fseek(fp, 0, SEEK_END); // Move to the end of the file
    fwrite(&record, sizeof(Metadata_records), 1, fp);
}

int get_index_from_metadata(char *tablename, char* fieldname){
    FILE *fp = fopen("metadata.bin","r");
    if(!fp) {
        perror("Failed to open metadata file");
        return -1; // Error opening file
    }
    int index_count;

    Metadata_records record;
    fseek(fp, sizeof(int), SEEK_SET); // Skip the capacity integer at the start
    while(fread(&record,sizeof(Metadata_records),1,fp)){
        if(strcmp(record.tablename, tablename) == 0 && strcmp(record.fieldname, fieldname) == 0){
            index_count = record.index_count;
            fclose(fp);
            return index_count;
        }
    }
    
}
void update_metadatafile_inplace(const char *tablename, const char *fieldname, int new_index){
    FILE *fp = fopen("metadata.bin", "r+");
    if (!fp) {
        perror("Failed to open metadata file");
        return;
    }

    Metadata_records record;
    fseek(fp, sizeof(int), SEEK_SET); // Skip the capacity integer at the start
    while (fread(&record, sizeof(Metadata_records), 1, fp)) {
        if (strcmp(record.tablename, tablename) == 0 && strcmp(record.fieldname, fieldname) == 0) {
            record.index_count = new_index; 
            fseek(fp, sizeof(Metadata_records), SEEK_CUR); 
            fwrite(&record, sizeof(Metadata_records), 1, fp); 
            break;
        }
    }
    fclose(fp);
}