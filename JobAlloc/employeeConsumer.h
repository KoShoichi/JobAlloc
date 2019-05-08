#pragma once
#ifndef _CEMPLOYEE_CONSUMER_H
#define _CEMPLOYEE_CONSUMER_H
#include "TaskConsumer.h"
#include "TaskHandler.h"
#include "job.h"
int EmployeeConsumer(int _type, void *_data, void *_handler, void *_context);

class CEmployeeHandler;
class CEmployeeConsumer : public CTaskConsumer
{
public:
	CEmployeeConsumer();
	virtual ~CEmployeeConsumer();
	virtual int Execute(int ret_code, int _type, void *_data, CTaskHandler *_handler, void *_context);

public:
	CEmployeeHandler *m_pEmployeeHandler;
};

class CStringValue;
//��ߴ���Ա����Ӧ�Ĺ�����Ϣ
class CEmployeeHandler : public CTaskHandler
{
public:
	CEmployeeHandler();
	virtual ~CEmployeeHandler();

	int OnStrategyDataInit(void* data);     //�������ݳ�ʼ��
	int OnStrategyDataInitEnd(void* data);  //�������ݳ�ʼ������
	int OnStrategyRelocation(void* data);   //��������Relocation
	int OnStrategyOTOSwitch(void* data);    //��������1��1����
	int OnStrategyOTNSwitch(void* data);    //��������1�Զཻ��
	int OnStrategyRelocationNew(void* data); //1��1������������RelocationNew

	void PostDrawMessage(char* title,int X,int Y,bool last=false);
	double OnReCalcCost();														 //�������е���cost
	double OnEvaluate(const char employeeid[]);										 //���ۺ���
	double OnPenalty(const char employeeid[],CStringValue* job1,CStringValue* job2);					 //�ص����ֵ�Penalty
protected:
	bool show;

public:
	long startTime;

};
#endif