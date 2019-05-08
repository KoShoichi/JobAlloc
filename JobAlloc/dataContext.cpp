#include "dataContext.h"
#include "file_util.h"
#include "string_util.h"
#include "string_array.h"
#include "arraylist.h"
#include "job.h"
#include "employee.h"
#include "applog.h"
#include "employeeConsumer.h"
#include "joballoc.h"

CDataContext* CDataContext::m_pContext = NULL;

CDataContext::CDataContext()
{
	m_employee = NULL;
	m_employeesdict    = new CDictionary;    
	m_jobdict          = new CDictionary;
	m_result           = new CDictionary;
	m_resultscene      = new CDictionary;
	m_employeeConsumer = new CEmployeeConsumer;
	m_totalcost=0.0;

};


CDataContext::~CDataContext()
{
	if (m_employee != NULL) {
		delete m_employee;
		m_employee = NULL;
	}
	delete m_employee;
	delete m_employeesdict;
	delete m_jobdict;
	delete m_result;
	delete m_resultscene;
	delete m_employeeConsumer;
};

//保存当前现场，拷贝m_result当前的信息到m_resultscene
void CDataContext::SaveScene(){
	BEGIN_DICT_FOREACH(m_result, _it, _key, _value);
	CArrayList* arrayList   = (CArrayList*)_value;
	CArrayList* resultArray = (CArrayList*)m_resultscene->GetObj(_key);
	if (resultArray!=NULL){
		resultArray->Clear();
		for (int i=0;i<arrayList->Size();++i){
			CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			CStringValue* pstr = (CStringValue*)resultArray->SetObj(-1,str,false);
			/*CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			pstr->s_value      = str->s_value;
			pstr->s_starttime  = str->s_starttime;
			pstr->s_endtime    = str->s_endtime;
			pstr->flag         = str->flag;*/
		}
	}else{
		resultArray = (CArrayList*)m_resultscene->SetObj(_key,new CArrayList);
		for (int i=0;i<arrayList->Size();++i){
			CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			CStringValue* pstr = (CStringValue*)resultArray->SetObj(-1,str,false);
			/*CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			pstr->s_value      = str->s_value;
			pstr->s_starttime  = str->s_starttime;
			pstr->s_endtime    = str->s_endtime;
			pstr->flag         = str->flag;*/
		}
	}
	END_DICT_FOREACH(m_result, _it);

}

//恢复现场，将m_resultscene复制到m_result中
void CDataContext::ResoreScene(){
	//m_result->Clear();//情况当前保存的现场信息
	BEGIN_DICT_FOREACH(m_resultscene, _it, _key, _value);
	CArrayList* arrayList   = (CArrayList*)_value;
	CArrayList* resultArray = (CArrayList*)m_result->GetObj(_key);
	if (resultArray!=NULL){
		resultArray->Clear();
		for (int i=0;i<arrayList->Size();++i){
			CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			CStringValue* pstr = (CStringValue*)resultArray->SetObj(-1,str,false);
			/*CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			pstr->s_value      = str->s_value;
			pstr->s_starttime  = str->s_starttime;
			pstr->s_endtime    = str->s_endtime;
			pstr->flag         = str->flag;*/
	    }
	}else{
		resultArray = (CArrayList*)m_result->SetObj(_key,new CArrayList);
		for (int i=0;i<arrayList->Size();++i){
			CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			CStringValue* pstr = (CStringValue*)resultArray->SetObj(-1,str,false);
			/*CStringValue* str  = (CStringValue*)arrayList->GetObj(i);
			pstr->s_value      = str->s_value;
			pstr->s_starttime  = str->s_starttime;
			pstr->s_endtime    = str->s_endtime;
			pstr->flag         = str->flag;*/
	    }
	}
	END_DICT_FOREACH(m_resultscene, _it);
}

