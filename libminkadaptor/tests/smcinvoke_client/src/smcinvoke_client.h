// Copyright (c) 2025, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _SMCINVOKE_CLIENT_H_
#define _SMCINVOKE_CLIENT_H_

#include <object.h>
#include <stdio.h>

#define SIZE_4KB 0x00001000

enum {
	attr_uid = 1,
	attr_pkg_flags,
	attr_pkg_name,
	attr_pkg_cert,
	attr_permissions,
	attr_system_time,
};

#define CREDENTIALS_BUF_SIZE_INC 4096

/* Private handle to memory shared with QTEE */
struct smcinvoke_priv_handle {
	void *addr;
	size_t size;
};

enum test_types {
	CALLBACKOBJ,
	MEMORYOBJ,
	PRINT_TZ_DIAGNOSTICS,
};

struct option testopts[] = {
	{"callbackobj", no_argument, NULL, 'c'},
	{"memoryobj", no_argument, NULL, 'm'},
	{"diagnostics", no_argument, NULL, 'd'},
	{NULL, 0, NULL, 0},
};

#endif /* _SMCINVOKE_CLIENT_H_ */
