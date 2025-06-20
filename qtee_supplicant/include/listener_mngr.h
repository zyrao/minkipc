// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef __LISTENER_MNGR_H
#define __LISTENER_MNGR_H

#include <stdint.h>
#include <stdio.h>
#include "object.h"

#define MSGV printf
#define MSGD printf
#define MSGE printf

/* List of services buffer length */
#define FILE_SERVICE_BUF_LEN    (20*1024)
#define TIME_SERVICE_BUF_LEN    (20*1024)
#define GPFILE_SERVICE_BUF_LEN  (504*1024)
/* End of list */

/* List of services id's */
#define FILE_SERVICE    0xa
#define TIME_SERVICE    0xb
#define GPFILE_SERVICE  0x7000

typedef int32_t (*svc_register)(void);
typedef void (*svc_deregister)(void);

/**
 * @brief Listener service.
 *
 * Represents a listener service to be initialized and started by the QTEE
 * supplicant. Each listener service offers a specific REE service to QTEE,
 * e.g. time service.
 */
struct listener_svc {
	char *service_name; /**< Name of the listener service. */
	int id; /**< Id of the listener service. */
	int is_registered; /**< Listener registration status. */
	char *file_name; /**< File name of the listener service. */
	void *lib_handle; /**< LibHandle for the listener. */
	char *svc_register; /**< Listener service (opt) register callback. */
	char *svc_deregister; /**< Listener service (opt) deregister callback. */
	char *dispatch_func; /**< Dispatch function of the listener. */
	Object cbo; /**< Store cbo of listener service. */
	size_t buf_len; /**< Buffer length for listener service. */
};

/**
 * @brief Start listener services.
 *
 * Starts listener services which wait for a listener request from QTEE.
 */
int start_listener_services(void);

#endif // __LISTENER_MNGR_H
