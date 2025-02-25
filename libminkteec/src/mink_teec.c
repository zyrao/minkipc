// Copyright (c) 2025, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include <stdlib.h>

#include "mink_teec.h"
#include "MinkCom.h"

#include "IClientEnv.h"
#include "IGPSession.h"
#include "IGPAppClient.h"
#include "CGPAppClient.h"
#include "CWait_open.h"
#include "IWait.h"

/**
 * @brief Get a MINK AppClient Object.
 *
 * @param app_client The MINK AppClient object requested by the client.
 * @return Object_OK on success.
 *         Object_ERROR_* on failure.
 */
static int32_t mink_get_app_client(Object *app_client)
{
	int32_t rv = Object_OK;
	Object root_env = Object_NULL;
	Object client_env = Object_NULL;

	rv = MinkCom_getRootEnvObject(&root_env);
	if (Object_isERROR(rv)) {
		MSGE("MinkCom_getRootEnvObject failed: 0x%x\n", rv);
		return rv;
	}

	rv = MinkCom_getClientEnvObject(root_env, &client_env);
	if (Object_isERROR(rv)) {
		MSGE("MinkCom_getClientEnvObject failed: 0x%x\n", rv);
		goto err_client_env;
	}

	/* Get the GPAppClient */
	rv = IClientEnv_open(client_env, CGPAppClient_UID, app_client);
	if (Object_isERROR(rv)) {
		MSGE("IClientEnv_open failed: %d\n", rv);
		goto err_app_client;
	}

err_app_client:
	Object_ASSIGN_NULL(client_env);

err_client_env:
	Object_ASSIGN_NULL(root_env);

	return rv;
}

/**
 * @brief Open a session with a Trusted Application over MINK-IPC.
 *
 * @param app_client The MINK AppClient object representing an opened context
 *                   with QTEE.
 * @param waiter_cbo The MINK Waiter Callback object to allow QTEE to request
 *                   cancellation of an invoked operation.
 * @param destination The UUID of the destination Trusted Application.
 * @param cancel_code A cancellation code to identify the cancellation request
 *                    on behalf of QTEE.
 * @param connection_method The method of connection to use.
 * @param connection_data Any necessary data required to support the connection
 *                        method chosen.
 * @param tee_paramTypes The type of the parameters in this request.
 * @param tee_exParamTypes The type of the extended parameters in this request
 *                         for use by QTEE.
 * @param m_params The MINK parameters passed to the MINK API in this request.
 * @param session The MINK object returned by QTEE to represent this session.
 * @param result The result returned from TEE.
 * @param eorigin The origin of the result from TEE.
 * @return Object_OK on success.
 *         Object_ERROR_* on failure.
 */
