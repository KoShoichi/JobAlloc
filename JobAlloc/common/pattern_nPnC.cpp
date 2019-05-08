#include "pattern_nPnC.h"
#include "linked_list.h"
#include "sequence.h"
#include "Thread.h"
#include "time_util.h"
#include "applog.h"

//链表内存回收线程
static int GarbageCollectorInit(CThread *pThread);
static int GarbageCollectorRun(CThread *pThread);
static int GarbageCollectorFinish(CThread *pThread);
static int GarbageCollectorStop(CThread *pThread);
//链表存储数据结构
static void CLinkedListData_Free(void *list_node);
//消费者线程
static int ConsumerRun(CThread *pThread);
static int ConsumerStop(CThread *pThread);



//消费者接口，目前不对外开放，其构建和析构均由生产者来控制
class CConsumer_NPNC
{
public:
    //自行构建Consumer与在Producer中调用RegisterNewConsumer效果相同
    CConsumer_NPNC(CProducer_NPNC *pProducer, consumer_call_back_t _func, void *_callback_context);
    //注册的Producer如果析构了，那么Consumer会随之一同析构
    ~CConsumer_NPNC();

    //完成剩余的数据处理后停止工作，非阻塞
    int FinishAndStop();
    //等待其停止工作，阻塞
    void WaitStop();
    HANDLE_NPNC GetContextHandle();

private:
    HANDLE_NPNC m_context;
};


//生产者数据
class CProdcerContext_NPNC
{
public:
    //CProdcer实例化时一同实例化
    CProdcerContext_NPNC(unsigned int max_consumer=1);
    ~CProdcerContext_NPNC();

    CLinkedList m_List;                 //生产者消费者之间的数据队列
    CConsumer_NPNC **m_pConsumerList;   //所有的消费者
    Sequence m_ConsumerIdx;             //注册上来的消费者的数据
    unsigned int m_MaxConsumer;         //允许的最大消费者数目
    CThread *m_GarbageCollector;        //数据内存回收线程
    Sequence m_PublishIdx;
};


//消费者数据
class CConsumerContext_NPNC
{
public:
    CConsumerContext_NPNC(CProdcerContext_NPNC *pProdcerContext, consumer_call_back_t _func, void *_callback_context);
    ~CConsumerContext_NPNC();

    CProdcerContext_NPNC *m_pProdcerContext;
    int m_index;
    bool m_running;
    CThread *m_thread;        //消费者工作线程
    consumer_call_back_t m_FuncCallBack;
    void *m_CallBackContext;
    NODEHANDLE m_CurNode;   //读取到的数据节点
};


//链表内存回收线程数据
class CGarbageCollector_NPNC
{
public:
    //创建相关线程时实例化
    CGarbageCollector_NPNC(CProdcerContext_NPNC *pProdcerContext);
    //回收链表中使用过的数据内存
    int Collect();

    CProdcerContext_NPNC *m_pProdcerContext;
    bool m_running;
};

//publish同步返回信号结构
class CSyncHandler_NPNC
{
public:
    CSyncHandler_NPNC();
    ~CSyncHandler_NPNC();
    Sequence count_;
};


//链表存储数据结构
class CLinkedListData_NPNC
{
public:
    //数据发布时有多少
    CLinkedListData_NPNC(void *_data, int _type, free_node_func_t _func, void *_handler, int _count);
    ~CLinkedListData_NPNC();

    int type_;
    void *data_;
    void *pHandler_;
    free_node_func_t free_func_;
    Sequence count_;            //数据消费计数器
    Sequence publish_index_;    //用于多线程插入时核对尾结点的顺序递增
    CSyncHandler_NPNC *pSyncHandler_;
};




/***************生产者数据*********************/

//在CProducer实例化时创建，作为CProducer成员
CProdcerContext_NPNC::CProdcerContext_NPNC(unsigned int max_consumer/*=1*/)
{    
    if (max_consumer <= 0)
        m_MaxConsumer = 1;
    else if (max_consumer > 1000)
        m_MaxConsumer = 1000;
    else
        m_MaxConsumer = max_consumer;

    m_pConsumerList = (CConsumer_NPNC **)calloc(m_MaxConsumer, sizeof(CConsumer_NPNC *));

    //队列垃圾回收线程设置
    m_GarbageCollector = new CThread;
    CGarbageCollector_NPNC *pGarbageCollector = new CGarbageCollector_NPNC(this);
    m_GarbageCollector->RegisterContext((void *)pGarbageCollector);
    m_GarbageCollector->RegisterInitHanlde(GarbageCollectorInit);
    m_GarbageCollector->RegisterRunHanlde(GarbageCollectorRun);
    m_GarbageCollector->RegisterFinishHanlde(GarbageCollectorFinish);
    m_GarbageCollector->RegisterStopHanlde(GarbageCollectorStop);
    m_GarbageCollector->Start();    //线程在构造函数中直接启动，析构函数中停止
}

