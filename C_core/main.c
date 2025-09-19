#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"
#include "utils.h"

int main(){
    // DELETE("MyDB.txt","data","tablename","column2");
    // INSERT("DORREL", "MyDB.txt", "tablename", "column2");
    // write_metadata_bin("MyDB.txt");
    printf("%d \n", get_capacity());
    // update_metadatafile_inplace("tablename","column2",6);
    // INSERT("darell","MyDB.txt","tablename","column1");
    // printf("%d", get_index_from_metadata("tablename","column1"));
    // print_metadata_bin();
    return 0;
}