bool CDataContext::SwitchJob(int Empi,int jobi, int Empj,int jobj,int n){

	if (Empi>=m_result->Size()||Empj>=m_result->Size()){
		return false;
	}
  
	//获取job
	CStringValue*  jobinfoi =GetJob(Empi,jobi);
	if (jobinfoi==NULL){
		return false;
	}
	/*CStringValue*  jobnewi = new CStringValue;
	jobnewi->flag		 = jobinfoi->flag;
	jobnewi->s_value	 = jobinfoi->s_value;
	jobnewi->s_starttime = jobinfoi->s_starttime;
	jobnewi->s_endtime   = jobinfoi->s_endtime;*/
	DeleteJob(Empi,jobi);				//复制后删除当前行员工信息

	for (int k=0;k<n;k++){ 
		CStringValue*  jobinfoj =GetJob(Empj,jobj+k);
		if (jobinfoj==NULL){
			break;						//最后一行不满N个插入N个
		}
		/*CStringValue*  jobnewj = new CStringValue;
		jobnewj->flag		 = jobinfoi->flag;
		jobnewj->s_value	 = jobinfoi->s_value;
		jobnewj->s_starttime = jobinfoi->s_starttime;
		jobnewj->s_endtime   = jobinfoi->s_endtime;*/
		InsertJob(Empi,jobinfoj);       //插入到当前行信息中
	}

	for (int k=0;k<n;k++){
		if (NULL!=GetJob(Empj,jobj))
		{
			DeleteJob(Empj,jobj);  //删除多换行的值
		}else{
			break;
		}
	}
	InsertJob(Empj,jobinfoi);		 //插入到多行中去
	return true;
    //这里完成交换
}

//n交换几次
bool CDataContext::RandomSwitchJob(int n){
	for (int k=0;k<n;k++)
	{
		srand((unsigned)time(NULL));
		int empi =rand()%m_result->Size(); //随机i员工
		int empj =rand()%m_result->Size(); //随机j员工
		CArrayList* arrayListi = GetJobList(empi);
		CArrayList* arrayListj = GetJobList(empj);
		int jobi =0;
		if (arrayListi->Size()>0){
			jobi = rand()%arrayListi->Size(); //随机i员工随机jobi
		}
		int jobj =0;
		if (arrayListj->Size()>0){
			 jobj = rand()%arrayListj->Size(); //随机j员工随机jobj
		}
		SwitchJob(empi,jobi,empj,jobj);
	}
	return true;
    //这里完成交换
}

bool CDataContext::DeleteJob(int i,int j){

	bool result = false;
	int selCount=0;
	if (i>=m_result->Size()){
		return false;
	}
	BEGIN_DICT_FOREACH(m_result, _it, _key, _value);
	CArrayList* arrayList   = (CArrayList*)_value;
	if (i==selCount){
		if(arrayList->Delete(j)<0){
			result= false;
			break;
		}
		result= true;
		break;
	}
	selCount++;
	END_DICT_FOREACH(m_result, _it);

	return result;
}

bool CDataContext::InsertJob(int i,CStringValue* jobinfo){
	bool result = false;
	int selCount=0;
	if (i>=m_result->Size()){
		return false;
	}
	BEGIN_DICT_FOREACH(m_result, _it, _key, _value);
	CArrayList* arrayList   = (CArrayList*)_value;
	if (i==selCount){
		CStringValue* pstr= (CStringValue*)arrayList->SetObj(-1,jobinfo,false);
		/*pstr->s_value      = jobinfo->s_value;
		pstr->s_starttime  = jobinfo->s_starttime;
		pstr->s_endtime    = jobinfo->s_endtime;
		pstr->flag = jobinfo->flag+1;		        //被标记过当前job是被标记过的*/
		arrayList->Sort(sortTime);			        //对时间进行排序从小到大排序
		result= true;
	}
	selCount++;
	END_DICT_FOREACH(m_result, _it);
	return result;
}

CStringValue* CDataContext::GetJob(int i,int j){

	CStringValue *ResultValue=NULL;
	int selCount=0;
	if (i>=m_result->Size()){
		return ResultValue;
	}
	BEGIN_DICT_FOREACH(m_result, _it, _key, _value);
	  CArrayList* arrayList   = (CArrayList*)_value;
	  if (i==selCount){
		ResultValue  = (CStringValue*)arrayList->GetObj(j);
	    break;
	  }
	  selCount++;
	END_DICT_FOREACH(m_result, _it);
	return ResultValue;
}


CArrayList* CDataContext::GetJobList(int i){

	CArrayList *ResultValue=NULL;
	int selCount=0;
	if (i>=m_result->Size()){
		return ResultValue;
	}
	BEGIN_DICT_FOREACH(m_result, _it, _key, _value);
	CArrayList* arrayList   = (CArrayList*)_value;
	if (i==selCount){
	  ResultValue= arrayList;
	}
	selCount++;
	END_DICT_FOREACH(m_result, _it);
	return ResultValue;
}

