// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <dlfcn.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "listener_mngr.h"
#include "MinkCom.h"

#include "CListenerCBO.h"
#include "CRegisterListenerCBO.h"
#include "IRegisterListenerCBO.h"
#include "IClientEnv.h"

#define MAX_LISTENERS 8
static struct listener_svc listeners[] = {
#ifdef TIME_LISTENER
	{
		.service_name = "time services",
		.id = TIME_SERVICE,
		.is_registered = false,
		.file_name = "libdrmtime.so.1",
		.lib_handle = NULL,
		.svc_register = NULL,
		.svc_deregister = NULL,
		.dispatch_func = "smci_dispatch",
		.cbo = Object_NULL,
		.buf_len = TIME_SERVICE_BUF_LEN,
	},
#endif
#ifdef TA_AUTOLOAD_LISTENER
	{
		.service_name = "taautoload service",
		.id = -1,
		.is_registered = false,
		.file_name = "libtaautoload.so.1",
		.lib_handle = NULL,
		.svc_register = "register_service",
		.svc_deregister = "deregister_service",
		.dispatch_func = NULL,
		.cbo = Object_NULL,
		.buf_len = 0,
	},
#endif
#ifdef FS_LISTENER
	{
		.service_name = "fs service",
		.id = FILE_SERVICE,
		.is_registered = false,
		.file_name = "libdrmfs.so.1",
		.lib_handle = NULL,
		.svc_register = NULL,
		.svc_deregister = NULL,
		.dispatch_func = "smci_dispatch",
		.cbo = Object_NULL,
		.buf_len = FILE_SERVICE_BUF_LEN,
	},
#endif
#ifdef GPFS_LISTENER
	{
		.service_name = "gpfs service",
		.id = GPFILE_SERVICE,
		.is_registered = false,
		.file_name = "libdrmfs.so.1",
		.lib_handle = NULL,
		.svc_register = NULL,
		.svc_deregister = NULL,
		.dispatch_func = "smci_gpdispatch",
		.cbo = Object_NULL,
		.buf_len = GPFILE_SERVICE_BUF_LEN,
	},
#endif
};

/* List of listener registration objects */
static Object obj_list[MAX_LISTENERS] = { Object_NULL };

/**
 * @brief De-register a listener service.
 *
 * De-registers a listener service by invoking the de-register callback defined by
 * the listener.
 */
static void dereg_listener_svc(size_t i)
{
	svc_deregister srv_deregister;

	srv_deregister = (svc_deregister)dlsym(listeners[i].lib_handle,
					       listeners[i].svc_deregister);
	if (srv_deregister == NULL) {
		MSGE("dlsym(%s) not found in lib %s: %s\n",
		     listeners[i].svc_deregister, listeners[i].file_name,
		     dlerror());
		return;
	}

	(*srv_deregister)();
}

/**
 * @brief Stop listener services.
 *
 * Stops all listener services waiting for a listener request from QTEE.
 */
static void stop_listeners_smci(void)
{
	size_t idx = 0;
	size_t n_listeners = sizeof(listeners)/sizeof(struct listener_svc);

	/* Stop the root listener services */
	MSGD("Total listener services to be stopped = %ld\n", n_listeners);

	for (idx = 0; idx < n_listeners; idx++) {
		if (Object_isNull(obj_list[idx]))
			continue;

		Object_ASSIGN_NULL(obj_list[idx]);
	}

	for (idx = 0; idx < n_listeners; idx++) {
		/* Resource cleanup for registered listeners */
		if(listeners[idx].is_registered) {
			Object_ASSIGN_NULL(listeners[idx].cbo);

			if (listeners[idx].svc_deregister)
				dereg_listener_svc(idx);

			listeners[idx].is_registered = false;
		}

		/* Close lib_handle for all listeners */
		if(listeners[idx].lib_handle != NULL) {
			dlclose(listeners[idx].lib_handle);
			listeners[idx].lib_handle = NULL;
		}
	}
}

/**
 * @brief Register a listener service.
 *
 * Registers a listener service by invoking the register callback defined by
 * the listener.
 */
