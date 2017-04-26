#include "ParserTypes.h"
#include "MyDB_TableReaderWriter.h"
#include "MyDB_Catalog.h"
#include "ExprTree.h"
#include "MyDB_Record.h"
#include "RegularSelection.h"
#include "Aggregate.h"
#include "RealOperation.h"
#include "ScanJoin.h"


RealOperation :: RealOperation(SQLStatement *inputSql, MyDB_CatalogPtr inputCatalog, 
        map <string, MyDB_TableReaderWriterPtr> inputTables, MyDB_BufferManagerPtr inputMgr){
            sql = inputSql;
            catalog = inputCatalog;
            allTables = inputTables;
            bufferMgr = inputMgr;
}

MyDB_TableReaderWriterPtr RealOperation :: joinTwoTable(){
    SFWQuery query = sql->getQuery();
    vector <pair <string, string>> tablesToProcess = query.getTables();
    string leftTbName = tablesToProcess[1].first;
    string rightTbName = tablesToProcess[0].first;
    string leftAlias = tablesToProcess[1].second;
    string rightAlias = tablesToProcess[0].second;
    cout << "left table: " << leftTbName << "\n";
    cout << "right table: " << rightTbName << "\n";

    MyDB_TableReaderWriterPtr leftTableReaderWriter = allTables[leftTbName];
    MyDB_TableReaderWriterPtr rightTableReaderWriter = allTables[rightTbName];
    leftTableReaderWriter->loadFromTextFile("./"+leftTbName+".tbl");
    rightTableReaderWriter->loadFromTextFile("./"+rightTbName+".tbl");

    MyDB_SchemaPtr mySchemaOut = make_shared <MyDB_Schema> ();
    vector <string> projections;
    MyDB_SchemaPtr leftSchema = leftTableReaderWriter->getTable()->getSchema();
    vector <pair <string, MyDB_AttTypePtr>> leftAtts = leftSchema -> getAtts();
    for (auto &p : leftAtts){
        mySchemaOut->appendAtt(p);
        projections.push_back("["+p.first+"]");
    }
    MyDB_SchemaPtr rightSchema = rightTableReaderWriter->getTable()->getSchema();
    vector <pair <string, MyDB_AttTypePtr>> rightAtts = rightSchema -> getAtts();
    for (auto &p : rightAtts){
        mySchemaOut->appendAtt(p);
        projections.push_back("["+p.first+"]");
    }

    MyDB_TablePtr myTableOut = make_shared <MyDB_Table> ("tableOut", "tableOut.bin", mySchemaOut);
    MyDB_TableReaderWriterPtr outputTableReadWriter = make_shared <MyDB_TableReaderWriter> (myTableOut, bufferMgr);

    vector <ExprTreePtr> allDisjunctions = query.getAllDisjunctions();
    vector <pair <string, string>> hashAtts;
    vector <string> finalPredicates;
    vector <string> leftPredicates;
    vector <string> rightPredicates;

    for (auto v : allDisjunctions){
        pair<bool, string> isJoinPair = v->IsJoinPredicate();
        if (isJoinPair.first){
            string delimiter = "|";
            string atts = isJoinPair.second;
            int pos = atts.find(delimiter);
            string leftAtt = atts.substr(0, pos);
            string rightAtt = atts.substr(pos+1, atts.size()-pos-1);
            finalPredicates.push_back(v->toString());
            hashAtts.push_back(make_pair(leftAtt, rightAtt));
            cout << v->toString() << "\n";
            cout << "atts: " << atts << "\n";
            cout << "left att: " << leftAtt << "\n";
            cout << "right att: " << rightAtt << "\n";
        }
        else{
            if (isJoinPair.second == leftAlias){
                leftPredicates.push_back(v->toString());
            }
            else{
                rightPredicates.push_back(v->toString());
            }

        }
    }

    string finalSelectionPredicate = parseStringPredicate(finalPredicates);
    string leftSelectionPredicate = parseStringPredicate(leftPredicates);
    string rightSelectionPredicate = parseStringPredicate(rightPredicates);
    cout << "final sel: " << finalSelectionPredicate << "\n";
    cout << "left sel: " << leftSelectionPredicate << "\n";
    cout << "right sel: " << rightSelectionPredicate << "\n";


    ScanJoin myOp (leftTableReaderWriter, rightTableReaderWriter, outputTableReadWriter,
                    finalSelectionPredicate, projections, hashAtts, leftSelectionPredicate, rightSelectionPredicate);
    myOp.run();
    return outputTableReadWriter;

    MyDB_RecordPtr temp = outputTableReadWriter->getEmptyRecord ();
    MyDB_RecordIteratorAltPtr myIter = outputTableReadWriter->getIteratorAlt ();
/*
    int counter = 0;
    int limit = 1;
    while (myIter->advance ()) {
            myIter->getCurrent (temp);
            if (limit > 0){
                cout << temp << "\n";
            }
            limit--;
            counter++;
    }
    cout << "Total " << counter << " records found. \n";
    */

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
    MyDB_TableReaderWriterPtr inputTableReaderWriter  = nullptr;
    if (tablesToProcess.size() == 2){
        inputTableReaderWriter = joinTwoTable();
    }
    else{
        inputTableReaderWriter = allTables[fullTableName];
        inputTableReaderWriter->loadFromTextFile("./" + fullTableName + ".tbl");
    }

    
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