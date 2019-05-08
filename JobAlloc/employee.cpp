#include "employee.h"

CEmployee::CEmployee(){
	m_pEmployCost  =new CDictionary;
	m_pEmployLoad  =new CDictionary;
	m_pEmployMoveCost = new CDictionary;
	m_pEmployMoveLoad = new CDictionary;
	m_pEmployMoveTime = new CDictionary;
}

CEmployee::~CEmployee(){
	delete m_pEmployCost;
	delete m_pEmployLoad;
	delete m_pEmployMoveCost;
	delete m_pEmployMoveLoad;
	delete m_pEmployMoveTime;
}

bool sortTime(CGObject *i, CGObject *j){
	CStringValue* t1 = (CStringValue*)i;
	CStringValue* t2 = (CStringValue*)j;
	if (atof(t2->s_starttime.c_str())>atof(t1->s_starttime.c_str())){
	//if (t2->s_starttime>t1->s_starttime){
		return true;
	}else{
		return false;
	}
}