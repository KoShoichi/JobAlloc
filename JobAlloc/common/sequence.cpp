#include "frame_public.h"
#include "sequence.h"

long long Sequence::CompareAndSet(long long expected_value, long long new_value)
{
    //TODO __sync_val_compare_and_swap 
    return InterlockedCompareExchange64(&value_, new_value, expected_value);
}

long long Sequence::AddAndGet(long long increment/*=1L*/)
{
    long long current_value;
    long long new_value;
    do {
        current_value = value_;
        new_value = current_value + increment;
        //CompareAndSet���ص���value_�ڱȽ�ʱ��ֵ������value_!=current_value����ʾ��ֵʧ�ܣ�������
    } while (current_value != CompareAndSet(current_value, new_value));
    return new_value;
}
