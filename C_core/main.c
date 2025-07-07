#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"

int main(){
    
    INSERT("LIONEL", "MyDB.txt", "tablename2", "column1");
    write_metadata_bin("MyDB.txt");

    return 0;
}