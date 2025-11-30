#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dbops.h"
#include "parser.h"
#include "utils.h"
static char active_metadata_file[512] = "MyDB.bin";
//TO MUCH RELIANCE ON AI LOOK OVER
char *trim(char *str){
    /* POTENTIAL MEMORY LEAK: this function allocates and returns a new buffer
       using malloc(). Callers must free() the returned pointer to avoid
       leaking memory. */
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

/* POTENTIAL MEMORY LEAK: indexify allocates and returns a new string via
    malloc(); callers must free() this buffer when no longer needed. */
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
    /* POTENTIAL MEMORY LEAK: this function allocates a temporary string
       `index` using malloc() and returns an int converted by atoi(). The
       allocated `index` is not freed before the function returns, causing a
       memory leak. Consider using a stack buffer or free(index) before
       returning. */
    char* index_start = strchr(data, '(');
    char* index_end = strchr(data, ')');

    int index_len = index_end - index_start - 1;
    if (index_start && index_end && index_len > 0) {
        char *index = malloc(index_len + 1);
        strncpy(index, index_start + 1, index_len);
        index[index_len] = '\0';

        /* NOTE: index is not freed; this leaks memory. Add free(index) before
           returning to avoid the leak, e.g.:
               int val = atoi(index);
               free(index);
               return val; */
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
        /* POTENTIAL MEMORY LEAK: remove_index_tag_copy returns allocated
           memory which must be freed. `comp_data` should be freed after use,
           otherwise this loop will leak for every iteration. */
        char* comp_data = remove_index_tag_copy(c->data[i]);
        if (strcmp(comp_data, data) == 0) {
            /* NOTE: comp_data must be freed before returning */
            return return_index(c->data[i]);
        }
    }

    fprintf(stderr, "Data not found in column.\n");
    return -1;   
}

/* NOTE: remove_index_tag_copy returns a malloc'd/strdup'd buffer. Callers
    must free() the returned string after use. */
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
    /* WARNING: this function opens a FILE* (fp) but does not fclose(fp).
       This can leak file descriptors. Also parse_file(fp) allocates heap
       memory for the returned Database; this memory is never freed here.
       The returned `char**` contains malloc()/strdup()'d strings; the
       caller must free the char* items and the array itself. */
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
    FILE *fp = fopen(metadata_file(), "r+"); /* NOTE: hardcoded metadata filename - consider deriving from DB filename */
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
    FILE *fp = fopen(metadata_file(), "r+"); /* NOTE: hardcoded metadata filename - consider deriving from DB filename */
    if (!fp) return;

    fseek(fp, 0, SEEK_SET); // Move to the beginning of the file
    fwrite(&new_capacity, sizeof(int), 1, fp); // Write the new capacity
    fclose(fp);
}
// ISSUE: THere is a index issue when writing because when a data is deleted at a random spot it wont align with how the index is added to the metaata bin. modify this function to write the index of the last piece of data + 1
void write_metadata_bin(char* filename){
    FILE *fp = fopen(active_metadata_file, "r+"); 

    int buffer;
    fread(&buffer,sizeof(int),1,fp);
    if(!buffer){
            buffer = 8;
            fseek(fp,0,SEEK_SET);
            fwrite(&buffer,sizeof(int),1,fp);
    }else{
            fseek(fp,sizeof(int),SEEK_SET);
        }

    FILE *fr = fopen(filename,"r");
    if(!fr){
        perror("FILE NOT FOUND");
        return;
    }
    Metadata_records metadata;
    char* tablename;
    Database db = parse_file(fr);
    for(int i = 0;i<db.table_count;i++){
        Table t = db.tables[i];
        for(int j = 0;j<t.column_count;j++){
            strncpy(tablename,t.tablename,sizeof(t.tablename));
            Column c = t.columns[j];

            int index = return_index(c.data[c.data_count - 1]);
            
            strncpy(metadata.tablename,t.tablename,64);
            strncpy(metadata.fieldname,c.columnname,64);
            metadata.index_count = index + 1;

            fseek(fp,0,SEEK_CUR);
            fwrite(&metadata,sizeof(Metadata_records),1,fp);
        }
    }
     /* WARNING: this code calls fcloseall() which may not be portable. The
         file pointers `fr` and `fp` opened earlier are not explicitly
         closed via fclose() here. Prefer calling fclose(fr); fclose(fp); to
         close resources. Otherwise file descriptors can leak. */
     fcloseall();
}

