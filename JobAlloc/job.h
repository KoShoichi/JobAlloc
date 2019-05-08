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
		string jobid;		  //当前job的ID编号
		string jobstarttime;  //当前job的结束日期
		string jobendtime;    //当前job的结束日期
};


#endif