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
	//��ȡ����Ա������������������Ա�����ݷ�����Ϣ
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
	//��consumer������
	return CTaskConsumer::Execute(ret_code, _type, _data, _handler, _context);
}

//Ա����Ĵ����࣬��չ������Ը��ݲ�ͬ�������ò�ͬ��Handler
CEmployeeHandler::CEmployeeHandler() : CTaskHandler(EMP_HANDLER_TYPE)
{
    show= false;
	startTime=0;
}

CEmployeeHandler::~CEmployeeHandler()
{

}
/***��ʼ���⣬���͹�����ʱ��������Ϣ����
 *  ʹ��̰���㷨��ֱ�Ӱ�����С����cost���з��䣬����˵һ��ʼ���乤��1ʱ��Ա��3�е��������cost��С���Ͱѹ���1�����Ա��3��
 */
int CEmployeeHandler::OnStrategyDataInit(void * data){
	CDataContext *pDataContext = CDataContext::GetInstance();
	CJob* jobInfo = (CJob*)data;
	char   mincostEmp[25] = {0};
	double mincost=INVALID_DOUBLE;
	//���乤����cost��С��
	BEGIN_DICT_FOREACH(pDataContext->m_employeesdict, _it, _key, _value);
	CEmployee* tmpemp = (CEmployee*)_value;
	CValue* dvalue=(CValue*)tmpemp->m_pEmployCost->GetObj(jobInfo->jobid.c_str());
	if (dvalue==NULL){
		continue;
	}
	if ( dvalue->d_value<mincost){
		mincost=dvalue->d_value;
		strcpy(mincostEmp,tmpemp->employeeId.c_str()); //����cost��С��Ա��ID
	}
	END_DICT_FOREACH(pDataContext->m_employeesdict, _it);

	//����Сcost�Ĺ��������mincostemp�Ľ��
	CArrayList* resultArray = (CArrayList*)pDataContext->m_result->GetObj(mincostEmp);
	if (resultArray==NULL){  
	    resultArray = (CArrayList*)pDataContext->m_result->SetObj(mincostEmp,new CArrayList);
	    CStringValue* pstr= (CStringValue*)resultArray->SetObj(-1,new CStringValue);
		pstr->s_value      = jobInfo->jobid;
		pstr->s_starttime  = jobInfo->jobstarttime;
		pstr->s_endtime    = jobInfo->jobendtime;
		pstr->flag         = 0;		  //δ����ǹ�
		resultArray->Sort(sortTime);  //��ʱ����������С��������
	}else{
		CStringValue* pstr= (CStringValue*)resultArray->SetObj(-1,new CStringValue);
		pstr->s_value = jobInfo->jobid;
		pstr->s_starttime  = jobInfo->jobstarttime;
		pstr->s_endtime    = jobInfo->jobendtime;
		pstr->flag         = 0; //δ����ǹ�
		resultArray->Sort(sortTime); //��ʱ����������С��������
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

	CProcess* mNewProcess = (CProcess*)data;  //��ȡ��ǰ��Process
	//��Ա���ӵ�һ��Ա���ĵ�һ������ʼ�����Ű����������뵽�����Ա�����Ű��У����Cost�и��ƾ͸��µ�ǰ�Ľ�
	CDataContext *pDataContext = CDataContext::GetInstance();

	//���ȱ����ֳ�����m_resultֵ
	pDataContext->SaveScene();
	//����Process����ǰ�Ĳ���,��ȡ��ǰ��Ҫ�����job��Ϣ���ӵ�һ��ȡjob��Ϣ
	CStringValue* jobvalue=pDataContext->GetJob(mNewProcess->employee,mNewProcess->job);
	if (jobvalue==NULL){         
		if (mNewProcess->employee==mNewProcess->SwitchTwo){//��ǰ������Ա���Ĺ�����Ϣ����ʱ�������һ��Ա��
			CProcess* mProcess = new CProcess;
			mProcess->employee =0;
			mProcess->job      =0;
			mProcess->SwitchEmployee =1;
			mProcess->SwitchJob=0;													       //��1��Ա���ĵ�һ��������2��Ա����һ����������
			mProcess->n        =mNewProcess->n;
			if (mNewProcess->n==1){
				pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //������1��1����
			}else{
				pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //������1��N����
			}
			delete mNewProcess; //�ͷ��ڴ�
			mNewProcess=NULL;
			return 0; 
		}else{
			mNewProcess->insertEmployee =mNewProcess->employee;							   //���뵽��һ����
			mNewProcess->employee = mNewProcess->SwitchTwo;								   //ȥȡ��һ��Ա������Ϣ
			mNewProcess->job =0;														   //��һ��������ʼ	
			pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATIONNEW,NULL);  //�ڶ���Relocation
			return 0;    
		}
	}

	//���뵽ָ����Ա����
	pDataContext->InsertJob(mNewProcess->insertEmployee,jobvalue);

	//ɾ��֮ǰ��Ա����Ϣ
	pDataContext->DeleteJob(mNewProcess->employee,mNewProcess->job);
	PostDrawMessage("RELOCATIONNEW:",200,30); 
	//���㵱ǰ�Ƿ�cost��С,���С�ڵ�ǰcost�͸���,����ָ��ֳ�
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){
		pDataContext->m_totalcost = recalcost; //�������������ڿ�ʼȡ��ǰemployee����һ��job������,�����ɾ���˵�ǰjob��ʱjob��index����
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //�ڶ���Relocation
		return 0;
	}else{
		pDataContext->ResoreScene();
		mNewProcess->job++;																//��ȡ��һ����������
		PostDrawMessage("RELOCATIONNEW:",200,30); 
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //�ڶ���Relocation
		return 0;
	}
	return 0;
}


