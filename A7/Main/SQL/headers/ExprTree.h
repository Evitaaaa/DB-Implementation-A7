
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "MyDB_AttType.h"
#include "MyDB_Catalog.h"
#include <string>
#include <vector>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;
enum MyDB_ExprType {boolExpr, doubleExpr, intExpr, stringExpr, identifierExpr, minusExpr,
					plusExpr, timesExpr, divideExpr, gtExpr, ltExpr, neqExpr, orExpr, eqExpr,
					notExpr, sumExpr, avgExpr};
enum MyDB_ExprAttType {boolAtt, doubleAtt, intAtt, stringAtt};

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
	virtual string toString () = 0;
	virtual pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName) = 0;
	virtual MyDB_ExprType getType() = 0;
	virtual MyDB_ExprAttType getAttType() = 0;
	virtual pair<bool, string> IsJoinPredicate() = 0;
	virtual ~ExprTree () {}
};

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}
	
	MyDB_ExprType getType() {
		return MyDB_ExprType::boolExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: boolAtt;
	}

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::doubleExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: doubleAtt;
	}		

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_DoubleAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~DoubleLiteral () {}
};

// this implement class ExprTree
class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::intExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: intAtt;
	}

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_IntAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::stringExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: stringAtt;
	}

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		cout <<"[ExprTree 147] getAttPair for string \n"; 
		return make_pair("", make_shared <MyDB_StringAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
	MyDB_ExprAttType attType2;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
		attType2 = MyDB_ExprAttType::boolAtt;
	}

	string toString () {
		return "[" + attName + "]";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::identifierExpr;
	}

	MyDB_ExprAttType getAttType() {
		return attType2;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName) override {
		string type;
        string attr_name = tbName + "." + attName + ".type";
		
		MyDB_AttTypePtr attType;
		if(catalog -> getString(attr_name, type)){
			if(type.compare("bool") == 0){
				attType = make_shared <MyDB_BoolAttType> ();
				attType2 = MyDB_ExprAttType::boolAtt;

			}
			else if(type.compare("string") == 0){
				attType = make_shared <MyDB_StringAttType> ();
				attType2 = MyDB_ExprAttType::stringAtt;
			}
			else if(type.compare("int") == 0){
				attType = make_shared <MyDB_IntAttType> ();
				attType2 = MyDB_ExprAttType::stringAtt;
			}
			else if(type.compare("double") == 0){
				attType = make_shared <MyDB_DoubleAttType> ();
				attType2 = MyDB_ExprAttType::stringAtt;
			}
		}
		return make_pair("[" + attName + "]", attType);
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, tableName);
	}

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::minusExpr;
	}
	MyDB_ExprAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExprAttType::doubleAtt || rhs->getAttType() == MyDB_ExprAttType::doubleAtt){
			return MyDB_ExprAttType::doubleAtt;
		}
		else{
			return MyDB_ExprAttType::intAtt;
		}
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::plusExpr;
	}

	MyDB_ExprAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExprAttType::stringAtt){
			return MyDB_ExprAttType ::stringAtt;
		}

		if(lhs->getAttType() == MyDB_ExprAttType::doubleAtt || rhs->getAttType() == MyDB_ExprAttType::doubleAtt){
			return MyDB_ExprAttType::doubleAtt;
		}
		else{
			return MyDB_ExprAttType::intAtt;
		}
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		if(getAttType() == MyDB_ExprAttType::stringAtt){
			return make_pair("", make_shared <MyDB_StringAttType> ());
		}
		else if(getAttType() == MyDB_ExprAttType::doubleAtt){
			return make_pair("", make_shared <MyDB_DoubleAttType> ());
		}
		else{
			return make_pair("", make_shared <MyDB_IntAttType> ());
		}
		
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::timesExpr;
	}

	MyDB_ExprAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExprAttType::doubleAtt || rhs->getAttType() == MyDB_ExprAttType::doubleAtt){
			return MyDB_ExprAttType::doubleAtt;
		}
		else{
			return MyDB_ExprAttType::intAtt;
		}
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::divideExpr;
	}

	MyDB_ExprAttType getAttType() {
		if(lhs->getAttType() == MyDB_ExprAttType::doubleAtt || rhs->getAttType() == MyDB_ExprAttType::doubleAtt){
			return MyDB_ExprAttType::doubleAtt;
		}
		else{
			return MyDB_ExprAttType::intAtt;
		}
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){

		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::gtExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: boolAtt;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		pair<bool, string> left = lhs -> IsJoinPredicate();
		pair<bool, string> right = rhs -> IsJoinPredicate();
		
		if (left.second == right.second || right.second == ""){
			return make_pair(false, left.second);
		}
		else{
			return make_pair(true, "");
		}
	}

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::ltExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: boolAtt;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		pair<bool, string> left = lhs -> IsJoinPredicate();
		pair<bool, string> right = rhs -> IsJoinPredicate();
		if (left.second == right.second || right.second == ""){
			return make_pair(false, left.second);
		}
		else{
			return make_pair(true, "");
		}
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::neqExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: boolAtt;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		pair<bool, string> left = lhs -> IsJoinPredicate();
		pair<bool, string> right = rhs -> IsJoinPredicate();
		if (left.second == right.second || right.second == ""){
			return make_pair(false, left.second);
		}
		else{
			return make_pair(true, "");
		}
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::orExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: boolAtt;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, lhs->IsJoinPredicate().second);
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::eqExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: boolAtt;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		pair<bool, string> left = lhs -> IsJoinPredicate();
		pair<bool, string> right = rhs -> IsJoinPredicate();
		if (left.second == right.second || right.second == ""){
			return make_pair(false, left.second);
		}
		else{
			return make_pair(true, "");
		}
	}

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::notExpr;
	}

	MyDB_ExprAttType getAttType() {
		return MyDB_ExprAttType :: boolAtt;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::sumExpr;
	}

	MyDB_ExprAttType getAttType() {
		return child->getAttType();
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("sum", make_shared <MyDB_DoubleAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::avgExpr;
	}

	MyDB_ExprAttType getAttType() {
		return child->getAttType();
	}	


	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		//cout << "[ExprTree line 620] return double \n";
		return make_pair("avg", make_shared <MyDB_DoubleAttType> ());
	}

	pair<bool, string> IsJoinPredicate(){
		return make_pair(false, "");
	}

	~AvgOp () {}
};

#endif