//currently useless will be useful in the future when we add a insert column or so feature otherwise write_metadata_bin will be used  to update metadata
void insert_metadata_record(char *tablename, char *fieldname, int index_count){
    /* WARNING: The FILE* `fp` is not closed in this function; add fclose(fp)
       to avoid leaking file descriptors. */
    FILE *fp = fopen(metadata_file(),"a+"); /* NOTE: hardcoded metadata filename - consider deriving from DB filename */
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
    FILE *fp = fopen(metadata_file(),"r"); /* NOTE: hardcoded metadata filename - consider deriving from DB filename */
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
    fclose(fp);
    return -1; // not found
}
void update_metadatafile_inplace(const char *tablename, const char *fieldname, int new_index){
    FILE *fp = fopen(metadata_file(), "r+"); /* NOTE: hardcoded metadata filename - consider deriving from DB filename */
    if (!fp) {
        perror("Failed to open metadata file");
        return;
    }

    Metadata_records record;
    fseek(fp, sizeof(int), SEEK_SET); // Skip the capacity integer at the start
    while (fread(&record, sizeof(Metadata_records), 1, fp)) {
        if (strcmp(record.tablename, tablename) == 0 && strcmp(record.fieldname, fieldname) == 0) {
            record.index_count = new_index; 
            // move file position back to the start of this record so we overwrite it
            if (fseek(fp, -((long)sizeof(Metadata_records)), SEEK_CUR) != 0) {
                perror("fseek failed");
                break;
            }
            fwrite(&record, sizeof(Metadata_records), 1, fp); 
            fflush(fp);
            break;
        }
    }
    fclose(fp);
}
void print_metadata_bin(){
    FILE* fp = fopen(metadata_file(),"r"); /* NOTE: hardcoded metadata filename - consider deriving from DB filename */
    if(!fp){
        perror("NO FILE FOUND");
        return;
    }
    
    Metadata_records buffer;
    fseek(fp,sizeof(int),SEEK_SET);
    while(fread(&buffer,sizeof(Metadata_records),1,fp)){
        printf("Table Name: %s\nField Name: %s\nIndex Count: %d\n",buffer.tablename,buffer.fieldname,buffer.index_count);
        
    }
    fclose(fp);
}

/* Active metadata filename (writable buffer). Defaults to "metadata.bin".
   Call initialize_runtime_database("MyDB.txt") to derive a per-DB metadata file
   (e.g. MyDB.bin). */


void initialize_runtime_database(const char* database_name){
    if(!database_name) return;
    size_t len =  strlen(database_name);
     /* POTENTIAL MEMORY LEAK: `name` is allocated with malloc() and only freed
         in a branch where the file didn't exist. If the file exists or the
         code takes other branches, `name` is not freed. Either free `name`
         in all paths or avoid heap allocation here (use a stack buffer). */
     char* name = malloc(sizeof(len) + 5);
    const char* dot = strrchr(database_name,'.');
    if(!dot){
        snprintf(active_metadata_file, sizeof(active_metadata_file), "%s.bin", database_name);
        snprintf(name,len + 5,"%s.txt",database_name);
        if(!file_exists_fopen(name)){
            FILE* fp = fopen(name, "w");
            free(name);
            if(!fp){
                perror("ERROR OCCURED!");
                return;
            }
        }
        if(!file_exists_fopen(active_metadata_file)){
            FILE* fp = fopen(active_metadata_file, "w");
            if(!fp){
                perror("ERROR OCCURED!");
                return;
            }
        }
    }
}

int file_exists_fopen(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1; // File exists
    }
    return 0; // File does not exist
}

const char* active_database(void){
    static char db_name[512];
    // Copy the active metadata filename and replace the extension with .txt
    strncpy(db_name, active_metadata_file, sizeof(db_name)-1);
    db_name[sizeof(db_name)-1] = '\0';
    char *dot = strrchr(db_name, '.');
    if (dot) *dot = '\0';
    strncat(db_name, ".txt", sizeof(db_name) - strlen(db_name) - 1);
    return db_name;
}

const char* metadata_file(void){
    return active_metadata_file;
}