// Copyright (c) 2025, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

// DO NOT EDIT (idlc v0.2.0)
#pragma once

#include <stdint.h>
#include "object.h"



#define IGPAppClient_ERROR_INVALID_UUID_LEN INT32_C(10)
#define IGPAppClient_ERROR_APP_NOT_FOUND INT32_C(11)
#define IGPAppClient_ERROR_APP_BLOCKED_ON_LISTENER INT32_C(12)
#define IGPAppClient_ERROR_APP_UNLOADED INT32_C(13)
#define IGPAppClient_ERROR_APP_RESTART_FAILED INT32_C(14)
#define IGPAppClient_ERROR_APP_AUTOLOAD_INVALID_BUFFER INT32_C(15)
#define IGPAppClient_ERROR_ACCESS_DENIED INT32_C(16)

#define IGPAppClient_OP_openSession 0
#define IGPAppClient_OP_openSessionV2 1

static inline int32_t
IGPAppClient_release(Object self)
{
    return Object_invoke(self, Object_OP_release, 0, 0);
}

static inline int32_t
IGPAppClient_retain(Object self)
{
    return Object_invoke(self, Object_OP_retain, 0, 0);
}

/*
*
*  Opens a session with the trusted application.
*
*  This method is part of the GP interface for the TA, and should not be directly used (internal to GP framework)
*
*  The caller/implementer (depending on parameter type) manually marshals the
*  content of four input/output buffers.
*
*  @param[in]  uuid               TA to open a session with, as TEE_UUID.
*  @param[in]  waitCBO            One waitCBO per session, will be used (when present) to implement the cancellation.
*  @param[in]  cancelCode         Optional code to use for cancellations.
*  @param[in]  connectionMethod   What CA identity credentials to use.
*  @param[in]  connectionData     Optional connection group identifier.
*  @param[in]  paramTypes         Parameter types, 1 byte per parameter.
*  @param[in]  exParamTypes       Extended information for parameters, 1 byte per parameter.
*  @param[in]  i1                 First input buffer.
*  @param[in]  i2                 Second input buffer.
*  @param[in]  i3                 Third input buffer.
*  @param[in]  i4                 Fourth input buffer.
*  @param[out] o1                 First output buffer.
*  @param[out] o2                 Second output buffer.
*  @param[out] o3                 Third output buffer.
*  @param[out] o4                 Fourth output buffer.
*  @param[in]  imem1              First optional memory region.
*  @param[in]  imem2              Second optional memory region.
*  @param[in]  imem3              Third optional memory region.
*  @param[in]  imem4              Fourth optional memory region.
*  @param[out] memrefOutSz1       Desired output size for memref 1, if larger than size provided.
*  @param[out] memrefOutSz2       Desired output size for memref 2, if larger than size provided.
*  @param[out] memrefOutSz3       Desired output size for memref 3, if larger than size provided.
*  @param[out] memrefOutSz4       Desired output size for memref 4, if larger than size provided.
*  @param[out] session            Newly opened session.
*  @param[out] retValue           GP return value.
*  @param[out] retOrigin          Where the GP return value originated.
*
*  @return  Object_OK if successful.
*
*/
static inline int32_t IGPAppClient_openSession(Object self, const void *uuid_ptr, size_t uuid_len, Object waitCBO, uint32_t cancelCode_val, uint32_t connectionMethod_val, uint32_t connectionData_val, uint32_t paramTypes_val, uint32_t exParamTypes_val, const void *i1_ptr, size_t i1_len, const void *i2_ptr, size_t i2_len, const void *i3_ptr, size_t i3_len, const void *i4_ptr, size_t i4_len, void *o1_ptr, size_t o1_len, size_t *o1_lenout, void *o2_ptr, size_t o2_len, size_t *o2_lenout, void *o3_ptr, size_t o3_len, size_t *o3_lenout, void *o4_ptr, size_t o4_len, size_t *o4_lenout, Object imem1, Object imem2, Object imem3, Object imem4, uint32_t *memrefOutSz1_ptr, uint32_t *memrefOutSz2_ptr, uint32_t *memrefOutSz3_ptr, uint32_t *memrefOutSz4_ptr, Object *session, uint32_t *retValue_ptr, uint32_t *retOrigin_ptr)
{
    struct bi {
        uint32_t m_cancelCode;
        uint32_t m_connectionMethod;
        uint32_t m_connectionData;
        uint32_t m_paramTypes;
        uint32_t m_exParamTypes;
    } i;

    i.m_cancelCode = cancelCode_val;
    i.m_connectionMethod = connectionMethod_val;
    i.m_connectionData = connectionData_val;
    i.m_paramTypes = paramTypes_val;
    i.m_exParamTypes = exParamTypes_val;
    struct bo {
        uint32_t m_memrefOutSz1;
        uint32_t m_memrefOutSz2;
        uint32_t m_memrefOutSz3;
        uint32_t m_memrefOutSz4;
        uint32_t m_retValue;
        uint32_t m_retOrigin;
    } o = {0,0,0,0,0,0};

    ObjectArg a[] = {
        {.b = (ObjectBuf) { &i, 20 } },
        {.bi = (ObjectBufIn) { uuid_ptr, uuid_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i1_ptr, i1_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i2_ptr, i2_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i3_ptr, i3_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i4_ptr, i4_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) {  &o, 24 } },
        {.b = (ObjectBuf) { o1_ptr, o1_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) { o2_ptr, o2_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) { o3_ptr, o3_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) { o4_ptr, o4_len * sizeof(uint8_t) } },
        {.o = waitCBO },
        {.o = imem1 },
        {.o = imem2 },
        {.o = imem3 },
        {.o = imem4 },
        {.o = Object_NULL },
    };

    int32_t result = Object_invoke(self, IGPAppClient_OP_openSession, a, ObjectCounts_pack(6, 5, 5, 1));

    *memrefOutSz1_ptr = o.m_memrefOutSz1;
    *memrefOutSz2_ptr = o.m_memrefOutSz2;
    *memrefOutSz3_ptr = o.m_memrefOutSz3;
    *memrefOutSz4_ptr = o.m_memrefOutSz4;
    *retValue_ptr = o.m_retValue;
    *retOrigin_ptr = o.m_retOrigin;
    *o1_lenout = a[7].b.size / sizeof(uint8_t);
    *o2_lenout = a[8].b.size / sizeof(uint8_t);
    *o3_lenout = a[9].b.size / sizeof(uint8_t);
    *o4_lenout = a[10].b.size / sizeof(uint8_t);
    *session = a[16].o;
    return result;
}


