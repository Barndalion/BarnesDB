#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"

int main(){
    // INSERT("INSERT_DATA", "MyDB.txt", "tablename", "column1");
    FILE *fp = fopen("MyDB.txt", "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    Database db = parse_file(fp);
    print_database(db);
    write_metadata_bin("metadata.bin");
    

   fclose(fp);

    return 0;
}