static int32_t
mink_open_session(Object app_client, Object waiter_cbo,
		  const TEEC_UUID *destination, uint32_t cancel_code,
		  uint32_t connection_method, uint32_t connection_data,
		  uint32_t tee_paramTypes, uint32_t tee_exParamTypes,
		  MINK_Parameter *m_params, Object *session,
		  TEEC_Result *result, uint32_t *eorigin)
{
	int32_t rv = Object_OK;
	uint32_t mem_sz_out[MAX_NUM_PARAMS] = { 0 };

	rv = IGPAppClient_openSession(app_client,
				      (const char *)destination,
				      sizeof(TEEC_UUID),
				      waiter_cbo,
				      cancel_code,
				      connection_method,
				      connection_data,
				      tee_paramTypes,
				      tee_exParamTypes,
				      m_params[0].in_buf.buf,
				      m_params[0].in_buf.len,
				      m_params[1].in_buf.buf,
				      m_params[1].in_buf.len,
				      m_params[2].in_buf.buf,
				      m_params[2].in_buf.len,
				      m_params[3].in_buf.buf,
				      m_params[3].in_buf.len,
				      m_params[0].out_buf.buf,
				      m_params[0].out_buf.len,
				      m_params[0].out_buf.len_out,
				      m_params[1].out_buf.buf,
				      m_params[1].out_buf.len,
				      m_params[1].out_buf.len_out,
				      m_params[2].out_buf.buf,
				      m_params[2].out_buf.len,
				      m_params[2].out_buf.len_out,
				      m_params[3].out_buf.buf,
				      m_params[3].out_buf.len,
				      m_params[3].out_buf.len_out,
				      Object_NULL,
				      Object_NULL,
				      Object_NULL,
				      Object_NULL,
				      &mem_sz_out[0],
				      &mem_sz_out[1],
				      &mem_sz_out[2],
				      &mem_sz_out[3],
				      session,
				      result,
				      eorigin);

	if (Object_isERROR(rv)) {
		MSGE("IGPAppClient_openSession failed: %d\n", rv);

		if (Object_ERROR_DEFUNCT == rv) {
			*result = TEEC_ERROR_TARGET_DEAD;
			*eorigin = TEEC_ORIGIN_TEE;
		} else if (Object_ERROR_BUSY == rv) {
			*result = TEEC_ERROR_BUSY;
			*eorigin = TEEC_ORIGIN_TEE;
		} else if (Object_ERROR_KMEM == rv ||
			   Object_ERROR_NOSLOTS == rv) {
			*result = TEEC_ERROR_OUT_OF_MEMORY;
			*eorigin = TEEC_ORIGIN_TEE;
		} else {
			*result = TEEC_ERROR_GENERIC;
			*eorigin = TEEC_ORIGIN_COMMS;
		}
	}

	for (size_t i = 0; i < MAX_NUM_PARAMS; i++)
		if (mem_sz_out[i] != 0)
			*m_params[i].out_buf.len_out = mem_sz_out[i];

	return rv;
}

/**
 * @brief Invoke a command within a session to a Trusted Application over
 * MINK-IPC.
 *
 * @param session The MINK object returned by QTEE to represent this session.
 * @param command_id Identifier for the command to invoke.
 * @param cancel_code A cancellation code to identify the cancellation request
 *                    on behalf of QTEE.
 * @param tee_paramTypes The type of the parameters in this request.
 * @param tee_exParamTypes The type of the extended parameters in this request
 *                         for use by QTEE.
 * @param m_params The MINK parameters passed to the MINK API in this request.
 * @param result The result returned from TEE.
 * @param eorigin The origin of the result from TEE.
 * @return Object_OK on success.
 *         Object_ERROR_* on failure.
 */
static int32_t mink_invoke_command(Object session, uint32_t command_id,
				   uint32_t cancel_code,
				   uint32_t tee_paramTypes,
				   uint32_t tee_exParamTypes,
				   MINK_Parameter *m_params,
				   TEEC_Result *result, uint32_t *eorigin)
{
	int32_t rv = Object_OK;
	uint32_t mem_sz_out[MAX_NUM_PARAMS] = { 0 };

	rv = IGPSession_invokeCommand(session,
				      command_id,
				      cancel_code,
				      MINK_TEEC_TIMEOUT_INFINITE,
				      tee_paramTypes,
				      tee_exParamTypes,
				      m_params[0].in_buf.buf,
				      m_params[0].in_buf.len,
				      m_params[1].in_buf.buf,
				      m_params[1].in_buf.len,
				      m_params[2].in_buf.buf,
				      m_params[2].in_buf.len,
				      m_params[3].in_buf.buf,
				      m_params[3].in_buf.len,
				      m_params[0].out_buf.buf,
				      m_params[0].out_buf.len,
				      m_params[0].out_buf.len_out,
				      m_params[1].out_buf.buf,
				      m_params[1].out_buf.len,
				      m_params[1].out_buf.len_out,
				      m_params[2].out_buf.buf,
				      m_params[2].out_buf.len,
				      m_params[2].out_buf.len_out,
				      m_params[3].out_buf.buf,
				      m_params[3].out_buf.len,
				      m_params[3].out_buf.len_out,
				      Object_NULL,
				      Object_NULL,
				      Object_NULL,
				      Object_NULL,
				      &mem_sz_out[0],
				      &mem_sz_out[1],
				      &mem_sz_out[2],
				      &mem_sz_out[3],
				      result,
				      eorigin);

	if (Object_isERROR(rv)) {
		MSGE("IGPSession_invokeCommand failed: %d\n", rv);

		if (Object_ERROR_DEFUNCT == rv) {
			*result = TEEC_ERROR_TARGET_DEAD;
			*eorigin = TEEC_ORIGIN_TEE;
		} else if (Object_ERROR_BUSY == rv) {
			*result = TEEC_ERROR_BUSY;
			*eorigin = TEEC_ORIGIN_TEE;
		} else if (Object_ERROR_KMEM == rv ||
			   Object_ERROR_NOSLOTS == rv) {
			*result = TEEC_ERROR_OUT_OF_MEMORY;
			*eorigin = TEEC_ORIGIN_TEE;
		} else {
			MSGE("Error communicating with TA: %d\n", rv);
			*result = TEEC_ERROR_GENERIC;
			*eorigin = TEEC_ORIGIN_COMMS;
		}

		if (eorigin) {
			*eorigin = TEEC_ORIGIN_TEE;
		}
	}

	for (size_t i = 0; i < MAX_NUM_PARAMS; i++)
		if (mem_sz_out[i] != 0)
			*m_params[i].out_buf.len_out = mem_sz_out[i];

	return rv;
}

