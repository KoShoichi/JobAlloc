#include "employeeConsumer.h"
#include "dataContext.h"
#include "job.h"
#include "employee.h"
#include "arraylist.h"
#include "joballoc.h"
#include "time_util.h"
int EmployeeConsumer(int _type, void *_data, void *_handler, void *_context)
{
	CTaskHandler *pTaskHandler = (CTaskHandler *)_handler;
	CDataContext *pTradingContext = (CDataContext *)_context;
	pTradingContext->m_employeeConsumer->InsertHandler(pTaskHandler);
	pTradingContext->m_employeeConsumer->Execute(0, _type, _data, pTaskHandler, _context);

	return 0;
}

CEmployeeConsumer::CEmployeeConsumer()
{
	m_pEmployeeHandler = new CEmployeeHandler;
}

CEmployeeConsumer::~CEmployeeConsumer()
{
	delete m_pEmployeeHandler;
}


int CEmployeeConsumer::Execute(int ret_code, int _type, void *_data, CTaskHandler *_handler, void *_context)
{
	//获取处理员工处理句柄，用来处理员工数据分配信息
	CEmployeeHandler *pHandler = (CEmployeeHandler *)GetHandler(EMP_HANDLER_TYPE, _handler);
	if (pHandler == NULL)
		pHandler = m_pEmployeeHandler;
	switch(_type)
	{
		case PDATA_TYPE_INIT:
			ret_code = pHandler->OnStrategyDataInit(_data);
			break;
		case PDATA_TYPE_INITEND:
			ret_code = pHandler->OnStrategyDataInitEnd(_data);
			break;
		case PDATA_TYPE_RELOCATION:
			ret_code = pHandler->OnStrategyRelocation(_data);
			break;
		case PDATA_TYPE_RELOCATIONNEW:
			ret_code = pHandler->OnStrategyRelocationNew(_data);
			break;
		case PDATA_TYPE_OTOSWITCH:
			ret_code = pHandler->OnStrategyOTOSwitch(_data);
			break;
		case PDATA_TYPE_OTNSWITCH:
			ret_code = pHandler->OnStrategyOTNSwitch(_data);
			break;
	}
	//本consumer不处理
	return CTaskConsumer::Execute(ret_code, _type, _data, _handler, _context);
}

//员工类的处理类，扩展情况可以根据不同需求设置不同的Handler
CEmployeeHandler::CEmployeeHandler() : CTaskHandler(EMP_HANDLER_TYPE)
{
    show= false;
	startTime=0;
}

CEmployeeHandler::~CEmployeeHandler()
{

}
/***初始化解，推送过来的时工作的信息内容
 *  使用贪心算法，直接按照最小工作cost进行分配，比如说一开始分配工作1时，员工3承担这项工作的cost最小，就把工作1分配给员工3。
 */
int CEmployeeHandler::OnStrategyDataInit(void * data){
	CDataContext *pDataContext = CDataContext::GetInstance();
	CJob* jobInfo = (CJob*)data;
	char   mincostEmp[25] = {0};
	double mincost=INVALID_DOUBLE;
	//分配工作给cost最小的
	BEGIN_DICT_FOREACH(pDataContext->m_employeesdict, _it, _key, _value);
	CEmployee* tmpemp = (CEmployee*)_value;
	CValue* dvalue=(CValue*)tmpemp->m_pEmployCost->GetObj(jobInfo->jobid.c_str());
	if (dvalue==NULL){
		continue;
	}
	if ( dvalue->d_value<mincost){
		mincost=dvalue->d_value;
		strcpy(mincostEmp,tmpemp->employeeId.c_str()); //查找cost最小的员工ID
	}
	END_DICT_FOREACH(pDataContext->m_employeesdict, _it);

	//将最小cost的工作分配给mincostemp的结果
	CArrayList* resultArray = (CArrayList*)pDataContext->m_result->GetObj(mincostEmp);
	if (resultArray==NULL){  
	    resultArray = (CArrayList*)pDataContext->m_result->SetObj(mincostEmp,new CArrayList);
	    CStringValue* pstr= (CStringValue*)resultArray->SetObj(-1,new CStringValue);
		pstr->s_value      = jobInfo->jobid;
		pstr->s_starttime  = jobInfo->jobstarttime;
		pstr->s_endtime    = jobInfo->jobendtime;
		pstr->flag         = 0;		  //未被标记过
		resultArray->Sort(sortTime);  //对时间进行排序从小到大排序
	}else{
		CStringValue* pstr= (CStringValue*)resultArray->SetObj(-1,new CStringValue);
		pstr->s_value = jobInfo->jobid;
		pstr->s_starttime  = jobInfo->jobstarttime;
		pstr->s_endtime    = jobInfo->jobendtime;
		pstr->flag         = 0; //未被标记过
		resultArray->Sort(sortTime); //对时间进行排序从小到大排序
	}
	pDataContext->m_totalcost = OnReCalcCost();
	PostDrawMessage("INITIALIZATION:",200,30);
	return 0;
}

