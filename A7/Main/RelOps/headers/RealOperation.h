#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Catalog.h"

class RealOperation {

public:

    RealOperation(SQLStatement *sql, MyDB_CatalogPtr myCatalog, 
            map <string, MyDB_TableReaderWriterPtr> allTables, MyDB_BufferManagerPtr bufferMgr);

    void run();

private:

    SQLStatement *sql;
    MyDB_CatalogPtr catalog;
    map <string, MyDB_TableReaderWriterPtr> allTables;
    MyDB_BufferManagerPtr bufferMgr;

    string parseStringPredicate(vector<string> allPredicates);
    MyDB_TableReaderWriterPtr joinTwoTable();
};