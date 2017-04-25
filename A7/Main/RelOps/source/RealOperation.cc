#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Catalog.h"
#include "ExprTree.h"
#include "MyDB_Record.h"
#include "RegularSelection.h"
#include "Aggregate.h"
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
    // tablesToProcess<tableName, alias>
    vector <pair <string, string>> tablesToProcess = query.getTables();
    
    // STEP 1 set input table
    //TODO should be a vector of tables
    /*

    */
    string fullTableName = tablesToProcess.front().first;
    MyDB_TableReaderWriterPtr inputTableReaderWriter = allTables[fullTableName];
    inputTableReaderWriter->loadFromTextFile("./" + fullTableName + ".tbl");
    
    // STEP 2 set output table 
    MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
    vector <ExprTreePtr> valuesToSelect = query.getAllValues();
    vector <string> projections;

    bool isAgg = false;
    vector <pair <MyDB_AggType, string>> aggsToCompute;
    vector <string> groupings;
    for (auto v : valuesToSelect){
        //cout <<"[RealOperation line 41] " << v->toString() << "\n";
        
        mySchemaOut->appendAtt(v->getAttPair(catalog, fullTableName));
        projections.push_back(v->toString());

        if(v->getType() == MyDB_ExprType::sumExpr){
            isAgg = true;
            aggsToCompute.push_back(make_pair(MyDB_AggType::sumAgg, v->toString().substr(3)));
        }
        else if(v->getType() == MyDB_ExprType::avgExpr){
            isAgg = true;
            aggsToCompute.push_back(make_pair(MyDB_AggType::avgAgg, v->toString().substr(3)));
        }
        else{
            groupings.push_back(v->toString());
        }
    }
    
    MyDB_TablePtr myTableOut = make_shared <MyDB_Table> ("tableOut", "tableOut.bin", mySchemaOut);
    MyDB_TableReaderWriterPtr outputTableReadWriter = make_shared <MyDB_TableReaderWriter> (myTableOut, bufferMgr);

    //aggregate?


    vector <ExprTreePtr> allDisjunctions = query.getAllDisjunctions();
    vector <string> allPredicates;
    for (auto d : allDisjunctions){
        allPredicates.push_back(d->toString());
    }
    
    string selectPredicate = parseStringPredicate(allPredicates);


    /*
    aggregate
    Aggregate (MyDB_TableReaderWriterPtr input, MyDB_TableReaderWriterPtr output,
		vector <pair <MyDB_AggType, string>> aggsToCompute,
		vector <string> groupings, string selectionPredicate);
    */
    if(isAgg){
        //cout << "[RealOperation line 81] do aggregate \n";
        Aggregate selectionOp (inputTableReaderWriter, outputTableReadWriter, aggsToCompute, groupings, selectPredicate);
        selectionOp.run();
    }
    else{
        RegularSelection selectionOp (inputTableReaderWriter, outputTableReadWriter, selectPredicate, projections);
        selectionOp.run();
    }

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