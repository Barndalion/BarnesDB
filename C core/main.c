#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"

int main(){
    FILE *fp = fopen("MyDB.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    // char query[] = "CREATE(barnes_db_table) VALUES(column1,column2,column3,column4)";
    // QueryToken *a = parse_query(query);
    // print_tokens(a, token_count);
    
    // Database db = parse_file(fp);
    // print_database(db);

    // char *ann = "list(5000)";
    // int index = extract_index(ann);
    // printf("Extracted index: %d\n", index);

    char **data = get_index_data(1,"tablename");
    if (data) {
        for (int i = 0; data[i] != NULL; i++) {
            printf("Data at index 1: %s\n", data[i]);
        }
        free(data);
    } else {
        printf("No data found for the specified index.\n");
    }

    fclose(fp);
    return 0;
}