int  CEmployeeHandler::OnStrategyDataInitEnd(void * data){
	PostDrawMessage("INITIALIZATION END:",200,30,true);
	return 0;
}

void CEmployeeHandler::PostDrawMessage(char* title,int X,int Y,bool last){
	if (show==true)
	{
		SetSzName(title);
		SetXVal(X);
		SetYVal(Y);
		if (GetWndHand()!=NULL){
			PostMessage(GetWndHand(),WM_UPDATESCREEN,0,0);
		}
		Sleep(5);
	}else{
		if (last==true)
		{
			SetSzName(title);
			SetXVal(X);
			SetYVal(Y);
			if (GetWndHand()!=NULL){
				PostMessage(GetWndHand(),WM_UPDATESCREEN,0,0);
			}
			Sleep(5);
		}
	}
}

//RelocationNew
int CEmployeeHandler::OnStrategyRelocationNew(void* data){

	CProcess* mNewProcess = (CProcess*)data;  //获取当前的Process
	//将员工从第一个员工的第一个任务开始，试着把这个任务插入到下面的员工的排班中，如果Cost有改善就更新当前的解
	CDataContext *pDataContext = CDataContext::GetInstance();

	//首先保存现场保存m_result值
	pDataContext->SaveScene();
	//根据Process处理当前的步骤,获取当前需要处理的job信息，从第一行取job信息
	CStringValue* jobvalue=pDataContext->GetJob(mNewProcess->employee,mNewProcess->job);
	if (jobvalue==NULL){         
		if (mNewProcess->employee==mNewProcess->SwitchTwo){//当前不存在员工的工作信息，此时到了最后一个员工
			CProcess* mProcess = new CProcess;
			mProcess->employee =0;
			mProcess->job      =0;
			mProcess->SwitchEmployee =1;
			mProcess->SwitchJob=0;													       //将1号员工的第一个工作和2号员工第一个工作交换
			mProcess->n        =mNewProcess->n;
			if (mNewProcess->n==1){
				pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //第三步1对1交换
			}else{
				pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //第三步1对N交换
			}
			delete mNewProcess; //释放内存
			mNewProcess=NULL;
			return 0; 
		}else{
			mNewProcess->insertEmployee =mNewProcess->employee;							   //插入到另一行中
			mNewProcess->employee = mNewProcess->SwitchTwo;								   //去取下一个员工的信息
			mNewProcess->job =0;														   //第一个工作开始	
			pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATIONNEW,NULL);  //第二步Relocation
			return 0;    
		}
	}

	//插入到指定的员工中
	pDataContext->InsertJob(mNewProcess->insertEmployee,jobvalue);

	//删除之前的员工信息
	pDataContext->DeleteJob(mNewProcess->employee,mNewProcess->job);
	PostDrawMessage("RELOCATIONNEW:",200,30); 
	//计算当前是否cost最小,如果小于当前cost就更新,否则恢复现场
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){
		pDataContext->m_totalcost = recalcost; //更新最新排列在开始取当前employee的下一个job做处理,如果是删除了当前job此时job的index不变
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //第二步Relocation
		return 0;
	}else{
		pDataContext->ResoreScene();
		mNewProcess->job++;																//获取下一个工作处理
		PostDrawMessage("RELOCATIONNEW:",200,30); 
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //第二步Relocation
		return 0;
	}
	return 0;
}


