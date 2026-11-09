#include "StdAfx.h"
#include "SymTable.h"

using namespace std;

CSymTable::CSymTable(int aTok, string aStr) : dVal(0), token(aTok), strlex(aStr)
{
}

CSymTable::~CSymTable()
{
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
