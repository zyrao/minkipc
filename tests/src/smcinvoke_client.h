// Copyright (c) 2025, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _SMCINVOKE_CLIENT_H_
#define _SMCINVOKE_CLIENT_H_

#include <object.h>
#include <stdio.h>

enum {
	attr_uid = 1,
	attr_pkg_flags,
	attr_pkg_name,
	attr_pkg_cert,
	attr_permissions,
	attr_system_time,
};

#define CREDENTIALS_BUF_SIZE_INC 4096

enum test_types {
  CALLBACKOBJ,
  PRINT_TZ_DIAGNOSTICS,
};

struct option testopts[] = {
    {"callbackobj", no_argument, NULL, 'c'},
    {"diagnostics", no_argument, NULL, 'd'},
    {NULL, 0, NULL, 0},
};

#endif /* _SMCINVOKE_CLIENT_H_ */
