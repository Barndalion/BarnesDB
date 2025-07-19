#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"

int main(){
    
    INSERT("LIONELll", "MyDB.txt", "tablename2", "column2");
    write_metadata_bin("MyDB.txt");
    return 0;
}