#include "frame_public.h"
#include "object.h"
#include "sequence.h"


CGObject::CGObject(int obj_type/*=0*/)
{
    pValue_ = NULL;
    pRefCount_ = new Sequence(1L);
    pPrevNo_ = new Sequence(0L);
    pPostNo_ = new Sequence(0L);
    m_ObjType = obj_type;
}

CGObject::~CGObject()
{
    if (pValue_ != NULL) {
        //��Ҫ��Free()�����������е���virtual�ĺ�����Σ��
        free(pValue_);
        pValue_ = NULL;
    }
    delete pRefCount_;
    delete pPrevNo_;
    delete pPostNo_;
}

void CGObject::AddRef()
{
    pRefCount_->AddAndGet();
}

void CGObject::Release()
{
    if (pRefCount_->AddAndGet(-1L) <= 0) {
        delete this;
        return;
    }
}

void * CGObject::Value()
{
    return pValue_;
}

void CGObject::Free()
{
    free(pValue_);
}

void * CGObject::Alloc(int iNum, void *pData/*=NULL*/)
{
    //iNum���ǽṹ��Ĵ�С
    if (iNum > 0)
        pValue_ = calloc(1, iNum);
    pRefCount_->Set(1);
    return pValue_;
}

long long CGObject::UpdatePrevNo()
{
    return pPrevNo_->AddAndGet();
}

long long CGObject::UpdatePostNo()
{
    return pPostNo_->AddAndGet();
}

long long CGObject::GetPrevNo()
{
    return pPrevNo_->Get();
}

long long CGObject::GetPostNo()
{
    return pPostNo_->Get();
}

int CGObject::GetObjType()
{
    return m_ObjType;
}

void free_object_data(void *_data)
{
    CGObject *p = (CGObject *)_data;
    p->Release();
}
