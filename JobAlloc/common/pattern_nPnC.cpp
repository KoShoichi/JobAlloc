#include "pattern_nPnC.h"
#include "linked_list.h"
#include "sequence.h"
#include "Thread.h"
#include "time_util.h"
#include "applog.h"

//�����ڴ�����߳�
static int GarbageCollectorInit(CThread *pThread);
static int GarbageCollectorRun(CThread *pThread);
static int GarbageCollectorFinish(CThread *pThread);
static int GarbageCollectorStop(CThread *pThread);
//����洢���ݽṹ
static void CLinkedListData_Free(void *list_node);
//�������߳�
static int ConsumerRun(CThread *pThread);
static int ConsumerStop(CThread *pThread);



//�����߽ӿڣ�Ŀǰ�����⿪�ţ��乹������������������������
class CConsumer_NPNC
{
public:
    //���й���Consumer����Producer�е���RegisterNewConsumerЧ����ͬ
    CConsumer_NPNC(CProducer_NPNC *pProducer, consumer_call_back_t _func, void *_callback_context);
    //ע���Producer��������ˣ���ôConsumer����֮һͬ����
    ~CConsumer_NPNC();

    //���ʣ������ݴ����ֹͣ������������
    int FinishAndStop();
    //�ȴ���ֹͣ����������
    void WaitStop();
    HANDLE_NPNC GetContextHandle();

private:
    HANDLE_NPNC m_context;
};


//����������
class CProdcerContext_NPNC
{
public:
    //CProdcerʵ����ʱһͬʵ����
    CProdcerContext_NPNC(unsigned int max_consumer=1);
    ~CProdcerContext_NPNC();

    CLinkedList m_List;                 //������������֮������ݶ���
    CConsumer_NPNC **m_pConsumerList;   //���е�������
    Sequence m_ConsumerIdx;             //ע�������������ߵ�����
    unsigned int m_MaxConsumer;         //����������������Ŀ
    CThread *m_GarbageCollector;        //�����ڴ�����߳�
    Sequence m_PublishIdx;
};


//����������
class CConsumerContext_NPNC
{
public:
    CConsumerContext_NPNC(CProdcerContext_NPNC *pProdcerContext, consumer_call_back_t _func, void *_callback_context);
    ~CConsumerContext_NPNC();

    CProdcerContext_NPNC *m_pProdcerContext;
    int m_index;
    bool m_running;
    CThread *m_thread;        //�����߹����߳�
    consumer_call_back_t m_FuncCallBack;
    void *m_CallBackContext;
    NODEHANDLE m_CurNode;   //��ȡ�������ݽڵ�
};


//�����ڴ�����߳�����
class CGarbageCollector_NPNC
{
public:
    //��������߳�ʱʵ����
    CGarbageCollector_NPNC(CProdcerContext_NPNC *pProdcerContext);
    //����������ʹ�ù��������ڴ�
    int Collect();

    CProdcerContext_NPNC *m_pProdcerContext;
    bool m_running;
};

//publishͬ�������źŽṹ
class CSyncHandler_NPNC
{
public:
    CSyncHandler_NPNC();
    ~CSyncHandler_NPNC();
    Sequence count_;
};


//����洢���ݽṹ
class CLinkedListData_NPNC
{
public:
    //���ݷ���ʱ�ж���
    CLinkedListData_NPNC(void *_data, int _type, free_node_func_t _func, void *_handler, int _count);
    ~CLinkedListData_NPNC();

    int type_;
    void *data_;
    void *pHandler_;
    free_node_func_t free_func_;
    Sequence count_;            //�������Ѽ�����
    Sequence publish_index_;    //���ڶ��̲߳���ʱ�˶�β����˳�����
    CSyncHandler_NPNC *pSyncHandler_;
};




/***************����������*********************/

//��CProducerʵ����ʱ��������ΪCProducer��Ա
CProdcerContext_NPNC::CProdcerContext_NPNC(unsigned int max_consumer/*=1*/)
{    
    if (max_consumer <= 0)
        m_MaxConsumer = 1;
    else if (max_consumer > 1000)
        m_MaxConsumer = 1000;
    else
        m_MaxConsumer = max_consumer;

    m_pConsumerList = (CConsumer_NPNC **)calloc(m_MaxConsumer, sizeof(CConsumer_NPNC *));

    //�������������߳�����
    m_GarbageCollector = new CThread;
    CGarbageCollector_NPNC *pGarbageCollector = new CGarbageCollector_NPNC(this);
    m_GarbageCollector->RegisterContext((void *)pGarbageCollector);
    m_GarbageCollector->RegisterInitHanlde(GarbageCollectorInit);
    m_GarbageCollector->RegisterRunHanlde(GarbageCollectorRun);
    m_GarbageCollector->RegisterFinishHanlde(GarbageCollectorFinish);
    m_GarbageCollector->RegisterStopHanlde(GarbageCollectorStop);
    m_GarbageCollector->Start();    //�߳��ڹ��캯����ֱ������������������ֹͣ
}

