#pragma once
#ifndef _CJOB_H
#define _CJOB_H
#include "object.h"
#include <string>
using namespace std;
class CJob:public CGObject{
    public: 
		CJob();
		~CJob();

	public:
		string jobid;		  //��ǰjob��ID���
		string jobstarttime;  //��ǰjob�Ľ�������
		string jobendtime;    //��ǰjob�Ľ�������
};


#endif