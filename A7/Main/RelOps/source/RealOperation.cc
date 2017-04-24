#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Catalog.h"
#include "ExprTree.h"
#include "MyDB_Record.h"
#include "RegularSelection.h"

#include "RealOperation.h"

RealOperation :: RealOperation(SQLStatement *inputSql, MyDB_CatalogPtr inputCatalog, 
        map <string, MyDB_TableReaderWriterPtr> inputTables, MyDB_BufferManagerPtr inputMgr){
            sql = inputSql;
            catalog = inputCatalog;
            allTables = inputTables;
            bufferMgr = inputMgr;
}

void RealOperation :: run() {
    SFWQuery query = sql->getQuery();
    vector <pair <string, string>> tablesToProcess = query.getTables();
    string fullTableName = tablesToProcess.front().first;
    MyDB_TableReaderWriterPtr inputTableReaderWriter = allTables[fullTableName];
    inputTableReaderWriter->loadFromTextFile("../" + fullTableName + ".tbl");
    
    MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
    vector <ExprTreePtr> valuesToSelect = query.getAllValues();
    vector <string> projections;
    for (auto v : valuesToSelect){
        cout << v->toString() << "\n";
        mySchemaOut->appendAtt(v->getAttPair(catalog, fullTableName));
        projections.push_back(v->toString());
    }
    
    MyDB_TablePtr myTableOut = make_shared <MyDB_Table> ("tableOut", "tableOut.bin", mySchemaOut);
    MyDB_TableReaderWriterPtr outputTableReadWriter = make_shared <MyDB_TableReaderWriter> (myTableOut, bufferMgr);

    vector <ExprTreePtr> allDisjunctions = query.getAllDisjunctions();
    vector <string> allPredicates;
    for (auto d : allDisjunctions){
        allPredicates.push_back(d->toString());
    }
    
    string selectPredicate = parseStringPredicate(allPredicates);
    RegularSelection selectionOp (inputTableReaderWriter, outputTableReadWriter, selectPredicate, projections);
    selectionOp.run();

    MyDB_RecordPtr temp = outputTableReadWriter->getEmptyRecord ();
    MyDB_RecordIteratorAltPtr myIter = outputTableReadWriter->getIteratorAlt ();

    int counter = 0;
    int limit = 30;
    while (myIter->advance ()) {
            myIter->getCurrent (temp);
            if (limit > 0){
                cout << temp << "\n";
            }
            limit--;
            counter++;
    }
    cout << "Total " << counter << " records found. \n";
}

string RealOperation :: parseStringPredicate(vector<string> allPredicates){
    if (allPredicates.size() == 1){
        return allPredicates.front();
    }
    else{
        string res = "&& (" + allPredicates[0] + ", " + allPredicates[1] + ")";
        for (int i = 2; i < allPredicates.size(); i++){
            res = "&& (" + res + ", " + allPredicates[i] + ")";
        }
        return res;
    }
}