//��CProducer����ʱһͬ����
CProdcerContext_NPNC::~CProdcerContext_NPNC()
{
    m_GarbageCollector->Safe_Stop();
    m_GarbageCollector->Join();
    delete m_GarbageCollector;
    m_GarbageCollector = NULL;

    for (unsigned int i=0; i<m_MaxConsumer; ++i) {
        if (m_pConsumerList[i] != NULL) {
            delete m_pConsumerList[i];
            m_pConsumerList[i] = NULL;
        }
    }    
    free(m_pConsumerList);
    m_pConsumerList = NULL;

    m_List.Clear();
}


/***************����������*********************/

CConsumerContext_NPNC::CConsumerContext_NPNC(CProdcerContext_NPNC *pProdcerContext, consumer_call_back_t _func, void *_callback_context)
{
    m_pProdcerContext = pProdcerContext;
    m_FuncCallBack = _func;
    m_CallBackContext = _callback_context;
    m_running = true;
    
    m_index = -1;       //ͨ��Producer��ע�᷽�����������
    m_CurNode = NULL;

    //�����߳�����
    m_thread = new CThread;
    m_thread->RegisterContext((void *)this);
    m_thread->RegisterRunHanlde(ConsumerRun);
    m_thread->RegisterStopHanlde(ConsumerStop);
}

CConsumerContext_NPNC::~CConsumerContext_NPNC()
{
    delete m_thread;    //�߳�ֹͣ�ɵ���Stop������
}

static int ConsumerRun(CThread *pThread)
{
    CConsumerContext_NPNC *p = (CConsumerContext_NPNC *)pThread->GetContext();
    CLinkedListData_NPNC *pLastData = NULL;
    while(p->m_running) {
        CLinkedList *pList = &(p->m_pProdcerContext->m_List);        

        if (p->m_CurNode == NULL) {
            //�����̼߳���ʱ�����о�û���ݣ���ôѭ��������ͷ��ֱ��������Ϊֹ
            p->m_CurNode = pList->HeadNode();
            if (p->m_CurNode == NULL) {
                TimeUtil::Sleep(2);
                continue;
            }
            CLinkedListData_NPNC *pLinkedData = (CLinkedListData_NPNC *)pList->GetValue(p->m_CurNode);
            if (p->m_FuncCallBack != NULL)
                p->m_FuncCallBack(pLinkedData->type_, pLinkedData->data_, pLinkedData->pHandler_, p->m_CallBackContext);
            //GLOG_DEBUG("Consumer���� �ڵ�:0x%08x ��ַ:0x%08x ����ǰ������:%lld", p->m_CurNode, pLinkedData, pLinkedData->count_.Get());
            if (pLinkedData->pSyncHandler_ != NULL) {
                pLinkedData->pSyncHandler_->count_.AddAndGet(-1);
            }
            pLastData = pLinkedData;
        }

        //����Ϊһ��������ѭ��������m_running���ƣ��Ա���ɶ���ʣ�������
        if (p->m_CurNode != NULL) {
            while(1) {
                //m_CurNode��ΪNULLʱ�����е��������Ѿ���������ģ��ж��Ƿ����¸����ýڵ㼴��
                NODEHANDLE pNextNode = pList->NextNode(p->m_CurNode);                
                if (pNextNode == NULL) {                    
                    TimeUtil::Sleep(2);
                    break;
                }
                if (pLastData != NULL) {
                    //ֻ�е�ȡ����һ����Ч�ڵ�ʱ���ܽ���ǰ�Ľڵ��ͷţ�
                    //��������½ڵ�ʱ�����ռ��߳̿��������ͷŵ�ǰ�ڵ㣬����޷�������ȡ����һ�ڵ�
                    pLastData->count_.AddAndGet(-1);
                    pLastData = NULL;
                }
                CLinkedListData_NPNC *pLinkedData = (CLinkedListData_NPNC *)pList->GetValue(pNextNode);
                if (p->m_FuncCallBack != NULL)
                    p->m_FuncCallBack(pLinkedData->type_, pLinkedData->data_, pLinkedData->pHandler_, p->m_CallBackContext);
                //GLOG_DEBUG("Consumer���� �ڵ�:0x%08x ��ַ:0x%08x ����ǰ������:%lld", pNextNode, pLinkedData, pLinkedData->count_.Get());
                if (pLinkedData->pSyncHandler_ != NULL) {
                    pLinkedData->pSyncHandler_->count_.AddAndGet(-1);
                }
                pLastData = pLinkedData;
                p->m_CurNode = pNextNode;
            }            
        }
    }
    return 0;
}