int CDataContext::LoadEmployee(){
	if (FileUtil::browse_init("employees", "*.dat") !=0 ) {
		GLOG_MSG("读取employees目录错误");
		return -1;
	}
	char file_name[MAX_PATH] = {0};    
	while(FileUtil::browse_next(file_name) >= 0) {
		char szEmployeeID[20] = {0};
		FileUtil::parse_file_name(file_name, NULL, szEmployeeID, NULL);

		FILE *fp = fopen(file_name, "r");
		if (fp == NULL) {
			GLOG_MSG("员工文件读取失败[%s]", file_name);
			continue;
		}

		CEmployee *pEmp  = (CEmployee*)m_employeesdict->SetObj(szEmployeeID,new CEmployee);
		pEmp->employeeId  = szEmployeeID;

		char buf[2048] = {0};
		fgets(buf, sizeof(buf), fp);
		pEmp->b          =atof(buf);
		fgets(buf, sizeof(buf), fp);
		pEmp->penalty    =atof(buf);
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		CStringArray *pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue=(CValue*)pEmp->m_pEmployCost->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1));
			delete pJobArray2;
		}
		delete pJobArray;
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue =(CValue*)pEmp->m_pEmployLoad->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1)); //工作负荷
			delete pJobArray2;
		}
		delete pJobArray;
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue =(CValue*)pEmp->m_pEmployMoveCost->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1));//移动cost
			delete pJobArray2;
		}
		delete pJobArray;
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue =(CValue*)pEmp->m_pEmployMoveLoad->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1));//移动负荷
			delete pJobArray2;
		}
		delete pJobArray;
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue=(CValue*)pEmp->m_pEmployMoveTime->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1));//移动时间
			delete pJobArray2;
		}
	}
	FileUtil::browse_end();
	return 0;
}


//加载工作信息表
int CDataContext::LoadJobs(){
	if (FileUtil::browse_init("jobs", "*.dat") !=0 ) {
		GLOG_MSG("读取jobs目录错误");
		return -1;
	}
	char file_name[MAX_PATH] = {0};    
	while(FileUtil::browse_next(file_name) >= 0) {
		char szJobID[20] = {0};
		FileUtil::parse_file_name(file_name, NULL, szJobID, NULL);

		FILE *fp = fopen(file_name, "r");
		if (fp == NULL) {
			GLOG_MSG("工作文件读取失败[%s]", file_name);
			continue;
		}

		CJob *pjob  = (CJob*)m_jobdict->SetObj(szJobID,new CJob);
		pjob->jobid = szJobID;

		char buf[20] = {0};
		fgets(buf, sizeof(buf), fp);
		pjob->jobstarttime = buf;
		fgets(buf, sizeof(buf), fp);
		pjob->jobendtime   = buf;
	}
	FileUtil::browse_end();
	return 0;
}


//初始化上下文数据
int CDataContext::Initialize(){

	m_employee = new CProducer_NPNC(1);
	m_employee->RegisterNewConsumer(EmployeeConsumer, this);

	//初始化员工信息表
	LoadEmployee();

	//初始化工作信息表
	LoadJobs();

	//初始化工作信息，发送所有工作的初始化消息到employee中处理
	BEGIN_DICT_FOREACH(m_jobdict, _it, _key, _value);
	CJob* tmpjob = (CJob*)_value;
	m_employee->Publish(tmpjob,PDATA_TYPE_INIT,free_object_data);	        //第一步初始化当前解
	END_DICT_FOREACH(m_jobdict, _it);

	m_employee->Publish(NULL,PDATA_TYPE_INITEND,NULL);	        //第一步初始化当前解
	
	//如果没有分配到在结果集中插入为空结果
	BEGIN_DICT_FOREACH(m_employeesdict, _it, _key, _value);
    CArrayList* resultArray = (CArrayList*)m_result->GetObj(_key);
	if (resultArray==NULL){
		 resultArray = (CArrayList*)m_result->SetObj(_key,new CArrayList);
	}
	END_DICT_FOREACH(m_employeesdict, _it);

    //从第一个员工开始Relocation
	CProcess* mProcess = new CProcess;
	mProcess->employee =0;
	mProcess->job      =0;
	mProcess->insertEmployee =1;
	//处理第一个员工的第一个job将他插入到第二个员工中去
	m_employee->Publish(mProcess,PDATA_TYPE_RELOCATION,NULL);   //第二步Relocation
	return 0;
}


CDataContext* CDataContext::GetInstance()
{
	if (m_pContext == NULL) {
		m_pContext= new CDataContext();
	}
	return m_pContext;
};