//Relocation
int CEmployeeHandler::OnStrategyRelocation(void* data){

	CProcess* mNewProcess = (CProcess*)data;  //��ȡ��ǰ��Process
	//��Ա���ӵ�һ��Ա���ĵ�һ������ʼ�����Ű����������뵽�����Ա�����Ű��У����Cost�и��ƾ͸��µ�ǰ�Ľ�
	CDataContext *pDataContext = CDataContext::GetInstance();

	//���ȱ����ֳ�����m_resultֵ
	pDataContext->SaveScene();
	//����Process����ǰ�Ĳ���,����2��
	CStringValue*jobvalue= pDataContext->GetJob(mNewProcess->employee,mNewProcess->job);
	if (jobvalue==NULL){         
		if (mNewProcess->employee>=pDataContext->m_result->Size()){//��ǰ������Ա���Ĺ�����Ϣ����ʱ�������һ��Ա��
			delete mNewProcess; //�ͷ��ڴ�
			mNewProcess = NULL;
			CProcess* mProcess = new CProcess;
			mProcess->employee =0;
			mProcess->job      =0;
			mProcess->n        =1;
			mProcess->SwitchEmployee =1;
			mProcess->SwitchJob=0;													   //��1��Ա���ĵ�һ��������2��Ա����һ����������
			PostDrawMessage("RELOCATION END:",200,30,true);
			pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //������1��1����
			return 0; 
		}else{
			mNewProcess->employee++;       //ȥȡ��һ��Ա������Ϣ
			mNewProcess->job =0;           //��һ��������ʼ	
			mNewProcess->insertEmployee =0;//Ĭ�ϴ�ͷ��ʼȡֵ
			pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //�ڶ���Relocation
			return 0;    
		}
	}else{
		//�����ʱjobvalue���Ѿ���ǵĴ��·���ʼȡ
		if (jobvalue->flag>=1){
			if (mNewProcess->insertEmployee==0){
				mNewProcess->insertEmployee=mNewProcess->employee+1;
				if (mNewProcess->insertEmployee>=pDataContext->m_result->Size()){
					mNewProcess->insertEmployee=0;  //��ͷ��ʼ��һ��
					mNewProcess->job++;	            //���￪ʼ��һ����Ҫ���¿�ʼBUG�޸���
					pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //�ڶ���Relocation
					return 0;
				}
			}

		}
	}

	//����Ǳ��������ȥ������һ��
	if (mNewProcess->employee==mNewProcess->insertEmployee){
		mNewProcess->insertEmployee++;
		if (mNewProcess->insertEmployee>=pDataContext->m_result->Size()){
			mNewProcess->job++;				//��ȡ��һ����������
			mNewProcess->insertEmployee=0;  //��ͷ��ʼ��һ��
		}
		//ָ����ǰ�µ�Process,��������PDATA_TYPE_RELOCATION����
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //�ڶ���Relocation
		return 0;   
	}

    //���뵽ָ����Ա����
	pDataContext->InsertJob(mNewProcess->insertEmployee,jobvalue);

	//ɾ��֮ǰ��Ա����Ϣ
	pDataContext->DeleteJob(mNewProcess->employee,mNewProcess->job);
	PostDrawMessage("RELOCATION:",200,30);
	//���㵱ǰ�Ƿ�cost��С,���С�ڵ�ǰcost�͸���,����ָ��ֳ�
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){
		pDataContext->m_totalcost = recalcost; //������������
		//�ڿ�ʼȡ��ǰemployee����һ��job������,�����ɾ���˵�ǰjob��ʱjob��index����
		mNewProcess->insertEmployee=0;		   //��ͷ��ʼ��һ��
		//ָ����ǰ�µ�Process,��������PDATA_TYPE_RELOCATION����
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //�ڶ���Relocation
		return 0;
	}else{	
		pDataContext->ResoreScene();
		mNewProcess->insertEmployee++;                                 //���뵽��һ��employee�г���
		//���ȫ��Ա�����������ˣ���ô����
		if (mNewProcess->insertEmployee>=pDataContext->m_result->Size()){
			mNewProcess->job++;				//��ȡ��һ����������
			mNewProcess->insertEmployee=0;  //��ͷ��ʼ��һ��
		}
		//ָ����ǰ�µ�Process,��������PDATA_TYPE_RELOCATION����
		PostDrawMessage("RELOCATION:",200,30);
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_RELOCATION,NULL);   //�ڶ���Relocation
		return 0;
	}
	return 0;
}