/**
 * @brief Get the TEE_* type corresponding to a TEEC_* MemRef parameter type.
 *
 * @param teec_type The TEEC_* type defined by the Global Platform TEE Client
 *                  API specification.
 * @param op The operation payload for the request being sent.
 * @param i Index of the parameter for which conversion is required.
 * @return TEE_* type for a Memory Reference parameter.
 *         TEEC_* type for any other parameter.
 */
static uint32_t get_tee_type(uint32_t teec_type, TEEC_Operation *op, size_t i)
{
	uint32_t flags = 0;

	switch (teec_type) {
	case TEEC_MEMREF_PARTIAL_INPUT:
		return TEE_PARAM_TYPE_MEMREF_INPUT;
	case TEEC_MEMREF_PARTIAL_OUTPUT:
		return TEE_PARAM_TYPE_MEMREF_OUTPUT;
	case TEEC_MEMREF_PARTIAL_INOUT:
		return TEE_PARAM_TYPE_MEMREF_INOUT;
	case TEEC_MEMREF_WHOLE:

		flags = op->params[i].memref.parent->flags;

		if ((flags & TEEC_MEM_INPUT) && (flags & TEEC_MEM_OUTPUT))
			return TEE_PARAM_TYPE_MEMREF_INOUT;

		if (flags & TEEC_MEM_INPUT)
			return TEE_PARAM_TYPE_MEMREF_INPUT;

		if (flags & TEEC_MEM_OUTPUT)
			return TEE_PARAM_TYPE_MEMREF_OUTPUT;

		break;
	default:
		break;
	}

	return teec_type;
}

/**
 * @brief Convert TEEC_* types to TEE_* types for Memory Reference parameters.
 *
 * @param op The operation payload for the request being sent.
 * @param tee_pType The parameter type encoding for the operation payload.
 */
static void tee_types_from_teec_types(TEEC_Operation *op, uint32_t *tee_pType)
{
	uint32_t teec_type = TEEC_NONE;
	uint32_t tee_type = TEEC_NONE;
	*tee_pType = op->paramTypes;

	for (size_t i = 0; i < MAX_NUM_PARAMS; i++) {
		teec_type = TEEC_PARAM_TYPE_GET(op->paramTypes, i);
		tee_type = get_tee_type(teec_type, op, i);

		/* In-place modification of mask */
		*tee_pType = TEEC_PARAM_TYPE_SET(tee_type, i, *tee_pType);
	}
}

/**
 * @brief Initialize the MINK parameters.
 *
 * @param m_params The MINK parameters to be passed to a MINK API.
 */
