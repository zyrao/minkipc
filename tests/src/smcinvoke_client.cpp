// Copyright (c) 2025, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "smcinvoke_client.h"

#include "CAppLoader.h"
#include "CDiagnostics.h"
#include "CTestCallable_open.h"
#include "CTestCallable_priv.h"
#include "IAppController.h"
#include "IAppLoader.h"
#include "IClientEnv.h"
#include "IDiagnostics.h"
#include "IOpener.h"
#include "ITestCBack.h"
#include "MinkCom.h"
#include "tzecotestapp_uids.h"
#include "tzt.h"

static void usage(void)
{
	printf("\n\n---------------------------------------------------------\n"
	       "Usage: smcinvoke_client -[OPTION] [ARGU_1] ...... [ARGU_n]\n\n"
	       "Runs the user space tests specified by option and arguments \n"
	       "parameter(s).\n"
	       "\n\n"
	       "OPTION can be:\n"
	       "  -c, Run tests to check callback objects in smcinvoke\n"
	       "      e.g. smcinvoke_vendor_client -c /data 1\n\n"
	       "  -d  Run the TZ diagnostics test that prints basic info on TZ heaps\n"
	       "      e.g. smcinvoke_vendor_client -d <no_of_iterations>\n"
	       "  -h, Print this help message and exit\n\n\n");
}

