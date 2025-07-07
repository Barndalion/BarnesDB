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
    
    
    
    Database db = parse_file("MyDB.txt");
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

retrieve_data RETRIEVE(char *select_data, char *filename, char *From_table, char *Field_name){
    retrieve_data result;

   
    Database db = parse_file(filename);
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
    int inline_pos; // Position of the inline data in the line
    int capacity = get_capacity();
    int in_table = 0; // Flag to check if we are in the correct table

    long in_file_pos;
    while(1){
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
            char *start = buffer + 1; 
            char *end = strchr(buffer, '{');
            char length = end - start;
            char column_name[65];
            strncpy(column_name, start, length); 
            column_name[length] = '\0';
            
            if(strcmp(column_name,Field_name)==0){
                char *data_start = strchr(buffer,':');
                if(data_start != NULL){
                    inline_pos = (data_start - buffer) + 1;
                    printf("inline_pos: %d\n", inline_pos);
                    int index = get_index_from_metadata(From_table, Field_name);
                    printf("index: %d\n", index);
                    update_metadatafile_inplace(From_table, Field_name, index + 1);
                    if(capacity <= index){
                        int old_capacity = get_capacity();
                        update_capacity(old_capacity * 2);
                        // TODO: ADD EXPONENTIAL GROWTH
                        fprintf(stderr, "Capacity exceeded. Cannot insert more data.\n");
                        break;
                    }
                    printf("in_file_pos: %ld\n", in_file_pos);
                    long targetted_offset = in_file_pos + (long)inline_pos + (long)(index * (SLOT_SIZE));

                    char slot_buffer[SLOT_SIZE];
                    memset(slot_buffer, '-', sizeof(slot_buffer));
                    char *modified_data = indexify(Insert_Data, index);
                    memcpy(slot_buffer, modified_data,strlen(modified_data));
                
                    fseek(fp, targetted_offset, SEEK_SET);
                    fwrite(slot_buffer, sizeof(char), SLOT_SIZE-1, fp);
                    fputc(',',fp);
                    
                    
                    return;
                    
                }
            }
        }
    }
    fclose(fp);
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
    // Write capacity at the start
    fseek(fp, 0, SEEK_SET);
    fwrite(&size, sizeof(int), 1, fp);

    Database db = parse_file(filename);
    print_database(db);

    Metadata_records records;
    // Start writing records after the header
    fseek(fp, sizeof(int), SEEK_SET);
    for(int i = 0; i < db.table_count; i++){
        Table *t = &db.tables[i];
        strcpy(records.tablename, t->tablename);
        for(int j = 0; j < t->column_count; j++){
            Column *c = &t->columns[j];
            strcpy(records.fieldname, c->columnname);
            printf("count: %d\n", c->data_count);
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
void update_metadatafile_inplace(const char *tablename, const char *fieldname, int new_index) {
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
