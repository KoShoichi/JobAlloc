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
		string  employeeId;			//Ա��ID���
		double b;					//Ա��������󸺺�;
		double penalty;				//Ա���Ĺ������ɳ������ֵ�penaltyϵ��
		
		CDictionary *m_pEmployCost;		//Ա���е�����JOBIDʱ�Ĺ���cost
		CDictionary *m_pEmployLoad;		//Ա���е�����JOBIDʱ�Ĺ�������
		CDictionary *m_pEmployMoveCost; //Ա���ӹ���job1�ƶ�������job2ʱ���ƶ�cost,��ʶ���Ϊjobid1->jobid2
		CDictionary *m_pEmployMoveLoad; //Ա���ӹ���job1�ƶ�������job2ʱ���ƶ�����,��ʶ���Ϊjobid1->jobid2
		CDictionary *m_pEmployMoveTime; //Ա���ӹ���job1�ƶ�������job2ʱ���ƶ�ʱ��,��ʶ���Ϊjobid1->jobid2

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
		int SwitchTwo;      //��������

	public:
		int SwitchEmployee; //��Ϊ������Ա��
		int SwitchJob;      //��Ϊ�����Ĺ���
};

bool sortTime(CGObject *i, CGObject *j);
#endif