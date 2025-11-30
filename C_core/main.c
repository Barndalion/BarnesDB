#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"
#include "utils.h"

int main(){
    initialize_runtime_database("hogwarts");
    // DELETE(active_database(),"DORREL","tablename","column2"); /* NOTE: hardcoded DB filename */
    // INSERT("DORREL", active_database(), "tablename", "column1"); /* NOTE: hardcoded DB filename */
    // write_metadata_bin("MyDB.txt"); /* NOTE: hardcoded DB filename */
    // printf("%d \n", get_capacity());
    // update_metadatafile_inplace("tablename","column2",6);
    // INSERT("darell","MyDB.txt","tablename","column2"); /* NOTE: hardcoded DB filename */
    // printf("%d", get_index_from_metadata("tablename","column1"));
    // print_metadata_bin();

    // printf("db_name: %s",active_database());
    // printf("metadata file: %s", metadata_file());
    return 0;
}