static inline int32_t IGPAppClient_openSessionV2(Object self, const void *uuid_ptr, size_t uuid_len, Object waitCBO, uint32_t cancelCode_val, uint32_t cancellationRequestTimeout_val, uint32_t connectionMethod_val, uint32_t connectionData_val, uint32_t paramTypes_val, uint32_t exParamTypes_val, const void *i1_ptr, size_t i1_len, const void *i2_ptr, size_t i2_len, const void *i3_ptr, size_t i3_len, const void *i4_ptr, size_t i4_len, void *o1_ptr, size_t o1_len, size_t *o1_lenout, void *o2_ptr, size_t o2_len, size_t *o2_lenout, void *o3_ptr, size_t o3_len, size_t *o3_lenout, void *o4_ptr, size_t o4_len, size_t *o4_lenout, Object imem1, Object imem2, Object imem3, Object imem4, uint32_t *memrefOutSz1_ptr, uint32_t *memrefOutSz2_ptr, uint32_t *memrefOutSz3_ptr, uint32_t *memrefOutSz4_ptr, Object *session, uint32_t *retValue_ptr, uint32_t *retOrigin_ptr)
{
    struct bi {
        uint32_t m_cancelCode;
        uint32_t m_cancellationRequestTimeout;
        uint32_t m_connectionMethod;
        uint32_t m_connectionData;
        uint32_t m_paramTypes;
        uint32_t m_exParamTypes;
    } i;

    i.m_cancelCode = cancelCode_val;
    i.m_cancellationRequestTimeout = cancellationRequestTimeout_val;
    i.m_connectionMethod = connectionMethod_val;
    i.m_connectionData = connectionData_val;
    i.m_paramTypes = paramTypes_val;
    i.m_exParamTypes = exParamTypes_val;
    struct bo {
        uint32_t m_memrefOutSz1;
        uint32_t m_memrefOutSz2;
        uint32_t m_memrefOutSz3;
        uint32_t m_memrefOutSz4;
        uint32_t m_retValue;
        uint32_t m_retOrigin;
    } o = {0,0,0,0,0,0};

    ObjectArg a[] = {
        {.b = (ObjectBuf) { &i, 24 } },
        {.bi = (ObjectBufIn) { uuid_ptr, uuid_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i1_ptr, i1_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i2_ptr, i2_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i3_ptr, i3_len * sizeof(uint8_t) } },
        {.bi = (ObjectBufIn) { i4_ptr, i4_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) {  &o, 24 } },
        {.b = (ObjectBuf) { o1_ptr, o1_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) { o2_ptr, o2_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) { o3_ptr, o3_len * sizeof(uint8_t) } },
        {.b = (ObjectBuf) { o4_ptr, o4_len * sizeof(uint8_t) } },
        {.o = waitCBO },
        {.o = imem1 },
        {.o = imem2 },
        {.o = imem3 },
        {.o = imem4 },
        {.o = Object_NULL },
    };

    int32_t result = Object_invoke(self, IGPAppClient_OP_openSessionV2, a, ObjectCounts_pack(6, 5, 5, 1));

    *memrefOutSz1_ptr = o.m_memrefOutSz1;
    *memrefOutSz2_ptr = o.m_memrefOutSz2;
    *memrefOutSz3_ptr = o.m_memrefOutSz3;
    *memrefOutSz4_ptr = o.m_memrefOutSz4;
    *retValue_ptr = o.m_retValue;
    *retOrigin_ptr = o.m_retOrigin;
    *o1_lenout = a[7].b.size / sizeof(uint8_t);
    *o2_lenout = a[8].b.size / sizeof(uint8_t);
    *o3_lenout = a[9].b.size / sizeof(uint8_t);
    *o4_lenout = a[10].b.size / sizeof(uint8_t);
    *session = a[16].o;
    return result;
}

