
#ifndef AGG_CC
#define AGG_CC

#include "MyDB_Record.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"
#include "Aggregate.h"
#include <unordered_map>

using namespace std;

Aggregate :: Aggregate (MyDB_TableReaderWriterPtr inputIn, MyDB_TableReaderWriterPtr outputIn,
                vector <pair <MyDB_AggType, string>> aggsToComputeIn,
                vector <string> groupingsIn, string selectionPredicateIn) {

	input = inputIn;
	output = outputIn;
	aggsToCompute = aggsToComputeIn;
	groupings = groupingsIn;
	selectionPredicate = selectionPredicateIn;

}

void Aggregate :: run () {

	// make sure that the number of attributes is OK
	if (output->getTable ()->getSchema ()->getAtts ().size () != aggsToCompute.size () + groupings.size ()) {
		cout << "error, the output schema needs to have the same number of atts as (# of aggs to compute + # groups).\n";
		return;
	}

	// first, we create a schema for the aggregate records... this is all grouping atts,
	// followed by all aggregate atts, followed by the count att
	MyDB_SchemaPtr aggSchema = make_shared <MyDB_Schema> ();
	int i = 0;
	int numGroups = groupings.size ();
	//cout << "[Aggregate line 38] groupings size " << numGroups << "\n";
	for (auto &a : output->getTable ()->getSchema ()->getAtts ()) {
		if (i < numGroups) 
			aggSchema->appendAtt (make_pair ("MyDB_GroupAtt" + to_string (i++), a.second));
		else{
			
			aggSchema->appendAtt (make_pair ("MyDB_AggAtt" + to_string (i++ - numGroups), a.second));
			//cout << "[Aggregate line 45] MyDB_AggAtt" <<  to_string (i - numGroups - 1) << " , " << a.second->toString() << "\n";
		}
	}
	aggSchema->appendAtt (make_pair ("MyDB_CntAtt", make_shared <MyDB_IntAttType> ()));

	// now, create the schema for the combined records
	MyDB_SchemaPtr combinedSchema = make_shared <MyDB_Schema> ();
	for (auto &a : input->getTable ()->getSchema ()->getAtts ())
		combinedSchema->appendAtt (a);
	for (auto &a : aggSchema->getAtts ())
		combinedSchema->appendAtt (a);

	// now, get an intput rec, an agg rec, and a combined rec
	MyDB_RecordPtr inputRec = input->getEmptyRecord ();
	MyDB_RecordPtr aggRec = make_shared <MyDB_Record> (aggSchema);
	MyDB_RecordPtr combinedRec = make_shared <MyDB_Record> (combinedSchema);
	combinedRec->buildFrom (inputRec, aggRec);
	
	// this is the current page where we are writing aggregate records
	MyDB_PageReaderWriter lastPage (true, *(input->getBufferMgr ()));

	// this is the list all of the pages used to store aggregate records
	vector <MyDB_PageReaderWriter> allPages;
	allPages.push_back (lastPage);

	// this is the hash index for all of the aggregate records
	unordered_map <size_t, vector <void *>> myHash;

	// this will compute each of the groupings
	vector <func> groupingComps;
	for (auto &s : groupings) {
		//cout << "[Aggregate line 76] " << s << "\n";
		groupingComps.push_back (inputRec->compileComputation (s));
	}

	// and this will verify that each of the groupings match up
	func checkGroups;
	string groupCheck;
	i = 0;

	// in case there is not a grouping...
	groupCheck = "bool[true]";

	for (auto &s : groupings) {
		string curClause = "== (" + s + ", [MyDB_GroupAtt" + to_string (i) + "])";
		if (i == 0) {
			groupCheck = curClause;
		} else {
			groupCheck = "&& (" + curClause + ", " + groupCheck + ")";
		}
		i++;
	}
	//cout << "[Aggregate line 96] groupCheck " << groupCheck << "\n";
	checkGroups = combinedRec->compileComputation (groupCheck);	

	// this will compute each of the aggregates for updating the aggregate record
	vector <func> aggComps;

	// this will compute the final aggregate value for each output record
	vector <func> finalAggComps;
	//cout << "[Aggregate line 103] build aggComps \n";
	i = 0;
	for (auto &s : aggsToCompute) {
		//cout <<  "[Aggregate line 106] i " << i << "\n";
		if (s.first == MyDB_AggType :: sumAgg || s.first == MyDB_AggType :: avgAgg) {
			aggComps.push_back (combinedRec->compileComputation ("+ (" + s.second + 
				", [MyDB_AggAtt" + to_string (i) + "])"));
				//cout << "[Aggregate line 110] + (" + s.second + ", [MyDB_AggAtt" + to_string (i) + "])" << "\n";


		} else if (s.first == MyDB_AggType :: cntAgg) {
			aggComps.push_back (combinedRec->compileComputation ("+ ( int[1], [MyDB_AggAtt"
				+ to_string (i) + "])"));
				//cout << " [Aggregate line 117] + ( int[1], [MyDB_AggAtt" + to_string (i) + "])" << "\n";

		}

		if (s.first == MyDB_AggType :: avgAgg) {
			finalAggComps.push_back (combinedRec->compileComputation ("/ ([MyDB_AggAtt" + to_string (i++) + "], [MyDB_CntAtt])"));
		} else {
			finalAggComps.push_back (combinedRec->compileComputation ("[MyDB_AggAtt" + to_string (i++) + "]"));
		}
	}
	aggComps.push_back (combinedRec->compileComputation ("+ ( int[1], [MyDB_CntAtt])"));

	// and this runs the selection on the input records
	func inputPred = inputRec->compileComputation (selectionPredicate);

	// at this point, we are ready to go!!
	MyDB_RecordIteratorPtr myIter = input->getIterator (inputRec);
	MyDB_AttValPtr zero = make_shared <MyDB_IntAttVal> ();
	int limit = 10;
	//cout << "[Aggregate line 135] iterate the record \n";
	while (myIter->hasNext ()) {

		myIter->getNext ();

		// see if it is accepted by the preicate
		if (!inputPred ()->toBool ()) {
			
			continue;
		}

		// hash the current record
		
		size_t hashVal = 0;
		for (auto &f : groupingComps) {
			hashVal ^= f ()->hash ();
		}
		

		// if there is a match, then get the list of matches
		vector <void *> &potentialMatches = myHash [hashVal];
		void *loc = nullptr;

		// and iterate though the potential matches, checking each of them
		for (auto &v : potentialMatches) {	

			aggRec->fromBinary (v);

			// check to see if it matches
			
			if (!checkGroups ()->toBool ()) {
				//cout << "[Aggregate line 167] check point \n";
				continue;
			}
			//cout << "[Aggregate line 170] check point \n";

			loc = v;
			break;
		}

		// if we did not find a match...
		if (loc == nullptr) {

			// set up the record...
			i = 0;
			for (auto &f : groupingComps) {
				aggRec->getAtt (i++)->set (f ());
				
			}
			for (int j = 0; j < aggComps.size (); j++) {
				aggRec->getAtt (i++)->set (zero);
			}
		}

		// update each of the aggregates
		i = 0;
		for (auto &f : aggComps) {
			//cout << "[Aggregate line 190] run f() " << i << "\n";
			aggRec->getAtt (numGroups + i++)->set (f ());
			
		}

		// if we did not find a match, write to a new location...
		aggRec->recordContentHasChanged ();
		if (loc == nullptr) {
			loc = lastPage.appendAndReturnLocation (aggRec);

			// if we could not write, then the page was full
			if (loc == nullptr) {
				MyDB_PageReaderWriter nextPage (true, *(input->getBufferMgr ()));
				lastPage = nextPage;
				allPages.push_back (lastPage);
				loc = lastPage.appendAndReturnLocation (aggRec);	
			}

			aggRec->fromBinary (loc);
			myHash [hashVal].push_back (loc);

		// otherwise, re-write to the old location
		} else {
			aggRec->toBinary (loc);
		}
	}

	// now, we have processed all of the database records... so we can output the aggregates
	MyDB_RecordIteratorAltPtr myIterAgain = getIteratorAlt (allPages);	

	// loop through all of the aggregate records
	MyDB_RecordPtr outRec = output->getEmptyRecord ();
	while (myIterAgain->advance ()) {

		myIterAgain->getCurrent (aggRec);

		// set the grouping atts
		for (i = 0; i < numGroups; i++) {
			outRec->getAtt (i)->set (aggRec->getAtt (i));
		}

		// set the aggregate atts
		for (auto &a : finalAggComps) {
			outRec->getAtt (i++)->set (a ());
		}
		outRec->recordContentHasChanged ();
		output->append (outRec);
	}
}

#endif

