#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"
#include "dbops.h"

int main(){
    INSERT("INSERT_DATA", "MyDB.txt", "tablename", "column1");
    return 0;
}