//Relocation
int CEmployeeHandler::OnStrategyRelocation(void* data){

	CProcess* mNewProcess = (CProcess*)data;  //获取当前的Process
	//将员工从第一个员工的第一个任务开始，试着把这个任务插入到下面的员工的排班中，如果Cost有改善就更新当前的解
	CDataContext *pDataContext = CDataContext::GetInstance();

	//首先保存现场保存m_result值
	pDataContext->SaveScene();
	//根据Process处理当前的步骤,交互2个
	CStringValue*jobvalue= pDataContext->GetJob(mNewProcess->employee,mNewProcess->job);
	if (jobvalue==NULL){         
		if (mNewProcess->employee>=pDataContext->m_result->Size()){//当前不存在员工的工作信息，此时到了最后一个员工
			delete mNewProcess; //释放内存
			mNewProcess = NULL;
			CProcess* mProcess = new CProcess;
			mProcess->employee =0;
			mProcess->job      =0;
			mProcess->n        =1;
			mProcess->SwitchEmployee =1;
			mProcess->SwitchJob=0;													   //将1号员工的第一个工作和2号员工第一个工作交换
			PostDrawMessage("RELOCATION END:",200,30,true);
			pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //第三步1对1交换
			return 0; 
		}else{
			mNewProcess->employee++;       //去取下一个员工的信息
			mNewProcess->job =0;           //第一个工作开始	
			mNewProcess->insertEmployee =0;//默认从头开始取值
			pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //第二步Relocation
			return 0;    
		}
	}else{
		//如果此时jobvalue是已经标记的从下方开始取
		if (jobvalue->flag>=1){
			if (mNewProcess->insertEmployee==0){
				mNewProcess->insertEmployee=mNewProcess->employee+1;
				if (mNewProcess->insertEmployee>=pDataContext->m_result->Size()){
					mNewProcess->insertEmployee=0;  //从头开始下一个
					mNewProcess->job++;	            //这里开始下一个需要重新开始BUG修复点
					pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //第二步Relocation
					return 0;
				}
			}

		}
	}

	//如果是本身就跳过去插入下一条
	if (mNewProcess->employee==mNewProcess->insertEmployee){
		mNewProcess->insertEmployee++;
		if (mNewProcess->insertEmployee>=pDataContext->m_result->Size()){
			mNewProcess->job++;				//获取下一个工作处理
			mNewProcess->insertEmployee=0;  //从头开始下一个
		}
		//指定当前新的Process,继续进行PDATA_TYPE_RELOCATION操作
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //第二步Relocation
		return 0;   
	}

    //插入到指定的员工中
	pDataContext->InsertJob(mNewProcess->insertEmployee,jobvalue);

	//删除之前的员工信息
	pDataContext->DeleteJob(mNewProcess->employee,mNewProcess->job);
	PostDrawMessage("RELOCATION:",200,30);
	//计算当前是否cost最小,如果小于当前cost就更新,否则恢复现场
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){
		pDataContext->m_totalcost = recalcost; //更新最新排列
		//在开始取当前employee的下一个job做处理,如果是删除了当前job此时job的index不变
		mNewProcess->insertEmployee=0;		   //从头开始下一个
		//指定当前新的Process,继续进行PDATA_TYPE_RELOCATION操作
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //第二步Relocation
		return 0;
	}else{	
		pDataContext->ResoreScene();
		mNewProcess->insertEmployee++;                                 //插入到下一个employee中尝试
		//如果全部员工都尝试完了，那么返回
		if (mNewProcess->insertEmployee>=pDataContext->m_result->Size()){
			mNewProcess->job++;				//获取下一个工作处理
			mNewProcess->insertEmployee=0;  //从头开始下一个
		}
		//指定当前新的Process,继续进行PDATA_TYPE_RELOCATION操作
		PostDrawMessage("RELOCATION:",200,30);
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //第二步Relocation
		return 0;
	}
	return 0;
}

