#ifndef DB_OPS_H
#define DB_OPS_H

#include "parser.h"

Table* get_table(Database *db, char *tablename);
Column* get_column(Table *table, char *columnname);

#endif