static void mink_params_INIT(MINK_Parameter *m_params)
{
	for (size_t i = 0; i < MAX_NUM_PARAMS; i++) {
		m_params[i].in_buf.buf = NULL;
		m_params[i].in_buf.len = 0;
		m_params[i].in_buf.sh_obj_index = 0;

		m_params[i].out_buf.buf = NULL;
		m_params[i].out_buf.len = 0;
		m_params[i].out_buf.len_out = &m_params[i].out_buf.len;
	}
}

/**
 * @brief Convert TEEC_* parameters to MINK parameters.
 *
 * @param param_types The parameter type encoding for the operation payload.
 * @param params The list of TEEC_* parameters in operation payload.
 * @param m_params The MINK parameters to be passed to a MINK API.
 * @param tee_exParamTypes The extended parameters type encoding in this
 *                         request for use by QTEE.
 */
static void mink_params_from_teec_params(uint32_t param_types,
					 TEEC_Parameter *params,
					 MINK_Parameter *m_params,
					 uint32_t *tee_exParamTypes)
{

	uint32_t type = TEEC_NONE;

	for (size_t i = 0; i < MAX_NUM_PARAMS; i++) {

		type = TEEC_PARAM_TYPE_GET(param_types, i);
		switch(type) {
		case TEEC_VALUE_INPUT:
		case TEEC_VALUE_OUTPUT:
		case TEEC_VALUE_INOUT:

			if (type == TEEC_VALUE_INPUT ||
			    type == TEEC_VALUE_INOUT) {
				m_params[i].in_buf.buf = &params[i].value;
				m_params[i].in_buf.len =  sizeof(TEEC_Value);
				m_params[i].in_buf.sh_obj_index = 0xFF; // N/A
			}

			if (type == TEEC_VALUE_OUTPUT ||
			    type == TEEC_VALUE_INOUT) {
				m_params[i].out_buf.buf = &params[i].value;
				m_params[i].out_buf.len =  sizeof(TEEC_Value);
				/* For TEEC_Value as per GP Spec, no size field
				 * is updated when returning from TEE. However to
				 * satisfy the IDL interface, we need to set a
				 * valid pointer for size_out. This is a no-op.
				 */
				m_params[i].out_buf.len_out = &m_params[i].out_buf.len;
			}

			break;
		case TEEC_MEMREF_TEMP_INPUT:
		case TEEC_MEMREF_TEMP_OUTPUT:
		case TEEC_MEMREF_TEMP_INOUT:

			/* Implicitly handles NULL tmpref */
			m_params[i].in_buf.buf = params[i].tmpref.buffer;
			m_params[i].in_buf.len = params[i].tmpref.size;
			m_params[i].in_buf.sh_obj_index = 0;

			if (type == TEEC_MEMREF_TEMP_OUTPUT ||
			    type == TEEC_MEMREF_TEMP_INOUT) {
				m_params[i].out_buf.buf = params[i].tmpref.buffer;
				m_params[i].out_buf.len =  params[i].tmpref.size;
			}
			m_params[i].out_buf.len_out = &params[i].tmpref.size;

			if (params[i].tmpref.buffer == NULL)
				*tee_exParamTypes = TEEC_PARAM_TYPE_SET(TEE_EX_PARAM_TYPE_MEMREF_NULL, i, *tee_exParamTypes);

			break;
		case TEEC_MEMREF_PARTIAL_INPUT:
		case TEEC_MEMREF_PARTIAL_OUTPUT:
		case TEEC_MEMREF_PARTIAL_INOUT:

			m_params[i].in_buf.buf = params[i].memref.parent->buffer + params[i].memref.offset;
			m_params[i].in_buf.len = params[i].memref.size;
			m_params[i].in_buf.sh_obj_index = 0;

			if (type == TEEC_MEMREF_PARTIAL_OUTPUT ||
			    type == TEEC_MEMREF_PARTIAL_INOUT) {
				m_params[i].out_buf.buf = params[i].memref.parent->buffer + params[i].memref.offset;
				m_params[i].out_buf.len = params[i].memref.size;
			}
			m_params[i].out_buf.len_out = &params[i].memref.size;

			break;
		case TEEC_MEMREF_WHOLE:

			m_params[i].in_buf.buf = params[i].memref.parent->buffer;
			m_params[i].in_buf.len = params[i].memref.parent->size;
			m_params[i].in_buf.sh_obj_index = 0;

			if (params[i].memref.parent->flags & TEEC_MEM_OUTPUT) {
				m_params[i].out_buf.buf = params[i].memref.parent->buffer;
				m_params[i].out_buf.len = params[i].memref.parent->size;
			}
			/* As per the GP spec, even if type is MEMREF_WHOLE, we must update the size here */
			m_params[i].out_buf.len_out = &params[i].memref.size;

			break;
		default:
			break;
		}
	}
}

