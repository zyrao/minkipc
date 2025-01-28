// Copyright (c) 2025, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _MINKCOM_H_
#define _MINKCOM_H_

#include "object.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get a RootEnv object which can be used to initiate MINK-IPC based
 * communication with QTEE.
 *
 * @param obj The RootEnv object requested by the client.
 * @return Object_OK on success.
 *         Object_ERROR on failure.
 */
int MinkCom_getRootEnvObject(Object *obj);

/**
 * @brief Get a ClientEnv object that is registered with QTEE with client's credentials
 *
 * @param root: The RootEnv object for initiating MINK-IPC based communication.
 * @param obj: The ClientEnv object requested by the client.
 *
 * @return Object_OK on success.
 *         Object_ERROR_* on failure.
*/
int MinkCom_getClientEnvObject(Object root, Object *obj);

#ifdef __cplusplus
}
#endif

#endif // _MINKCOM_H_