int CEmployeeHandler::OnStrategyOTOSwitch(void* data){
	//交换后进行RelocationNew
	CProcess* mNewProcess = (CProcess*)data;  //获取当前的Process
	//将员工从第一个员工的第一个任务开始，试着把这个任务插入到下面的员工的排班中，如果Cost有改善就更新当前的解
	CDataContext *pDataContext = CDataContext::GetInstance();

	//首先保存现场保存m_result值
	pDataContext->SaveScene();
	//根据Process处理当前的步骤,交互2个员工信息交换失败
	if(!pDataContext->SwitchJob(mNewProcess->employee,mNewProcess->job,mNewProcess->SwitchEmployee,mNewProcess->SwitchJob)){
		//如果该行没有工作那么跳过这个employee
		if (mNewProcess->job==0){
			mNewProcess->employee++;
			if (mNewProcess->employee>=pDataContext->m_result->Size()){
				delete mNewProcess; //释放内存
				mNewProcess =NULL;
				PostDrawMessage("OTOSWITCH END:",200,30,true);
				CProcess* mProcess = new CProcess;
				mProcess->employee =0;
				mProcess->job      =0;
				mProcess->SwitchEmployee =1;
				mProcess->n =2;															   //首先进行1对2交换
				mProcess->SwitchJob=0;													   //将1号员工的第一个工作和2号员工第一个工作交换
				pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //第四步1对N交换
				return 0; 
			}else{
				mNewProcess->job =0;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;				           //从下一个empid进行交换
				mNewProcess->SwitchJob =0;
				pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //第三步1对1交换
				return 0;
			}
		}else{
           PostDrawMessage("OTOSWITCH Error:",200,30,true);
		   return 0;
		}
		
	}else{
          PostDrawMessage("OTOSWITCH:",200,30);
	}
	//计算当前是否cost最小,如果小于当前cost就更新,否则恢复现场
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){
		pDataContext->m_totalcost = recalcost; 
		//交换后如果得到改善进行RELOCATIONNEW操作
		CProcess* mProcess = new CProcess;
		mProcess->employee =mNewProcess->employee;
		mProcess->job      =0;
		mProcess->insertEmployee = mNewProcess->SwitchEmployee;
		mProcess->SwitchTwo=mNewProcess->SwitchEmployee;
		mProcess->n       = mNewProcess->n;
		PostDrawMessage("OTOSWITCH:",200,30);
		delete mNewProcess; //释放内存
		mNewProcess =NULL;
		pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //第二步的RelocationNew
		return 0;
	}else{
		pDataContext->ResoreScene();
		mNewProcess->SwitchJob++;													 //和SwitchEmp的下一个job进行交换
		if (pDataContext->GetJob(mNewProcess->SwitchEmployee,mNewProcess->SwitchJob)==NULL){
			mNewProcess->SwitchEmployee++;
			mNewProcess->SwitchJob =0;
			if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){        //已经交换到最后一个
				mNewProcess->job++;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;
				mNewProcess->SwitchJob =0;
				if (pDataContext->GetJob(mNewProcess->employee,mNewProcess->job)==NULL){ //到下一个empid
					mNewProcess->employee++;
					mNewProcess->job =0;
					mNewProcess->SwitchEmployee=mNewProcess->employee+1;				 //从下一个empid进行交换
					mNewProcess->SwitchJob =0;
					if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){
						delete mNewProcess; //释放内存
						mNewProcess =NULL;
						PostDrawMessage("OTOSWITCH END:",200,30,true);
						CProcess* mProcess = new CProcess;
						mProcess->employee =0;
						mProcess->job      =0;
						mProcess->SwitchEmployee =1;
						mProcess->n =2;															   //首先进行1对2交换
						mProcess->SwitchJob=0;													   //将1号员工的第一个工作和2号员工第一个工作交换
						pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //第四步1对N交换
						return 0; 
					}
				}
			}
		}
		PostDrawMessage("OTOSWITCH:",200,30);
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //第三步1对1交换
		return 0;
	}
	return 0;
}

