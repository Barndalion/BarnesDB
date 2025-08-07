#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"

int main(){

    DELETE("MyDB.txt","FUCKING","tablename2","column2");
    // INSERT("LIONELll", "MyDB.txt", "tablename2", "column1");
    // write_metadata_bin("MyDB.txt");
    printf("completed");
    return 0;
}