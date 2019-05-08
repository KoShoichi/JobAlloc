#pragma once
#ifndef _CEMPLOYEE_H
#define _CEMPLOYEE_H
#include "dictionary.h"
#include "object.h"
#include <string>
using namespace std;
class CEmployee: public CGObject{
	public:
		CEmployee();
		~CEmployee();
	public:
		string  employeeId;			//员工ID编号
		double b;					//员工工作最大负荷;
		double penalty;				//员工的工作负荷超过部分的penalty系数
		
		CDictionary *m_pEmployCost;		//员工承担工作JOBID时的工作cost
		CDictionary *m_pEmployLoad;		//员工承担工作JOBID时的工作负荷
		CDictionary *m_pEmployMoveCost; //员工从工作job1移动到工作job2时的移动cost,标识序号为jobid1->jobid2
		CDictionary *m_pEmployMoveLoad; //员工从工作job1移动到工作job2时的移动负荷,标识序号为jobid1->jobid2
		CDictionary *m_pEmployMoveTime; //员工从工作job1移动到工作job2时的移动时间,标识序号为jobid1->jobid2

};

class CValue:public CGObject{
	public:
		double d_value;
};

class CStringValue:public CGObject{
	public:
		string s_value;

	public:
		string s_starttime;
		string s_endtime;

    public:
		int flag;

};

class CProcess{
	public:
		int employee;
		int job;
		int insertEmployee;
		int n;

	public:
		int SwitchTwo;      //交换的行

	public:
		int SwitchEmployee; //作为交换的员工
		int SwitchJob;      //作为交互的工作
};

bool sortTime(CGObject *i, CGObject *j);
#endif