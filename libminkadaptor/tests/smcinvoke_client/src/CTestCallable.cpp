// Copyright (c) 2025, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include "object.h"

#include "cdefs.h"
#include "ITestCBack.h"
#include "ITestCBack_invoke.h"
#include "CTestCallable_open.h"
#include "CTestCallable_priv.h"
#include "heap.h"
#include "tzt.h"

static int32_t CTestCallable_retain(TestCallable *me)
{
	if (!me) {
		return Object_ERROR;
	}

	me->refs++;
	LOGD_PRINT("[%s (%p)] %d -> %d\n", __FUNCTION__, me, me->refs - 1,
		   me->refs);
	return Object_OK;
}

static int32_t CTestCallable_release(TestCallable *me)
{
	if (!me) {
		return Object_ERROR;
	}

	me->refs--;
	LOGD_PRINT("[%s (%p)] %d -> %d\n", __FUNCTION__, me, me->refs + 1,
		   me->refs);

	if (me->refs == 0) {
		LOGD_PRINT("[%s (%p)] delete\n", __FUNCTION__, me);
		Object_RELEASE_IF(me->oArg);
		Object_RELEASE_IF(me->oOOArg);
		Object_RELEASE_IF(me->oOOArg0);
		Object_RELEASE_IF(me->oOOArg1);
		Object_RELEASE_IF(me->oOOArg2);
		HEAP_FREE_PTR(me);
	}
	return Object_OK;
}

static int32_t CTestCallable_call(TestCallable *me)
{
	if (!me) {
		return Object_ERROR;
	}

	LOGD_PRINT("[%s (%p)]\n", __FUNCTION__, me);

	me->counter++;
	me->op = ITestCallable_OP_call;
	return me->retValue;
}

static int32_t CTestCallable_callWithBuffer(TestCallable *me,
					    const void *arg_ptr, size_t arg_len)
{
	if (!me) {
		return Object_ERROR;
	}

	LOGD_PRINT("[%s (%p)]\n", __FUNCTION__, me);

	me->counter++;
	me->op = ITestCallable_OP_callWithBuffer;

	if ((arg_len == me->bArg_len) &&
	    (0 == memcmp(arg_ptr, me->bArg_ptr, arg_len))) {
		return me->retValue;
	} else {
		return me->retValueError;
	}
}

static int32_t CTestCallable_callWithObject(TestCallable *me, Object arg_val)
{
	if (!me) {
		return Object_ERROR;
	}

	LOGD_PRINT("[%s (%p)]\n", __FUNCTION__, me);

	me->counter++;
	me->op = ITestCallable_OP_callWithObject;

	// The passed object is supposed to be a callback Obj, so we can compare its members directly
	if ((arg_val.invoke == me->oArg.invoke) &&
	    (arg_val.context == me->oArg.context)) {

		return me->retValue;
	} else if (arg_val.invoke == me->oOOArg.invoke) {

		// oOOArg is initialized to a remote object, so we can use it for this comparison
		return ITestCallable_ERROR_OBJECT_REMOTE;
	} else {
		return me->retValueError;
	}
}

static ITestCallable_DEFINE_INVOKE(CTestCallable_invoke, CTestCallable_,
				   TestCallable *)

int32_t CTestCallable_open(Object remoteObj, Object *obj)
{
	TestCallable *me = HEAP_ZALLOC_TYPE(TestCallable);
	if (!me) {
		return Object_ERROR_KMEM;
	}

	// Keep a reference to the remote object
	Object_INIT(me->oOOArg, remoteObj);
	me->responseCounter = 0;
	me->refs = 1;
	me->op = -1;
	*obj = (Object){ CTestCallable_invoke, me };

	LOGD_PRINT("[%s (%p)]\n", __FUNCTION__, me);

	return Object_OK;
}
