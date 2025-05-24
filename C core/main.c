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
    
    Database db = parse_file(fp);
    print_database(db);
    
    char *tablename = "tablename";
    char *columnname = "column1";

    Table *t = get_table(&db, tablename);
    Column *c = get_column(t,columnname);

    printf("Table: %s\n", t->tablename);
    for(int i = 0; i < t->column_count; i++){
        printf("Colums in the table above: %s\n", t->columns[i].columnname);
    }
    printf("Column: %s\n", c->columnname);
    for(int i = 0; i < c->data_count; i++){
        if(strcmp(c->datatype, "int") == 0 || strcmp(c->datatype, "float") == 0){
            printf("Data: %g\n", c->data.int_data[i]);
        } else {
            printf("Data: %s\n", c->data.data[i]);
        }
    }


  

    fclose(fp);
    return 0;
}