static int ConsumerStop(CThread *pThread)
{
    CConsumerContext_NPNC *p = (CConsumerContext_NPNC *)pThread->GetContext();
    p->m_running = false;
    return 0;
}


/***************�����ڴ�����߳�*********************/

//��CProdcerContextʵ����ʱһͬ�����������������߳�Ҳһͬ������
//��Ա��Ϊ���ã�����û����������
//���ɵ�ʵ��ֻ����һ���߳���ʹ�ã������߳�Finishʱ�ͷŸ�ʵ�����ڴ�
CGarbageCollector_NPNC::CGarbageCollector_NPNC(CProdcerContext_NPNC *pProdcerContext)
{
    m_pProdcerContext = pProdcerContext;
}

int CGarbageCollector_NPNC::Collect()
{
    //������ͷȡ�ڵ㣬����ڵ������Ϊ0��˵���ѱ�������ɣ�������ͷ�
    //��;�¼����������ֻ�ܴ�����ĩβȡ���ݣ����Զ�����֮ǰ���ݵļ�����û��Ӱ��
    //���ݻ���ʱ�������һ���ڵ㣬������;�¼�������������ܻ�ȡ��һ����Ч�ڵ�
    int free_count = 0;
    NODEHANDLE pListNode = m_pProdcerContext->m_List.HeadNode();    
    if (pListNode == NULL)
        return 0;

    while(1) {
        NODEHANDLE pNextListNode = m_pProdcerContext->m_List.NextNode(pListNode);
        if (pNextListNode == NULL)
            return free_count;

        CLinkedListData_NPNC *pData = (CLinkedListData_NPNC *)(m_pProdcerContext->m_List.GetValue(pListNode));
        if (pData == NULL) {
            m_pProdcerContext->m_List.EraseNode(pListNode);
            ++free_count;
        }
        else if (pData->count_.Get() <= 0) {
            //GLOG_DEBUG("Garbage���� �ڵ�:0x%08x ��ַ:0x%08x ����ǰ������:%lld", pListNode, pData, pData->count_.Get());
            m_pProdcerContext->m_List.EraseNode(pListNode);
            ++free_count;
        }

        pListNode = pNextListNode;
    }

    return 0;
}

static int GarbageCollectorInit(CThread *pThread) {
    CGarbageCollector_NPNC *p = (CGarbageCollector_NPNC *)pThread->GetContext();
    p->m_running = true;
    return 0;
}

static int GarbageCollectorRun(CThread *pThread)
{
    CGarbageCollector_NPNC *p = (CGarbageCollector_NPNC *)pThread->GetContext();
    while(p->m_running) {
        if (p->Collect() == 0)
            TimeUtil::Sleep(1000);
    }
    return 0;
}

static int GarbageCollectorFinish(CThread *pThread)
{
    CGarbageCollector_NPNC *p = (CGarbageCollector_NPNC *)pThread->GetContext();
    delete p;
    return 0;
}

static int GarbageCollectorStop(CThread *pThread) {
    CGarbageCollector_NPNC *p = (CGarbageCollector_NPNC *)pThread->GetContext();
    p->m_running = false;
    return 0;
}


/***************�������ݽṹ*********************/

CLinkedListData_NPNC::CLinkedListData_NPNC(void *_data, int _type, free_node_func_t _func, void *_handler, int _count) : count_(_count)
{
    data_ = _data;
    pHandler_ = _handler;
    type_ = _type;
    free_func_ = _func;
    pSyncHandler_ = NULL;
}

CLinkedListData_NPNC::~CLinkedListData_NPNC()
{
    if (data_ != NULL && free_func_ != NULL)
        free_func_(data_);
}

static void CLinkedListData_Free(void *list_node)
{
    CLinkedListData_NPNC *p = (CLinkedListData_NPNC *)list_node;
    delete p;
}


/***************publishͬ�������źŽṹ*********************/
CSyncHandler_NPNC::CSyncHandler_NPNC() : count_(0)
{

}

CSyncHandler_NPNC::~CSyncHandler_NPNC()
{

}