static void test_smcinvoke_cback_basic(Object appObj, Object clientEnv)
{
	Object oTCB = Object_NULL;
	Object oCB = Object_NULL;
	Object oCB1 = Object_NULL;
	TestCallable *cb = NULL;
	uint8_t bi[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	int ret = Object_OK;

	// Get the remote test object from the TA
	SILENT_OK(IOpener_open(appObj, CTzEcoTestApp_TestCBack_UID, &oTCB));

	// Get a local callable object
	SILENT_OK(CTestCallable_open(clientEnv, &oCB));
	cb = (TestCallable *)oCB.context;

	// Set the expected return and check preconditions
	SILENT_TRUE(cb->op == -1);
	SILENT_TRUE(cb->counter == 0);
	SILENT_TRUE(cb->refs == 1);
	cb->retValue = Object_OK;

	ret = ITestCBack_call(oTCB, oCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	// Check it happened as expected
	TEST_TRUE(ret == cb->retValue);
	TEST_TRUE(cb->counter == 1);
	TEST_TRUE(cb->op == ITestCallable_OP_call);

	// Repeat with BI
	cb->counter = 0;
	// Some unique error code which doesn't overlap with Mink error codes
	cb->retValueError = 0x0AFAFAFA;
	cb->bArg_ptr = bi;
	cb->bArg_len = sizeof(bi);

	ret = ITestCBack_callWithBuffer(oTCB, bi, sizeof(bi), oCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	TEST_TRUE(ret == cb->retValue);
	TEST_TRUE(cb->counter == 1);
	TEST_TRUE(cb->op == ITestCallable_OP_callWithBuffer);

	// And with a buffer that doesn't match
	cb->counter = 0;

	ret = ITestCBack_callWithBuffer(oTCB, bi, sizeof(bi) - 1, oCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	TEST_TRUE(ret == cb->retValueError);
	TEST_TRUE(cb->counter == 1);
	TEST_TRUE(cb->op == ITestCallable_OP_callWithBuffer);

	// With another callable object as argument
	SILENT_OK(CTestCallable_open(clientEnv, &oCB1));
	cb->counter = 0;
	Object_ASSIGN(cb->oArg, oCB1);
	ret = ITestCBack_callWithObject(oTCB, oCB1, oCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	TEST_TRUE(ret == cb->retValue);
	TEST_TRUE(cb->counter == 1);
	TEST_TRUE(cb->op == ITestCallable_OP_callWithObject);

	// Now with a remote object as argument
	cb->counter = 0;
	cb->op = -1;
	ret = ITestCBack_callWithObject(oTCB, oTCB, oCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	TEST_TRUE(ret = ITestCallable_ERROR_OBJECT_REMOTE);
	TEST_TRUE(cb->counter == 1);
	TEST_TRUE(cb->op == ITestCallable_OP_callWithObject);

	Object_RELEASE_IF(oCB1);

	// Now test the callback object is retained by TA
	cb->counter = 0;
	cb->op = -1;
	ret = ITestCBack_set(oTCB, oCB);
	TEST_OK(ret);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	TEST_TRUE(cb->counter == 0);
	TEST_TRUE(cb->op == -1);

	// Now call it
	ret = ITestCBack_callSet(oTCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	// Check it happened as expected
	TEST_TRUE(ret == cb->retValue);
	TEST_TRUE(cb->counter == 1);
	TEST_TRUE(cb->op == ITestCallable_OP_call);

	// Release it
	cb->counter = 0;
	cb->op = -1;
	ret = ITestCBack_set(oTCB, Object_NULL);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	TEST_OK(ret);
	TEST_TRUE(cb->counter == 0);
	TEST_TRUE(cb->op == -1);

	// Release callback object after set
	ret = ITestCBack_set(oTCB, oCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	// Release it
	Object_ASSIGN_NULL(oCB);
	ret = ITestCBack_callSet(oTCB);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	ret = ITestCBack_set(oTCB, Object_NULL);
	LOGD_PRINT("%s:%d: ret=0x%x counter=%zu op=%d refs=%d\n", __FUNCTION__,
		   __LINE__, ret, cb->counter, cb->op, cb->refs);

	// We are done!
	Object_ASSIGN_NULL(oTCB);
}

static int smcinvoke_setup_env(Object *clientEnv, Object *appLoader)
{
	Object rootEnv = Object_NULL;

	TEST_OK(MinkCom_getRootEnvObject(&rootEnv));
	TEST_OK(MinkCom_getClientEnvObject(rootEnv, clientEnv));
	SILENT_OK(IClientEnv_open(*clientEnv, CAppLoader_UID, appLoader));

	Object_ASSIGN_NULL(rootEnv);

	return 0;
}

static int __readFile(std::string const &filename, size_t &size,
		      uint8_t *buffer)
{
	FILE *file = NULL;
	size_t readBytes = 0;
	int ret = 0;

	do {
		file = fopen(filename.c_str(), "r");
		if (file == NULL) {
			LOGE_PRINT("Failed to open file %s: %s (%d)\n",
				   filename.c_str(), strerror(errno), errno);
			ret = -1;
			break;
		}

		readBytes = fread(buffer, 1, size, file);
		if (readBytes != size) {
			LOGE_PRINT(
				"Error reading the file %s: %zu vs %zu bytes: %s (%d)\n",
				filename.c_str(), readBytes, size,
				strerror(errno), errno);
			ret = -1;
			break;
		}

		ret = size;
	} while (0);

	if (file) {
		fclose(file);
	}

	return ret;
}

static int __getFileSize(std::string const &filename)
{
	FILE *file = NULL;
	int size = 0;
	int ret = 0;

	do {
		file = fopen(filename.c_str(), "r");
		if (file == NULL) {
			LOGE_PRINT("Failed to open file %s: %s (%d)\n",
				   filename.c_str(), strerror(errno), errno);
			size = -1;
			break;
		}

		ret = fseek(file, 0L, SEEK_END);
		if (ret) {
			LOGE_PRINT("Error seeking in file %s: %s (%d)\n",
				   filename.c_str(), strerror(errno), errno);
			size = -1;
			break;
		}

		size = ftell(file);
		if (size == -1) {
			LOGE_PRINT("Error telling size of file %s: %s (%d)\n",
				   filename.c_str(), strerror(errno), errno);
			size = -1;
			break;
		}
	} while (0);

	if (file) {
		fclose(file);
	}

	return size;
}

static int loadApp(Object appLoader, std::string const &path,
		   Object *appController, Object *appLegacy)
{
	size_t size = 0;
	uint8_t *buffer = NULL;
	int ret = 0;

	do {
		MSGD("Load %s", path.c_str());
		ret = __getFileSize(path);
		if (ret <= 0) {
			ret = -1;
			break;
		}

		size = (size_t)ret;
		buffer = new uint8_t[size];
		if (buffer == NULL) {
			LOGE_PRINT("Buffer allocation failed.\n");
			ret = -1;
			break;
		}

		ret = __readFile(path, size, buffer);
		if (ret < 0)
			break;
		MSGE("Load %s, size %zu, buf %p.\n", path.c_str(), size,
		     buffer);

		ret = IAppLoader_loadFromBuffer(appLoader, buffer, size,
						appController);
		if (ret) {
			LOGE_PRINT("Loading %s app failed, ret: %d\n",
				   path.c_str(), ret);
			break;
		}

		TEST_OK(IAppController_getAppObject(*appController, appLegacy));

	} while (0);

	if (buffer)
		delete[] buffer;

	return ret;
}

static int run_tzecotestapp_test(int argc, char *argv[], int flag)
{
	int32_t result = Object_OK;
	Object clientEnv = Object_NULL;
	Object appLoader = Object_NULL;
	Object appController = Object_NULL;
	Object appObj = Object_NULL;

	if (argc < 3) {
		usage();
		return -1;
	}

	std::string appFullPath = argv[2];
	size_t iterations = atoi(argv[3]);

	TEST_OK(smcinvoke_setup_env(&clientEnv, &appLoader));
	appFullPath.append("/").append("tzecotestapp.mbn");
	TEST_OK(loadApp(appLoader, appFullPath, &appController, &appObj));
	LOGD_PRINT("pass\n");

	for (size_t i = 0; i < iterations; i++) {
		switch (flag) {
		case CALLBACKOBJ:
			// Callback obj test
			test_smcinvoke_cback_basic(appObj, clientEnv);
			LOGD_PRINT(
				" test_smcinvoke_cback_basic iteration %zu finished\n",
				i);
			break;
		default:
			break;
		}
	}

	result = IAppController_unload(appController);
	if (result == 0)
		printf("Unload Successful\n");

	// Release the controller, thus also unloading the TA
	Object_release(appObj);
	Object_release(appController);

	Object_RELEASE_IF(appLoader);
	Object_RELEASE_IF(clientEnv);

	LOGD_PRINT("pass\n");

	return 0;
}

static int run_tz_diagnostics_test(int argc, char *argv[])
{
	Object rootEnv = Object_NULL;
	Object clientEnv = Object_NULL;
	Object appObject = Object_NULL;
	IDiagnostics_HeapInfo heapInfo;

	if (argc < 2) {
		usage();
		return -1;
	}

	int iterations = atoi(argv[2]);
	memset((void *)&heapInfo, 0, sizeof(IDiagnostics_HeapInfo));

	TEST_OK(MinkCom_getRootEnvObject(&rootEnv));
	TEST_OK(MinkCom_getClientEnvObject(rootEnv, &clientEnv));
	TEST_OK(IClientEnv_open(clientEnv, CDiagnostics_UID, &appObject));

	for (int i = 0; i < iterations; i++) {
		LOGE_PRINT("Retrieve TZ heap info Iteration %d\n", i);
		TEST_OK(IDiagnostics_queryHeapInfo(appObject, &heapInfo));

		LOGD_PRINT("%d = Total bytes as heap\n", heapInfo.totalSize);
		LOGD_PRINT("%d = Total bytes allocated from heap\n",
			   heapInfo.usedSize);
		LOGD_PRINT("%d = Total bytes free on heap\n",
			   heapInfo.freeSize);
		LOGD_PRINT("%d = Total bytes overhead\n",
			   heapInfo.overheadSize);
		LOGD_PRINT("%d = Total bytes wasted\n", heapInfo.wastedSize);
		LOGD_PRINT("%d = Largest free block size\n\n",
			   heapInfo.largestFreeBlockSize);

		LOGE_PRINT("Done!\n\n");
	}

	Object_ASSIGN_NULL(appObject);
	Object_ASSIGN_NULL(clientEnv);
	Object_ASSIGN_NULL(rootEnv);
	return 0;
}

static int run_smcinvoke_test_command(int argc, char *argv[],
				      unsigned int test_mask)
{
	if ((test_mask & (1 << CALLBACKOBJ)) == (1U << CALLBACKOBJ)) {
		printf("Run callback obj test...\n");
		return run_tzecotestapp_test(argc, argv, CALLBACKOBJ);
	} else if ((test_mask & (1 << PRINT_TZ_DIAGNOSTICS)) ==
		   (1U << PRINT_TZ_DIAGNOSTICS)) {
		printf("Run TZ Diagnostics and print those...\n");
		return run_tz_diagnostics_test(argc, argv);
	} else {
		usage();
		return -1;
	}
}

static unsigned int parse_command(int argc, char *const argv[])
{
	int command = 0;
	unsigned int ret = 0;

	while ((command = getopt_long(argc, argv, "cdh", testopts, NULL)) !=
	       -1) {
		printf("command is: %d\n", command);
		switch (command) {
		case 'c':
			ret = 1 << CALLBACKOBJ;
			break;
		case 'd':
			ret = 1 << PRINT_TZ_DIAGNOSTICS;
			break;
		case 'h':
			usage();
			break;
		default:
			usage();
		}

		if (command == (int)('q') || command == (int)('e'))
			break;
	}

	optind = 1;

	return ret;
}

int main(int argc, char *argv[])
{
	unsigned int test_mask = parse_command(argc, argv);
	return run_smcinvoke_test_command(argc, argv, test_mask);
}