int CEmployeeHandler::OnStrategyOTNSwitch(void* data){
	char buf[255]={0};
		//交换后进行RelocationNew
	CProcess* mNewProcess = (CProcess*)data;  //获取当前的Process
	//将员工从第一个员工的第一个任务开始，试着把这个任务插入到下面的员工的排班中，如果Cost有改善就更新当前的解
	CDataContext *pDataContext = CDataContext::GetInstance();

	if (mNewProcess->n>=4){
		delete mNewProcess; //释放内存
		mNewProcess =NULL;
		PostDrawMessage("OTNSWITCH END:",200,30,true); //直到1对3的交换
	
		//大于3600秒结束寻找
		if (startTime==0){
			TimeUtil::GetTimeOfDay(&startTime);
		}else{
			long endtime;
			TimeUtil::GetTimeOfDay(&endtime);
			if ((endtime-startTime)>3600){
				return 0;
			}
		}
		//结束后进行随机,在重新执行算法直达规定时间完成
		pDataContext->RandomSwitchJob(2);
		CProcess* mProcess = new CProcess;
		mProcess->employee =0;
		mProcess->job      =0;
		mProcess->insertEmployee =1;
		//处理第一个员工的第一个job将他插入到第二个员工中去
		pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_RELOCATION,NULL);   //第二步Relocation
		return 0;
	}

	//首先保存现场保存m_result值
	pDataContext->SaveScene();
	//根据Process处理当前的步骤,交互2个员工信息交换失败
	if(!pDataContext->SwitchJob(mNewProcess->employee,mNewProcess->job,mNewProcess->SwitchEmployee,mNewProcess->SwitchJob,mNewProcess->n)){
		if (mNewProcess->job==0){
			mNewProcess->employee++;
			if (mNewProcess->employee>=pDataContext->m_result->Size()){
				mNewProcess->n++;                 //交换的个数加1
				sprintf(buf,"OT%dSWITCH",mNewProcess->n);
				PostDrawMessage(buf,200,30);
				mNewProcess->employee =0;
				mNewProcess->job      =0;
				mNewProcess->SwitchEmployee =1;
				mNewProcess->SwitchJob=0;	
				pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //第四步1对N交换
				return 0; 
			}else{
				mNewProcess->job =0;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;				           //从下一个empid进行交换
				mNewProcess->SwitchJob =0;
				pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //第四步1对N交换
				return 0; 
			}
		}else{
			sprintf(buf,"OT%dSWITCH Error",mNewProcess->n);
			PostDrawMessage(buf,200,30,true);
			return 0;
		}
	
	}else{
		sprintf(buf,"OT%dSWITCH",mNewProcess->n);
		PostDrawMessage(buf,200,30);
	}
	//计算当前是否cost最小,如果小于当前cost就更新,否则恢复现场
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){

		pDataContext->m_totalcost = recalcost; 
		//交换后如果得到改善进行RELOCATIONNEW操作
		CProcess* mProcess = new CProcess;
		mProcess->employee =mNewProcess->employee;
		mProcess->job      =0;
		mProcess->insertEmployee = mNewProcess->SwitchEmployee;
		mProcess->SwitchTwo=mNewProcess->SwitchEmployee;
		mProcess->n   = mNewProcess->n;
		sprintf(buf,"OT%dSWITCH",mNewProcess->n);
		PostDrawMessage(buf,200,30);
		delete mNewProcess; //释放内存
		mNewProcess =NULL;
		pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //第二步的RelocationNew
		return 0;
	}else{
		pDataContext->ResoreScene();
		mNewProcess->SwitchJob++;													 //和SwitchEmp的下一个job进行交换
		if (pDataContext->GetJob(mNewProcess->SwitchEmployee,mNewProcess->SwitchJob)==NULL){
			mNewProcess->SwitchEmployee++;
			mNewProcess->SwitchJob =0;
			if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){        //已经交换到最后一个
				mNewProcess->job++;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;
				mNewProcess->SwitchJob =0;
				if (pDataContext->GetJob(mNewProcess->employee,mNewProcess->job)==NULL){ //到下一个empid
					mNewProcess->employee++;
					mNewProcess->job =0;
					mNewProcess->SwitchEmployee=mNewProcess->employee+1;				 //从下一个empid进行交换
					mNewProcess->SwitchJob =0;
					if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){
						sprintf(buf,"OT%dSWITCH END",mNewProcess->n);
						PostDrawMessage(buf,200,30,true);
						mNewProcess->n++;    //交换的个数加1
						mNewProcess->employee =0;
						mNewProcess->job      =0;
						mNewProcess->SwitchEmployee =1;
						mNewProcess->SwitchJob=0;	
						pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //第四步1对N交换
						return 0; 
					}
				}
			}
		}
		sprintf(buf,"OT%dSWITCH",mNewProcess->n);
		PostDrawMessage(buf,200,30);
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //第四步1对N交换
		return 0;
	}
	return 0;
}

double CEmployeeHandler::OnReCalcCost(){
	CDataContext *pDataContext = CDataContext::GetInstance();
	double totalcoust = 0.0;
	BEGIN_DICT_FOREACH(pDataContext->m_result, _it, _key, _value);
	totalcoust+=OnEvaluate(_key);
	END_DICT_FOREACH(pDataContext->m_result, _it);
	return totalcoust;
}