int CEmployeeHandler::OnStrategyOTOSwitch(void* data){
	//���������RelocationNew
	CProcess* mNewProcess = (CProcess*)data;  //��ȡ��ǰ��Process
	//��Ա���ӵ�һ��Ա���ĵ�һ������ʼ�����Ű����������뵽�����Ա�����Ű��У����Cost�и��ƾ͸��µ�ǰ�Ľ�
	CDataContext *pDataContext = CDataContext::GetInstance();

	//���ȱ����ֳ�����m_resultֵ
	pDataContext->SaveScene();
	//����Process����ǰ�Ĳ���,����2��Ա����Ϣ����ʧ��
	if(!pDataContext->SwitchJob(mNewProcess->employee,mNewProcess->job,mNewProcess->SwitchEmployee,mNewProcess->SwitchJob)){
		//�������û�й�����ô�������employee
		if (mNewProcess->job==0){
			mNewProcess->employee++;
			if (mNewProcess->employee>=pDataContext->m_result->Size()){
				delete mNewProcess; //�ͷ��ڴ�
				mNewProcess =NULL;
				PostDrawMessage("OTOSWITCH END:",200,30,true);
				CProcess* mProcess = new CProcess;
				mProcess->employee =0;
				mProcess->job      =0;
				mProcess->SwitchEmployee =1;
				mProcess->n =2;															   //���Ƚ���1��2����
				mProcess->SwitchJob=0;													   //��1��Ա���ĵ�һ��������2��Ա����һ����������
				pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //���Ĳ�1��N����
				return 0; 
			}else{
				mNewProcess->job =0;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;				           //����һ��empid���н���
				mNewProcess->SwitchJob =0;
				pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //������1��1����
				return 0;
			}
		}else{
           PostDrawMessage("OTOSWITCH Error:",200,30,true);
		   return 0;
		}
		
	}else{
          PostDrawMessage("OTOSWITCH:",200,30);
	}
	//���㵱ǰ�Ƿ�cost��С,���С�ڵ�ǰcost�͸���,����ָ��ֳ�
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){
		pDataContext->m_totalcost = recalcost; 
		//����������õ����ƽ���RELOCATIONNEW����
		CProcess* mProcess = new CProcess;
		mProcess->employee =mNewProcess->employee;
		mProcess->job      =0;
		mProcess->insertEmployee = mNewProcess->SwitchEmployee;
		mProcess->SwitchTwo=mNewProcess->SwitchEmployee;
		mProcess->n       = mNewProcess->n;
		PostDrawMessage("OTOSWITCH:",200,30);
		delete mNewProcess; //�ͷ��ڴ�
		mNewProcess =NULL;
		pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //�ڶ�����RelocationNew
		return 0;
	}else{
		pDataContext->ResoreScene();
		mNewProcess->SwitchJob++;													 //��SwitchEmp����һ��job���н���
		if (pDataContext->GetJob(mNewProcess->SwitchEmployee,mNewProcess->SwitchJob)==NULL){
			mNewProcess->SwitchEmployee++;
			mNewProcess->SwitchJob =0;
			if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){        //�Ѿ����������һ��
				mNewProcess->job++;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;
				mNewProcess->SwitchJob =0;
				if (pDataContext->GetJob(mNewProcess->employee,mNewProcess->job)==NULL){ //����һ��empid
					mNewProcess->employee++;
					mNewProcess->job =0;
					mNewProcess->SwitchEmployee=mNewProcess->employee+1;				 //����һ��empid���н���
					mNewProcess->SwitchJob =0;
					if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){
						delete mNewProcess; //�ͷ��ڴ�
						mNewProcess =NULL;
						PostDrawMessage("OTOSWITCH END:",200,30,true);
						CProcess* mProcess = new CProcess;
						mProcess->employee =0;
						mProcess->job      =0;
						mProcess->SwitchEmployee =1;
						mProcess->n =2;															   //���Ƚ���1��2����
						mProcess->SwitchJob=0;													   //��1��Ա���ĵ�һ��������2��Ա����һ����������
						pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //���Ĳ�1��N����
						return 0; 
					}
				}
			}
		}
		PostDrawMessage("OTOSWITCH:",200,30);
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTOSWITCH,NULL);	   //������1��1����
		return 0;
	}
	return 0;
}