////////////////////////////////////////////////////////////////////////////////////////////////////

CProducer_NPNC::CProducer_NPNC(unsigned int max_consumer/*=1*/)
{
    CProdcerContext_NPNC *p = new CProdcerContext_NPNC(max_consumer);
    m_context = (HANDLE_NPNC)p;
}

CProducer_NPNC::~CProducer_NPNC()
{
    WaitFinish();   //����Producer�������������Consumer�����������԰�ȫ�����ֹͣ����Consumer
    CProdcerContext_NPNC *p = (CProdcerContext_NPNC *)m_context;
    if (p != NULL)
        delete p;
}

int CProducer_NPNC::Publish(void *_data, int _type/*=0*/, free_data_func_t _func/*=NULL*/, void *_handler/*=NULL*/)
{
    CProdcerContext_NPNC *pContext = (CProdcerContext_NPNC *)m_context;
    //��CLinkedListData�ṹ��ԭʼ���ݰ�װ����뵽����
    //�������ʱ���ݵ�ǰ������������������������һ�κ������-1�����������������������ϣ����ݿ����ͷ�
    CLinkedListData_NPNC *pListData = new CLinkedListData_NPNC(_data, _type, _func, _handler, 0);
    pListData->publish_index_.Set(pContext->m_PublishIdx.AddAndGet());  //�ؼ����ڲ�ͬ�߳�������Data��index��Ȼ�ǲ�ͬ��
    //���߳�ͬʱ����ʱ��ȷ��˳�����
    while(1) {
        CLinkedListData_NPNC *pEndData = (CLinkedListData_NPNC *)pContext->m_List.GetValue(pContext->m_List.EndNode());
        //Data�����е�һ��Data��Ȼ�ǻ�ȡ��indexΪ1���߳������룬֮��Ҳ�ǰ���index��˳������������
        if (pEndData == NULL) {
            pListData->count_.Set(GetConsumerCount());            
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish���� �ڵ�:0x%08x ��ַ:0x%08x ����ǰ������:%lld", it_tmp, pListData, pListData->count_.Get());
            break;
        }
        else if (pListData->publish_index_.Get() == pEndData->publish_index_.Get() + 1) {
            pListData->count_.Set(GetConsumerCount());            
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish���� �ڵ�:0x%08x ��ַ:0x%08x ����ǰ������:%lld", it_tmp, pListData, pListData->count_.Get());
            break;
        }
    }
    return 0;
}

HANDLE_NPNC CProducer_NPNC::PublishSync(void *_data, int _type/*=0*/, free_data_func_t _func/*=NULL*/, void *_handler/*=NULL*/)
{
    CProdcerContext_NPNC *pContext = (CProdcerContext_NPNC *)m_context;
    //��CLinkedListData�ṹ��ԭʼ���ݰ�װ����뵽����
    //�������ʱ���ݵ�ǰ������������������������һ�κ������-1�����������������������ϣ����ݿ����ͷ�
    CLinkedListData_NPNC *pListData = new CLinkedListData_NPNC(_data, _type, _func, _handler, 0);
    pListData->pSyncHandler_ = new CSyncHandler_NPNC;

    pListData->publish_index_.Set(pContext->m_PublishIdx.AddAndGet());  //�ؼ����ڲ�ͬ�߳�������Data��index��Ȼ�ǲ�ͬ��
    //���߳�ͬʱ����ʱ��ȷ��˳�����
    while(1) {
        CLinkedListData_NPNC *pEndData = (CLinkedListData_NPNC *)pContext->m_List.GetValue(pContext->m_List.EndNode());
        //Data�����е�һ��Data��Ȼ�ǻ�ȡ��indexΪ1���߳������룬֮��Ҳ�ǰ���index��˳������������
        if (pEndData == NULL) {
            int _count = GetConsumerCount();
            pListData->count_.Set(_count);
            pListData->pSyncHandler_->count_.Set(_count);
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish���� �ڵ�:0x%08x ��ַ:0x%08x ����ǰ������:%lld", it_tmp, pListData, pListData->count_.Get());
            break;
        }
        else if (pListData->publish_index_.Get() == pEndData->publish_index_.Get() + 1) {
            int _count = GetConsumerCount();
            pListData->count_.Set(_count);
            pListData->pSyncHandler_->count_.Set(_count);
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish���� �ڵ�:0x%08x ��ַ:0x%08x ����ǰ������:%lld", it_tmp, pListData, pListData->count_.Get());
            break;
        }
    }
    return (HANDLE_NPNC)pListData->pSyncHandler_;
}

