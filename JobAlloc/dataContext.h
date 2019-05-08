
/************************************************************************/
/* 处理排序数据的上下文myj												*/
/************************************************************************/
#include "employeeConsumer.h"
#include "TaskConsumer.h"
#include "pattern_nPnC.h"

#define EMP_HANDLER_TYPE			    0x101

#define PDATA_TYPE_INIT                     0x801   //初始化当前解的集合
#define PDATA_TYPE_RELOCATION               0x802   //对当前解进行RELOCATION
#define PDATA_TYPE_OTOSWITCH                0x803   //对当前解进行1对1交换
#define PDATA_TYPE_OTNSWITCH                0x804   //对当前解进行1对多交换
#define PDATA_TYPE_RELOCATIONNEW            0x805   //对当前解进行RELOCATIONNEW
#define PDATA_TYPE_INITEND                  0x806   //初始化当前解的集合结束
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

	//获取第i个员工第j个job的信息
   CStringValue*  GetJob(int i,int j);
   CArrayList* CDataContext::GetJobList(int i);
   //删除第i个员工第j个job的信息
   bool DeleteJob(int i,int j);

   //将jobinfo插入到第i个员工中去,这里会对插入进行重新排序
   bool InsertJob(int i,CStringValue* jobinfo);

   //将empi的jobi和empj和jobj进行交换
   bool SwitchJob(int Empi,int jobi, int Empj,int jobj,int n=1);

   //将empi的jobi和empj和jobj进行交换
   bool RandomSwitchJob(int n);

public:
	CProducer_NPNC *m_employee;
	CTaskConsumer  *m_employeeConsumer;
	CDictionary    *m_employeesdict;
	CDictionary    *m_jobdict;

public:
	CDictionary     *m_result;			//保存最后的结果 员工ID:j01->j02->j03 value cost
	CDictionary     *m_resultscene;		//保存当前现场
	double          m_totalcost;		//cost目标函数
private:
	CDataContext();
	static CDataContext* m_pContext;
};