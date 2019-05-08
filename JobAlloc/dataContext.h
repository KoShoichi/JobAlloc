
/************************************************************************/
/* �����������ݵ�������myj												*/
/************************************************************************/
#include "employeeConsumer.h"
#include "TaskConsumer.h"
#include "pattern_nPnC.h"

#define EMP_HANDLER_TYPE			    0x101

#define PDATA_TYPE_INIT                     0x801   //��ʼ����ǰ��ļ���
#define PDATA_TYPE_RELOCATION               0x802   //�Ե�ǰ�����RELOCATION
#define PDATA_TYPE_OTOSWITCH                0x803   //�Ե�ǰ�����1��1����
#define PDATA_TYPE_OTNSWITCH                0x804   //�Ե�ǰ�����1�Զཻ��
#define PDATA_TYPE_RELOCATIONNEW            0x805   //�Ե�ǰ�����RELOCATIONNEW
#define PDATA_TYPE_INITEND                  0x806   //��ʼ����ǰ��ļ��Ͻ���
class CTaskConsumer;
class CStringValue;
class CArrayList;
class CDataContext {
public:
	~CDataContext();

	static CDataContext* GetInstance();
	int  Initialize();
	int  LoadEmployee();
	int  LoadJobs();
	void SaveScene();
	void ResoreScene();

	//��ȡ��i��Ա����j��job����Ϣ
   CStringValue*  GetJob(int i,int j);
   CArrayList* CDataContext::GetJobList(int i);
   //ɾ����i��Ա����j��job����Ϣ
   bool DeleteJob(int i,int j);

   //��jobinfo���뵽��i��Ա����ȥ,�����Բ��������������
   bool InsertJob(int i,CStringValue* jobinfo);

   //��empi��jobi��empj��jobj���н���
   bool SwitchJob(int Empi,int jobi, int Empj,int jobj,int n=1);

   //��empi��jobi��empj��jobj���н���
   bool RandomSwitchJob(int n);

public:
	CProducer_NPNC *m_employee;
	CTaskConsumer  *m_employeeConsumer;
	CDictionary    *m_employeesdict;
	CDictionary    *m_jobdict;

public:
	CDictionary     *m_result;			//�������Ľ�� Ա��ID:j01->j02->j03 value cost
	CDictionary     *m_resultscene;		//���浱ǰ�ֳ�
	double          m_totalcost;		//costĿ�꺯��
private:
	CDataContext();
	static CDataContext* m_pContext;
};