int CProducer_NPNC::GetConsumerCount()
{
    CProdcerContext_NPNC *pContext = (CProdcerContext_NPNC *)m_context;
    return (int)(pContext->m_ConsumerIdx.Get() + 1);
}

HANDLE_NPNC CProducer_NPNC::GetContextHandle()
{
    return m_context;
}

void CProducer_NPNC::WaitFinish()
{
    CProdcerContext_NPNC *pContext = (CProdcerContext_NPNC *)m_context;
    for (unsigned int i=0; i<pContext->m_MaxConsumer; ++i) {
        if (pContext->m_pConsumerList[i] != NULL) {
            pContext->m_pConsumerList[i]->FinishAndStop();
        }
    }
    for (unsigned int i=0; i<pContext->m_MaxConsumer; ++i) {
        if (pContext->m_pConsumerList[i] != NULL) {
            pContext->m_pConsumerList[i]->WaitStop();
        }
    }
}

int CProducer_NPNC::RegisterNewConsumer(consumer_call_back_t _func, void *_callback_context/*=NULL*/)
{
    CConsumer_NPNC *pConsumer = new CConsumer_NPNC(this, _func, _callback_context);
    CConsumerContext_NPNC *pConsumerContext = (CConsumerContext_NPNC *)pConsumer->GetContextHandle();
    CProdcerContext_NPNC *pProdcerContext = (CProdcerContext_NPNC *)m_context;
    pConsumerContext->m_index = (int)(pProdcerContext->m_ConsumerIdx.AddAndGet());
    pConsumerContext->m_CurNode = pProdcerContext->m_List.EndNode();    //��ͨ�����Ӽ���������֤���ڵ����Ч

    if (pConsumerContext->m_index >= (int)pProdcerContext->m_MaxConsumer) {
        pProdcerContext->m_ConsumerIdx.AddAndGet(-1);   //�س�����������֤��������ֵ�İ�ȫ
        delete pConsumer;
        return -1;
    }
    pProdcerContext->m_pConsumerList[pConsumerContext->m_index] = pConsumer;
    pConsumerContext->m_thread->Start();        //ע��ɹ��������̣߳�ֱ��������ֹͣ�����̲߳�ֹͣ����;����ֹ��Ҫ�ڻص����������п���
    return 0;
}

int CProducer_NPNC::WaitHandle(HANDLE_NPNC _handle)
{
    CSyncHandler_NPNC *pHandle = (CSyncHandler_NPNC *)_handle;
    while(1) {
        if (pHandle->count_.Get() <= 0)
            break;
        TimeUtil::Sleep(1);
    }
    return 0;
}

void CProducer_NPNC::CloseHandle(HANDLE_NPNC _handle)
{
    CSyncHandler_NPNC *pHandle = (CSyncHandler_NPNC *)_handle;
    delete pHandle;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

CConsumer_NPNC::CConsumer_NPNC(CProducer_NPNC *pProducer, consumer_call_back_t _func, void *_callback_context)
{
    CProdcerContext_NPNC *pProducerContext = (CProdcerContext_NPNC *)pProducer->GetContextHandle();
    CConsumerContext_NPNC *p = new CConsumerContext_NPNC(pProducerContext, _func, _callback_context);
    m_context = (HANDLE_NPNC)p;
}

CConsumer_NPNC::~CConsumer_NPNC()
{
    CConsumerContext_NPNC *p = (CConsumerContext_NPNC *)m_context;
    if (p == NULL)
        return;
    
    delete p;
}

HANDLE_NPNC CConsumer_NPNC::GetContextHandle()
{
    return m_context;
}

int CConsumer_NPNC::FinishAndStop()
{
    CConsumerContext_NPNC *p = (CConsumerContext_NPNC *)m_context;
    return p->m_thread->Safe_Stop();
}

void CConsumer_NPNC::WaitStop()
{
    CConsumerContext_NPNC *p = (CConsumerContext_NPNC *)m_context;
    p->m_thread->Join();
}

//////////////////////////////////////////////////////////////////////////////////////////

CConsumerBase::CConsumerBase(CConsumerBase *_next/*=NULL*/)
{
    m_pNext = _next;
}

CConsumerBase::~CConsumerBase()
{

}

int CConsumerBase::Execute(int _type, void *_data, void *_handler, void *_context)
{
    if (m_pNext == NULL)
        return 0;
    return m_pNext->Execute(_type, _data, _handler, _context);
}

void CConsumerBase::SetNext(CConsumerBase *_next)
{
    m_pNext = _next;
}
