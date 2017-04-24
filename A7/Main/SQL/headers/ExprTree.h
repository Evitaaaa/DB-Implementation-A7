
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

// this class encapsules a parsed SQL expression (such as "this.that > 34.5 AND 4 = 5")

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:
	virtual string toString () = 0;
	virtual pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName) = 0;
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
	}

	string toString () {
		return "[" + attName + "]";
	}

	MyDB_ExprType getType() {
		return MyDB_ExprType::identifierExpr;
	}	

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName) override {
		string type;
        string attr_name = tbName + "." + attName + ".type";
		
		MyDB_AttTypePtr attType;
		if(catalog -> getString(attr_name, type)){
			if(type.compare("bool") == 0){
				attType = make_shared <MyDB_BoolAttType> ();
			}
			else if(type.compare("string") == 0){
				attType = make_shared <MyDB_StringAttType> ();
			}
			else if(type.compare("int") == 0){
				attType = make_shared <MyDB_IntAttType> ();
			}
			else if(type.compare("double") == 0){
				attType = make_shared <MyDB_DoubleAttType> ();
			}
		}
		return make_pair("[" + attName + "]", attType);
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
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

	pair<string, MyDB_AttTypePtr> getAttPair (MyDB_CatalogPtr catalog, string tbName){
		return make_pair("", make_shared <MyDB_BoolAttType> ());
	}

	~AvgOp () {}
};

#endif
