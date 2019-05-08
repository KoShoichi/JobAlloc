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
//这边处理员工对应的工作信息
class CEmployeeHandler : public CTaskHandler
{
public:
	CEmployeeHandler();
	virtual ~CEmployeeHandler();

	int OnStrategyDataInit(void* data);     //场景数据初始化
	int OnStrategyDataInitEnd(void* data);  //场景数据初始化结束
	int OnStrategyRelocation(void* data);   //场景数据Relocation
	int OnStrategyOTOSwitch(void* data);    //场景数据1对1交换
	int OnStrategyOTNSwitch(void* data);    //场景数据1对多交换
	int OnStrategyRelocationNew(void* data); //1对1交换场景数据RelocationNew

	void PostDrawMessage(char* title,int X,int Y,bool last=false);
	double OnReCalcCost();														 //计算所有的总cost
	double OnEvaluate(const char employeeid[]);										 //评价函数
	double OnPenalty(const char employeeid[],CStringValue* job1,CStringValue* job2);					 //重叠部分的Penalty
protected:
	bool show;

public:
	long startTime;

};
#endif