static int reg_listener_svc(size_t i)
{
	svc_register srv_register;
	int ret = 0;

	listeners[i].lib_handle = dlopen(listeners[i].file_name,
					 RTLD_NOW);
	if (listeners[i].lib_handle == NULL) {
		MSGE("dlopen(%s, RLTD_NOW) failed: %s\n",
		     listeners[i].file_name, dlerror());
		return -1;
	}

	srv_register = (svc_register)dlsym(listeners[i].lib_handle,
					   listeners[i].svc_register);
	if (srv_register == NULL) {
		MSGE("dlsym(%s) not found in lib %s: %s\n",
		     listeners[i].svc_register, listeners[i].file_name,
		     dlerror());
		return -1;
	}

	ret = (*srv_register)();
	if (ret < 0) {
		MSGE("Register for dlsym(%s) failed: %d",
		     listeners[i].svc_register, ret);
		return -1;
	}

	listeners[i].is_registered = true;

	return ret;
}

int start_listener_services(void)
{
	int ret = 0;
	int32_t rv = Object_OK;
	size_t idx = 0;
	size_t n_listeners = sizeof(listeners)/sizeof(struct listener_svc);

	Object root = Object_NULL;
	Object client_env = Object_NULL;
	Object register_obj = Object_NULL;
	Object mo = Object_NULL;

	MSGD("Total listener services to start = %ld\n", n_listeners);

	for (idx = 0; idx < n_listeners; idx++) {

		/* Does the service define it's own registration callback? */
		if (listeners[idx].svc_register) {
			ret = reg_listener_svc(idx);
			if (ret) {
				MSGE("reg_listener_svc failed: 0x%x\n", rv);
				goto exit_release;
			}

			/* If yes, skip the default registration path */
			continue;
		}

		/* There are 4 threads for each callback. */
		rv = MinkCom_getRootEnvObject(&root);
		if (Object_isERROR(rv)) {
			MSGE("getRootEnvObject failed: 0x%x\n", rv);
			ret = -1;
			goto exit_release;
		}

		rv = MinkCom_getClientEnvObject(root, &client_env);
		if (Object_isERROR(rv)) {
			Object_ASSIGN_NULL(root);
			MSGE("getClientEnvObject failed: 0x%x\n", rv);
			ret = -1;
			goto exit_release;
		}

		rv = IClientEnv_open(client_env, CRegisterListenerCBO_UID,
				     &register_obj);
		if (Object_isERROR(rv)) {
			Object_ASSIGN_NULL(client_env);
			Object_ASSIGN_NULL(root);
			MSGE("IClientEnv_open failed: 0x%x\n", rv);
			ret = -1;
			goto exit_release;
		}

		Object_ASSIGN(obj_list[idx], register_obj);
		Object_ASSIGN_NULL(register_obj);

		rv = MinkCom_getMemoryObject(root, listeners[idx].buf_len,
					     &mo);
		if (Object_isERROR(rv)) {
			Object_ASSIGN_NULL(client_env);
			Object_ASSIGN_NULL(root);
			ret = -1;
			MSGE("getMemoryObject failed: 0x%x", rv);
			goto exit_release_obj;
		}

		/* Create CBO listener and register it */
		rv = CListenerCBO_new(&listeners[idx].cbo, mo,
				      &listeners[idx]);
		if (Object_isERROR(rv)) {
			Object_ASSIGN_NULL(mo);
			Object_ASSIGN_NULL(client_env);
			Object_ASSIGN_NULL(root);
			ret = -1;
			MSGE("CListenerCBO_new failed: 0x%x\n", rv);
			goto exit_release_obj;
		}

		rv = IRegisterListenerCBO_register(obj_list[idx],
						   listeners[idx].id,
						   listeners[idx].cbo,
						   mo);
		if (Object_isERROR(rv)) {
			Object_ASSIGN_NULL(mo);
			Object_ASSIGN_NULL(client_env);
			Object_ASSIGN_NULL(root);
			ret = -1;
			MSGE("IRegisterListenerCBO_register(%d) failed: 0x%x",
			     listeners[idx].id, rv);
			goto exit_release_cbo;
		}

		listeners[idx].is_registered = true;

		Object_ASSIGN_NULL(mo);
		Object_ASSIGN_NULL(client_env);
		Object_ASSIGN_NULL(root);
	}

	return ret;

exit_release_cbo:
	Object_ASSIGN_NULL(listeners[idx].cbo);

exit_release_obj:
	Object_ASSIGN_NULL(obj_list[idx]);

exit_release:
	stop_listeners_smci();

	return ret;
}