TEEC_Result initialize_context(TEEC_Context *ctx)
{
	TEEC_Result ret = TEEC_SUCCESS;
	int32_t rv = Object_OK;

	Object app_client = Object_NULL;
	Object waiter_cbo = Object_NULL;

	/* Get the GP App client object */
	rv = mink_get_app_client(&app_client);
	if (Object_isERROR(rv)) {
		MSGE("mink_get_app_client failed: %d\n", rv);
		ret = TEEC_ERROR_GENERIC;
		goto err_gp_app_client;
	}

	/* Get the Cancellation CBO */
	rv = CWait_open(&waiter_cbo);
	if (Object_isERROR(rv)) {
		MSGE("CWait_open failed: 0x%x\n", rv);
		ret = TEEC_ERROR_GENERIC;
		goto err_waiter_cbo;
	}

	/* Store these Mink Objects for the current
	 * context
	 */
	ctx->imp.app_client = app_client;
	ctx->imp.waiter_cbo = waiter_cbo;

	return ret;

err_waiter_cbo:
	Object_ASSIGN_NULL(app_client);

err_gp_app_client:

	return ret;
}

void finalize_context(TEEC_Context *ctx)
{
	Object_ASSIGN_NULL(ctx->imp.waiter_cbo);
	Object_ASSIGN_NULL(ctx->imp.app_client);
}

TEEC_Result open_session(TEEC_Context *ctx, TEEC_Session *session,
			 const TEEC_UUID *destination, uint32_t conn_method,
			 const void *connection_data, TEEC_Operation *op,
			 uint32_t *ret_origin)
{
	int32_t ret = Object_OK;

	TEEC_Result result = TEEC_SUCCESS;
	uint32_t eorigin = TEEC_ORIGIN_COMMS;
	uint32_t conn_data = 0;
	if (connection_data)
		conn_data = *(const uint32_t *)connection_data;

	uint32_t cancel_code = 0;

	if (ret_origin) {
		*ret_origin = TEEC_ORIGIN_COMMS;
	}

	uint32_t tee_paramTypes = 0;
	uint32_t tee_exParamTypes = 0;
	MINK_Parameter m_params[MAX_NUM_PARAMS];
	mink_params_INIT(m_params);

	if (op) {
		cancel_code = (rand() & CANCEL_CODE_MASK);
		op->imp.cancel_code = cancel_code;
		op->imp.session = session;

		mink_params_from_teec_params(op->paramTypes, op->params,
					     m_params, &tee_exParamTypes);

		tee_types_from_teec_types(op, &tee_paramTypes);
	}

	ret = mink_open_session(ctx->imp.app_client, ctx->imp.waiter_cbo,
				destination, cancel_code, conn_method,
				conn_data, tee_paramTypes, tee_exParamTypes,
				m_params, &(session->imp.session_obj), &result,
				&eorigin);

	if (ret)
		MSGE("mink_open_session() failed: %d\n", ret);

	if (result) {
		/* If we have an error originating from trusted app, then
		 * returned session object is not NULL and needs to be assigned
		 * NULL. But if we have an error originating from TEE or
		 * transport, then no session object is created.
		 */
		if (eorigin == TEEC_ORIGIN_TRUSTED_APP)
			Object_ASSIGN_NULL(session->imp.session_obj);
	} else {
		session->imp.ctx = ctx;
	}

	if (ret_origin)
		*ret_origin = eorigin;

	return result;
}

