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

//���浱ǰ�ֳ�������m_result��ǰ����Ϣ��m_resultscene
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

//�ָ��ֳ�����m_resultscene���Ƶ�m_result��
void CDataContext::ResoreScene(){
	//m_result->Clear();//�����ǰ������ֳ���Ϣ
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
  
	//��ȡjob
	CStringValue*  jobinfoi =GetJob(Empi,jobi);
	if (jobinfoi==NULL){
		return false;
	}
	/*CStringValue*  jobnewi = new CStringValue;
	jobnewi->flag		 = jobinfoi->flag;
	jobnewi->s_value	 = jobinfoi->s_value;
	jobnewi->s_starttime = jobinfoi->s_starttime;
	jobnewi->s_endtime   = jobinfoi->s_endtime;*/
	DeleteJob(Empi,jobi);				//���ƺ�ɾ����ǰ��Ա����Ϣ

	for (int k=0;k<n;k++){ 
		CStringValue*  jobinfoj =GetJob(Empj,jobj+k);
		if (jobinfoj==NULL){
			break;						//���һ�в���N������N��
		}
		/*CStringValue*  jobnewj = new CStringValue;
		jobnewj->flag		 = jobinfoi->flag;
		jobnewj->s_value	 = jobinfoi->s_value;
		jobnewj->s_starttime = jobinfoi->s_starttime;
		jobnewj->s_endtime   = jobinfoi->s_endtime;*/
		InsertJob(Empi,jobinfoj);       //���뵽��ǰ����Ϣ��
	}

	for (int k=0;k<n;k++){
		if (NULL!=GetJob(Empj,jobj))
		{
			DeleteJob(Empj,jobj);  //ɾ���໻�е�ֵ
		}else{
			break;
		}
	}
	InsertJob(Empj,jobinfoi);		 //���뵽������ȥ
	return true;
    //������ɽ���
}

//n��������
bool CDataContext::RandomSwitchJob(int n){
	for (int k=0;k<n;k++)
	{
		srand((unsigned)time(NULL));
		int empi =rand()%m_result->Size(); //���iԱ��
		int empj =rand()%m_result->Size(); //���jԱ��
		CArrayList* arrayListi = GetJobList(empi);
		CArrayList* arrayListj = GetJobList(empj);
		int jobi =0;
		if (arrayListi->Size()>0){
			jobi = rand()%arrayListi->Size(); //���iԱ�����jobi
		}
		int jobj =0;
		if (arrayListj->Size()>0){
			 jobj = rand()%arrayListj->Size(); //���jԱ�����jobj
		}
		SwitchJob(empi,jobi,empj,jobj);
	}
	return true;
    //������ɽ���
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
		pstr->flag = jobinfo->flag+1;		        //����ǹ���ǰjob�Ǳ���ǹ���*/
		arrayList->Sort(sortTime);			        //��ʱ����������С��������
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
		GLOG_MSG("��ȡemployeesĿ¼����");
		return -1;
	}
	char file_name[MAX_PATH] = {0};    
	while(FileUtil::browse_next(file_name) >= 0) {
		char szEmployeeID[20] = {0};
		FileUtil::parse_file_name(file_name, NULL, szEmployeeID, NULL);

		FILE *fp = fopen(file_name, "r");
		if (fp == NULL) {
			GLOG_MSG("Ա���ļ���ȡʧ��[%s]", file_name);
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
			dvalue->d_value= atof(pJobArray2->at(1)); //��������
			delete pJobArray2;
		}
		delete pJobArray;
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue =(CValue*)pEmp->m_pEmployMoveCost->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1));//�ƶ�cost
			delete pJobArray2;
		}
		delete pJobArray;
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue =(CValue*)pEmp->m_pEmployMoveLoad->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1));//�ƶ�����
			delete pJobArray2;
		}
		delete pJobArray;
		fgets(buf, sizeof(buf), fp);
		StringUtil::trim(buf);
		pJobArray = StringUtil::split(buf, '|');
		for(int i=0;i<pJobArray->Size();i++){
			CStringArray *pJobArray2 = StringUtil::split(pJobArray->at(i), ',');
			CValue* dvalue=(CValue*)pEmp->m_pEmployMoveTime->SetObj(pJobArray2->at(0),new CValue);
			dvalue->d_value= atof(pJobArray2->at(1));//�ƶ�ʱ��
			delete pJobArray2;
		}
	}
	FileUtil::browse_end();
	return 0;
}


//���ع�����Ϣ��
int CDataContext::LoadJobs(){
	if (FileUtil::browse_init("jobs", "*.dat") !=0 ) {
		GLOG_MSG("��ȡjobsĿ¼����");
		return -1;
	}
	char file_name[MAX_PATH] = {0};    
	while(FileUtil::browse_next(file_name) >= 0) {
		char szJobID[20] = {0};
		FileUtil::parse_file_name(file_name, NULL, szJobID, NULL);

		FILE *fp = fopen(file_name, "r");
		if (fp == NULL) {
			GLOG_MSG("�����ļ���ȡʧ��[%s]", file_name);
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


//��ʼ������������
int CDataContext::Initialize(){

	m_employee = new CProducer_NPNC(1);
	m_employee->RegisterNewConsumer(EmployeeConsumer, this);

	//��ʼ��Ա����Ϣ��
	LoadEmployee();

	//��ʼ��������Ϣ��
	LoadJobs();

	//��ʼ��������Ϣ���������й����ĳ�ʼ����Ϣ��employee�д���
	BEGIN_DICT_FOREACH(m_jobdict, _it, _key, _value);
	CJob* tmpjob = (CJob*)_value;
	m_employee->Publish(tmpjob,PDATA_TYPE_INIT,free_object_data);	        //��һ����ʼ����ǰ��
	END_DICT_FOREACH(m_jobdict, _it);

	m_employee->Publish(NULL,PDATA_TYPE_INITEND,NULL);	        //��һ����ʼ����ǰ��
	
	//���û�з��䵽�ڽ�����в���Ϊ�ս��
	BEGIN_DICT_FOREACH(m_employeesdict, _it, _key, _value);
    CArrayList* resultArray = (CArrayList*)m_result->GetObj(_key);
	if (resultArray==NULL){
		 resultArray = (CArrayList*)m_result->SetObj(_key,new CArrayList);
	}
	END_DICT_FOREACH(m_employeesdict, _it);

    //�ӵ�һ��Ա����ʼRelocation
	CProcess* mProcess = new CProcess;
	mProcess->employee =0;
	mProcess->job      =0;
	mProcess->insertEmployee =1;
	//�����һ��Ա���ĵ�һ��job�������뵽�ڶ���Ա����ȥ
	m_employee->Publish(mProcess,PDATA_TYPE_RELOCATION,NULL);   //�ڶ���Relocation
	return 0;
}


CDataContext* CDataContext::GetInstance()
{
	if (m_pContext == NULL) {
		m_pContext= new CDataContext();
	}
	return m_pContext;
};