//评价函数 = 目标函数（上面的） + 重叠部分的penalty
double CEmployeeHandler::OnEvaluate(const char employeeid[]){
    CDataContext *pDataContext = CDataContext::GetInstance();
	//cost = c_14 + c_16 + c_11 + cm_146 + cm_161 + c_28 + c_23 + c_22 + cm_283 + cm_232 + c_35 + c_37 + cm_357 + (l_14 + l_16 + l_11 + lm_146 + lm_161 – b_1)p_1 + (l_28 + l_23 + l_22 + lm_283 + lm_232 – b_2)p_2 + (l_35 + l_37 + lm_357 – b_3)p_3
	double totalcost =0.0;
	double jobcost   =0.0;
	double jobload   =0.0;
	double movecost  =0.0;
	double moveload  =0.0;
	double penalty   =0.0;
	//结果中计算总的cost
	//获取员工信息，Key未员工的ID
	CEmployee* pEmp  = (CEmployee*)pDataContext->m_employeesdict->GetObj(employeeid); //获取当前员工信息
	if (pEmp==NULL){
		return 0;//不存在该员工信息
	}
	//double p = pEmp->penalty; //员工的Penalty系数
	CArrayList* jobList = (CArrayList*)pDataContext->m_result->GetObj(employeeid);
	for (int i=0;i<jobList->Size();++i){
		CStringValue* str =(CStringValue*)jobList->GetObj(i);
		CStringValue* job1 =  str;

		CValue* d_value = (CValue*)pEmp->m_pEmployCost->GetObj(str->s_value.c_str()); //获取job的cost
		CValue* d_value3 = (CValue*)pEmp->m_pEmployLoad->GetObj(str->s_value.c_str());
		jobcost+=d_value->d_value;                                                   //c_14 + c_16 + c_11 ：员工1的所有工作cost和移动cost的总和
		jobload+=d_value3->d_value;													 //工作负荷
		if (i<jobList->Size()-1){
			CStringValue* str2 =(CStringValue*)jobList->GetObj(i+1);
			CStringValue* job2 = str2;
			string strjob = str->s_value+"->"+ str2->s_value;//从job1移动到job2
			CValue* d_value2 = (CValue*)pEmp->m_pEmployMoveCost->GetObj(strjob.c_str());
			if (d_value2!=NULL){
				movecost+=d_value2->d_value;	   //cm_146 + cm_161
			}
			CValue* d_value4 = (CValue*)pEmp->m_pEmployMoveLoad->GetObj(strjob.c_str());
			if (d_value4!=NULL)
			{
				moveload+=d_value4->d_value;											 //移动负荷
			}
			penalty += OnPenalty(employeeid,job1,job2);
		}
	}
	double loadPenalty =(jobload+moveload-pEmp->b)*pEmp->penalty;
	if (loadPenalty<=0)
	{
		loadPenalty=0;
	}
	totalcost = jobcost+movecost+loadPenalty+penalty;
	return totalcost;
}

//计算employeeid，从job1移动到job2是否有工作重叠
double CEmployeeHandler::OnPenalty(const char employeeid[],CStringValue* job1,CStringValue* job2){
	CDataContext *pDataContext = CDataContext::GetInstance();

	CEmployee* pEmp  = (CEmployee*)pDataContext->m_employeesdict->GetObj(employeeid); //获取当前员工信息
	if (pEmp==NULL){
		return 0; //不存在当前员工信息
	}
	//计算重叠部分的penalty
	string j12= job1->s_value+"->"+job2->s_value; //从job1移动到job2
	CValue* dvalue=(CValue*)pEmp->m_pEmployMoveTime->GetObj(j12.c_str());
	double tm_ijj1 =3;//员工employeeid从工作j移动到工作j1时的移动时间
	if (dvalue!=NULL){
		tm_ijj1 = dvalue->d_value; //当前员工的移动时间
	}
	//j的结束时间+移动到j1的时间-j1的开始时间就是工作重叠
	double penalty = atof(job1->s_endtime.c_str())+tm_ijj1-atof(job2->s_starttime.c_str());
	if(penalty<=0){
		return 0;
	}else{ 
		return penalty*5000;
	}
	//return penalty;
}