void close_session(TEEC_Session *session)
{
	Object_ASSIGN_NULL(session->imp.session_obj);
	session->imp.ctx = NULL;
}

TEEC_Result invoke_command(TEEC_Session *session, uint32_t command_id,
			   TEEC_Operation *op, uint32_t *ret_origin)
{
	TEEC_Result ret = TEEC_SUCCESS;

	TEEC_Result result = TEEC_SUCCESS;
	uint32_t eorigin = TEEC_ORIGIN_COMMS;
	uint32_t cancel_code = 0;

	if (ret_origin) {
		*ret_origin = TEEC_ORIGIN_COMMS;
	}

	uint32_t tee_paramTypes = 0;
	uint32_t tee_exParamTypes = 0;
	MINK_Parameter m_params[MAX_NUM_PARAMS];
	mink_params_INIT(m_params);

	if (op) {
		cancel_code = (rand() & CANCEL_CODE_MASK);
		op->imp.cancel_code = cancel_code;
		op->imp.session = session;

		mink_params_from_teec_params(op->paramTypes, op->params,
					     m_params, &tee_exParamTypes);

		tee_types_from_teec_types(op, &tee_paramTypes);
	}

	ret = mink_invoke_command(session->imp.session_obj, command_id,
				  cancel_code, tee_paramTypes, tee_exParamTypes,
				  m_params, &result, &eorigin);

	if (ret)
		MSGE("mink_invoke_command() failed: %d\n", ret);

	if (ret_origin)
		*ret_origin = eorigin;

	return result;
}

TEEC_Result register_shared_memory(TEEC_Context *ctx, TEEC_SharedMemory *shm,
				   uint8_t convert)
{
	/* This shared memory is a Registered Memory */
	shm->imp.type = TEEC_MEMORY_REGISTERED;

	/* Is this being converted from a TempMemoryReference? */
	shm->imp.converted = convert;

	/* Based on size, we might need to use a MINK Memory Object */
	shm->imp.mem_obj = Object_NULL;
	shm->imp.ctx = ctx;

	return TEEC_SUCCESS;
}

TEEC_Result allocate_shared_memory(TEEC_Context *ctx, TEEC_SharedMemory *shm)
{
	if (shm->size <= TEEC_SHM_MAX_HEAP_SZ) {
		shm->buffer = (void *)malloc(shm->size);
		shm->imp.mem_obj = Object_NULL;
	}

	/* Otherwise, allocate using MINK Memory Object */

	/* This shared memory is a Registered Memory */
	shm->imp.type = TEEC_MEMORY_ALLOCATED;
	shm->imp.ctx = ctx;

	return TEEC_SUCCESS;
}

void release_shared_memory(TEEC_SharedMemory *shm)
{
	if (shm->imp.type == TEEC_MEMORY_ALLOCATED) {
		free(shm->buffer);
		shm->buffer = NULL;
		shm->size = 0;
	}

	/* If there's a backing memory object, release it */
	Object_ASSIGN_NULL(shm->imp.mem_obj);

	shm->imp.converted = 0;
	shm->imp.type = TEEC_MEMORY_FREE;
	shm->imp.ctx = NULL;
}

void request_cancellation(TEEC_Operation *op)
{
	TEEC_Session *session = (TEEC_Session *)op->imp.session;
	TEEC_Context *ctx = (TEEC_Context *)session->imp.ctx;

	Object waiter_cbo = ctx->imp.waiter_cbo;
	if (Object_isNull(waiter_cbo)) {
		MSGE("Waiter CBO not available!\n");
		return;
	}

	IWait_signal(waiter_cbo, op->imp.cancel_code, IWait_EVENT_CANCEL);
}