int CEmployeeHandler::OnStrategyOTNSwitch(void* data){
	char buf[255]={0};
		//���������RelocationNew
	CProcess* mNewProcess = (CProcess*)data;  //��ȡ��ǰ��Process
	//��Ա���ӵ�һ��Ա���ĵ�һ������ʼ�����Ű����������뵽�����Ա�����Ű��У����Cost�и��ƾ͸��µ�ǰ�Ľ�
	CDataContext *pDataContext = CDataContext::GetInstance();

	if (mNewProcess->n>=4){
		delete mNewProcess; //�ͷ��ڴ�
		mNewProcess =NULL;
		PostDrawMessage("OTNSWITCH END:",200,30,true); //ֱ��1��3�Ľ���
	
		//����3600�����Ѱ��
		if (startTime==0){
			TimeUtil::GetTimeOfDay(&startTime);
		}else{
			long endtime;
			TimeUtil::GetTimeOfDay(&endtime);
			if ((endtime-startTime)>3600){
				return 0;
			}
		}
		//������������,������ִ���㷨ֱ��涨ʱ�����
		pDataContext->RandomSwitchJob(2);
		CProcess* mProcess = new CProcess;
		mProcess->employee =0;
		mProcess->job      =0;
		mProcess->insertEmployee =1;
		//�����һ��Ա���ĵ�һ��job�������뵽�ڶ���Ա����ȥ
		pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_RELOCATION,NULL);   //�ڶ���Relocation
		return 0;
	}

	//���ȱ����ֳ�����m_resultֵ
	pDataContext->SaveScene();
	//����Process����ǰ�Ĳ���,����2��Ա����Ϣ����ʧ��
	if(!pDataContext->SwitchJob(mNewProcess->employee,mNewProcess->job,mNewProcess->SwitchEmployee,mNewProcess->SwitchJob,mNewProcess->n)){
		if (mNewProcess->job==0){
			mNewProcess->employee++;
			if (mNewProcess->employee>=pDataContext->m_result->Size()){
				mNewProcess->n++;                 //�����ĸ�����1
				sprintf(buf,"OT%dSWITCH",mNewProcess->n);
				PostDrawMessage(buf,200,30);
				mNewProcess->employee =0;
				mNewProcess->job      =0;
				mNewProcess->SwitchEmployee =1;
				mNewProcess->SwitchJob=0;	
				pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //���Ĳ�1��N����
				return 0; 
			}else{
				mNewProcess->job =0;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;				           //����һ��empid���н���
				mNewProcess->SwitchJob =0;
				pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //���Ĳ�1��N����
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
	//���㵱ǰ�Ƿ�cost��С,���С�ڵ�ǰcost�͸���,����ָ��ֳ�
	double recalcost = OnReCalcCost();
	if(recalcost<pDataContext->m_totalcost){

		pDataContext->m_totalcost = recalcost; 
		//����������õ����ƽ���RELOCATIONNEW����
		CProcess* mProcess = new CProcess;
		mProcess->employee =mNewProcess->employee;
		mProcess->job      =0;
		mProcess->insertEmployee = mNewProcess->SwitchEmployee;
		mProcess->SwitchTwo=mNewProcess->SwitchEmployee;
		mProcess->n   = mNewProcess->n;
		sprintf(buf,"OT%dSWITCH",mNewProcess->n);
		PostDrawMessage(buf,200,30);
		delete mNewProcess; //�ͷ��ڴ�
		mNewProcess =NULL;
		pDataContext->m_employee->Publish(mProcess,PDATA_TYPE_RELOCATIONNEW,NULL);   //�ڶ�����RelocationNew
		return 0;
	}else{
		pDataContext->ResoreScene();
		mNewProcess->SwitchJob++;													 //��SwitchEmp����һ��job���н���
		if (pDataContext->GetJob(mNewProcess->SwitchEmployee,mNewProcess->SwitchJob)==NULL){
			mNewProcess->SwitchEmployee++;
			mNewProcess->SwitchJob =0;
			if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){        //�Ѿ����������һ��
				mNewProcess->job++;
				mNewProcess->SwitchEmployee=mNewProcess->employee+1;
				mNewProcess->SwitchJob =0;
				if (pDataContext->GetJob(mNewProcess->employee,mNewProcess->job)==NULL){ //����һ��empid
					mNewProcess->employee++;
					mNewProcess->job =0;
					mNewProcess->SwitchEmployee=mNewProcess->employee+1;				 //����һ��empid���н���
					mNewProcess->SwitchJob =0;
					if (mNewProcess->SwitchEmployee>=pDataContext->m_result->Size()){
						sprintf(buf,"OT%dSWITCH END",mNewProcess->n);
						PostDrawMessage(buf,200,30,true);
						mNewProcess->n++;    //�����ĸ�����1
						mNewProcess->employee =0;
						mNewProcess->job      =0;
						mNewProcess->SwitchEmployee =1;
						mNewProcess->SwitchJob=0;	
						pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //���Ĳ�1��N����
						return 0; 
					}
				}
			}
		}
		sprintf(buf,"OT%dSWITCH",mNewProcess->n);
		PostDrawMessage(buf,200,30);
		pDataContext->m_employee->Publish(mNewProcess,PDATA_TYPE_OTNSWITCH,NULL);	   //���Ĳ�1��N����
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

//���ۺ��� = Ŀ�꺯��������ģ� + �ص����ֵ�penalty
double CEmployeeHandler::OnEvaluate(const char employeeid[]){
    CDataContext *pDataContext = CDataContext::GetInstance();
	//cost = c_14 + c_16 + c_11 + cm_146 + cm_161 + c_28 + c_23 + c_22 + cm_283 + cm_232 + c_35 + c_37 + cm_357 + (l_14 + l_16 + l_11 + lm_146 + lm_161 �C b_1)p_1 + (l_28 + l_23 + l_22 + lm_283 + lm_232 �C b_2)p_2 + (l_35 + l_37 + lm_357 �C b_3)p_3
	double totalcost =0.0;
	double jobcost   =0.0;
	double jobload   =0.0;
	double movecost  =0.0;
	double moveload  =0.0;
	double penalty   =0.0;
	//����м����ܵ�cost
	//��ȡԱ����Ϣ��KeyδԱ����ID
	CEmployee* pEmp  = (CEmployee*)pDataContext->m_employeesdict->GetObj(employeeid); //��ȡ��ǰԱ����Ϣ
	if (pEmp==NULL){
		return 0;//�����ڸ�Ա����Ϣ
	}
	//double p = pEmp->penalty; //Ա����Penaltyϵ��
	CArrayList* jobList = (CArrayList*)pDataContext->m_result->GetObj(employeeid);
	for (int i=0;i<jobList->Size();++i){
		CStringValue* str =(CStringValue*)jobList->GetObj(i);
		CStringValue* job1 =  str;

		CValue* d_value = (CValue*)pEmp->m_pEmployCost->GetObj(str->s_value.c_str()); //��ȡjob��cost
		CValue* d_value3 = (CValue*)pEmp->m_pEmployLoad->GetObj(str->s_value.c_str());
		jobcost+=d_value->d_value;                                                   //c_14 + c_16 + c_11 ��Ա��1�����й���cost���ƶ�cost���ܺ�
		jobload+=d_value3->d_value;													 //��������
		if (i<jobList->Size()-1){
			CStringValue* str2 =(CStringValue*)jobList->GetObj(i+1);
			CStringValue* job2 = str2;
			string strjob = str->s_value+"->"+ str2->s_value;//��job1�ƶ���job2
			CValue* d_value2 = (CValue*)pEmp->m_pEmployMoveCost->GetObj(strjob.c_str());
			if (d_value2!=NULL){
				movecost+=d_value2->d_value;	   //cm_146 + cm_161
			}
			CValue* d_value4 = (CValue*)pEmp->m_pEmployMoveLoad->GetObj(strjob.c_str());
			if (d_value4!=NULL)
			{
				moveload+=d_value4->d_value;											 //�ƶ�����
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

//����employeeid����job1�ƶ���job2�Ƿ��й����ص�
double CEmployeeHandler::OnPenalty(const char employeeid[],CStringValue* job1,CStringValue* job2){
	CDataContext *pDataContext = CDataContext::GetInstance();

	CEmployee* pEmp  = (CEmployee*)pDataContext->m_employeesdict->GetObj(employeeid); //��ȡ��ǰԱ����Ϣ
	if (pEmp==NULL){
		return 0; //�����ڵ�ǰԱ����Ϣ
	}
	//�����ص����ֵ�penalty
	string j12= job1->s_value+"->"+job2->s_value; //��job1�ƶ���job2
	CValue* dvalue=(CValue*)pEmp->m_pEmployMoveTime->GetObj(j12.c_str());
	double tm_ijj1 =3;//Ա��employeeid�ӹ���j�ƶ�������j1ʱ���ƶ�ʱ��
	if (dvalue!=NULL){
		tm_ijj1 = dvalue->d_value; //��ǰԱ�����ƶ�ʱ��
	}
	//j�Ľ���ʱ��+�ƶ���j1��ʱ��-j1�Ŀ�ʼʱ����ǹ����ص�
	double penalty = atof(job1->s_endtime.c_str())+tm_ijj1-atof(job2->s_starttime.c_str());
	if(penalty<=0){
		return 0;
	}else{ 
		return penalty*5000;
	}
	//return penalty;
}
