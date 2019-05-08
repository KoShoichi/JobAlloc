#pragma once
#ifndef _G_SEQUENCE_H
#define _G_SEQUENCE_H

#include "frame_public.h"

class DLL_API Sequence
{
	public:
		const static long long INITIAL_VALUE = -1L;

		Sequence(long long init_value = INITIAL_VALUE) {
			value_ = init_value;
		}

		virtual long long Get() {
			return value_;
		}

		virtual void Set(long long _val) {
			value_ = _val;        
		}

		virtual long long CompareAndSet(long long expected_value, long long new_value);

		virtual long long AddAndGet(long long increment = 1L);

	protected:
		//value_会在多线程环境中被频繁更改，使用pad避免value_更改对其他的缓存变量造成影响
		long long p1, p2, p3, p4, p5, p6, p7;

		volatile long long value_;

		long long p9, p10, p11, p12, p13, p14, p15;
};

#endif