//在CProducer析构时一同析构
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


/***************消费者数据*********************/

CConsumerContext_NPNC::CConsumerContext_NPNC(CProdcerContext_NPNC *pProdcerContext, consumer_call_back_t _func, void *_callback_context)
{
    m_pProdcerContext = pProdcerContext;
    m_FuncCallBack = _func;
    m_CallBackContext = _callback_context;
    m_running = true;
    
    m_index = -1;       //通过Producer的注册方法来生成序号
    m_CurNode = NULL;

    //消费线程设置
    m_thread = new CThread;
    m_thread->RegisterContext((void *)this);
    m_thread->RegisterRunHanlde(ConsumerRun);
    m_thread->RegisterStopHanlde(ConsumerStop);
}

CConsumerContext_NPNC::~CConsumerContext_NPNC()
{
    delete m_thread;    //线程停止由调用Stop来控制
}

static int ConsumerRun(CThread *pThread)
{
    CConsumerContext_NPNC *p = (CConsumerContext_NPNC *)pThread->GetContext();
    CLinkedListData_NPNC *pLastData = NULL;
    while(p->m_running) {
        CLinkedList *pList = &(p->m_pProdcerContext->m_List);        

        if (p->m_CurNode == NULL) {
            //消费线程加入时队列中就没数据，那么循环检查队列头，直到有数据为止
            p->m_CurNode = pList->HeadNode();
            if (p->m_CurNode == NULL) {
                TimeUtil::Sleep(2);
                continue;
            }
            CLinkedListData_NPNC *pLinkedData = (CLinkedListData_NPNC *)pList->GetValue(p->m_CurNode);
            if (p->m_FuncCallBack != NULL)
                p->m_FuncCallBack(pLinkedData->type_, pLinkedData->data_, pLinkedData->pHandler_, p->m_CallBackContext);
            //GLOG_DEBUG("Consumer处理 节点:0x%08x 地址:0x%08x 处理前计数器:%lld", p->m_CurNode, pLinkedData, pLinkedData->count_.Get());
            if (pLinkedData->pSyncHandler_ != NULL) {
                pLinkedData->pSyncHandler_->count_.AddAndGet(-1);
            }
            pLastData = pLinkedData;
        }

        //以下为一个独立的循环，不受m_running控制，以便完成队列剩余的数据
        if (p->m_CurNode != NULL) {
            while(1) {
                //m_CurNode不为NULL时，其中的数据是已经被处理过的，判断是否有下个可用节点即可
                NODEHANDLE pNextNode = pList->NextNode(p->m_CurNode);                
                if (pNextNode == NULL) {                    
                    TimeUtil::Sleep(2);
                    break;
                }
                if (pLastData != NULL) {
                    //只有当取到下一个有效节点时才能将当前的节点释放，
                    //否则产生新节点时垃圾收集线程可能抢先释放当前节点，造成无法正常获取到下一节点
                    pLastData->count_.AddAndGet(-1);
                    pLastData = NULL;
                }
                CLinkedListData_NPNC *pLinkedData = (CLinkedListData_NPNC *)pList->GetValue(pNextNode);
                if (p->m_FuncCallBack != NULL)
                    p->m_FuncCallBack(pLinkedData->type_, pLinkedData->data_, pLinkedData->pHandler_, p->m_CallBackContext);
                //GLOG_DEBUG("Consumer处理 节点:0x%08x 地址:0x%08x 处理前计数器:%lld", pNextNode, pLinkedData, pLinkedData->count_.Get());
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


/***************链表内存回收线程*********************/

//在CProdcerContext实例化时一同被创建，包括工作线程也一同被创建
//成员都为引用，所以没有析构函数
//生成的实例只会在一个线程中使用，所以线程Finish时释放该实例的内存
CGarbageCollector_NPNC::CGarbageCollector_NPNC(CProdcerContext_NPNC *pProdcerContext)
{
    m_pProdcerContext = pProdcerContext;
}

int CGarbageCollector_NPNC::Collect()
{
    //从链表头取节点，如果节点计数降为0，说明已被消费完成，则进行释放
    //中途新加入的消费者只能从链表末尾取数据，所以对链表之前数据的计数器没有影响
    //数据回收时保留最后一个节点，用于中途新加入的消费者总能获取到一个有效节点
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
            //GLOG_DEBUG("Garbage处理 节点:0x%08x 地址:0x%08x 处理前计数器:%lld", pListNode, pData, pData->count_.Get());
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


/***************链表数据结构*********************/

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


/***************publish同步返回信号结构*********************/
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
    WaitFinish();   //现在Producer析构会带动所有Consumer都析构，所以安全起见先停止所有Consumer
    CProdcerContext_NPNC *p = (CProdcerContext_NPNC *)m_context;
    if (p != NULL)
        delete p;
}

int CProducer_NPNC::Publish(void *_data, int _type/*=0*/, free_data_func_t _func/*=NULL*/, void *_handler/*=NULL*/)
{
    CProdcerContext_NPNC *pContext = (CProdcerContext_NPNC *)m_context;
    //用CLinkedListData结构对原始数据包装后加入到队列
    //加入队列时根据当前的消费者数量来计数，消费一次后计数器-1，计数器归零则代表消费完毕，数据可以释放
    CLinkedListData_NPNC *pListData = new CLinkedListData_NPNC(_data, _type, _func, _handler, 0);
    pListData->publish_index_.Set(pContext->m_PublishIdx.AddAndGet());  //关键在于不同线程所生成Data的index必然是不同的
    //多线程同时插入时，确保顺序插入
    while(1) {
        CLinkedListData_NPNC *pEndData = (CLinkedListData_NPNC *)pContext->m_List.GetValue(pContext->m_List.EndNode());
        //Data链表中第一个Data必然是获取到index为1的线程来加入，之后也是按照index的顺序来加入链表
        if (pEndData == NULL) {
            pListData->count_.Set(GetConsumerCount());            
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish处理 节点:0x%08x 地址:0x%08x 处理前计数器:%lld", it_tmp, pListData, pListData->count_.Get());
            break;
        }
        else if (pListData->publish_index_.Get() == pEndData->publish_index_.Get() + 1) {
            pListData->count_.Set(GetConsumerCount());            
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish处理 节点:0x%08x 地址:0x%08x 处理前计数器:%lld", it_tmp, pListData, pListData->count_.Get());
            break;
        }
    }
    return 0;
}

HANDLE_NPNC CProducer_NPNC::PublishSync(void *_data, int _type/*=0*/, free_data_func_t _func/*=NULL*/, void *_handler/*=NULL*/)
{
    CProdcerContext_NPNC *pContext = (CProdcerContext_NPNC *)m_context;
    //用CLinkedListData结构对原始数据包装后加入到队列
    //加入队列时根据当前的消费者数量来计数，消费一次后计数器-1，计数器归零则代表消费完毕，数据可以释放
    CLinkedListData_NPNC *pListData = new CLinkedListData_NPNC(_data, _type, _func, _handler, 0);
    pListData->pSyncHandler_ = new CSyncHandler_NPNC;

    pListData->publish_index_.Set(pContext->m_PublishIdx.AddAndGet());  //关键在于不同线程所生成Data的index必然是不同的
    //多线程同时插入时，确保顺序插入
    while(1) {
        CLinkedListData_NPNC *pEndData = (CLinkedListData_NPNC *)pContext->m_List.GetValue(pContext->m_List.EndNode());
        //Data链表中第一个Data必然是获取到index为1的线程来加入，之后也是按照index的顺序来加入链表
        if (pEndData == NULL) {
            int _count = GetConsumerCount();
            pListData->count_.Set(_count);
            pListData->pSyncHandler_->count_.Set(_count);
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish处理 节点:0x%08x 地址:0x%08x 处理前计数器:%lld", it_tmp, pListData, pListData->count_.Get());
            break;
        }
        else if (pListData->publish_index_.Get() == pEndData->publish_index_.Get() + 1) {
            int _count = GetConsumerCount();
            pListData->count_.Set(_count);
            pListData->pSyncHandler_->count_.Set(_count);
            NODEHANDLE it_tmp = pContext->m_List.InsertAfter(NULL, pListData, CLinkedListData_Free);
            //GLOG_DEBUG("Publish处理 节点:0x%08x 地址:0x%08x 处理前计数器:%lld", it_tmp, pListData, pListData->count_.Get());
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
    pConsumerContext->m_CurNode = pProdcerContext->m_List.EndNode();    //先通过增加计数器来保证最后节点的有效

    if (pConsumerContext->m_index >= (int)pProdcerContext->m_MaxConsumer) {
        pProdcerContext->m_ConsumerIdx.AddAndGet(-1);   //回撤计数器来保证计数器数值的安全
        delete pConsumer;
        return -1;
    }
    pProdcerContext->m_pConsumerList[pConsumerContext->m_index] = pConsumer;
    pConsumerContext->m_thread->Start();        //注册成功即启动线程，直到生产者停止工作线程才停止，中途的中止需要在回调函数中自行控制
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
