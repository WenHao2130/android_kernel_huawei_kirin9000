/* Copyright (c) Hisilicon Technologies Co., Ltd. 2013-2019. All rights reserved.
 * FileName: ion_tee_op.c
 * Description: This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;
 * either version 2 of the License,
 * or (at your option) any later version.
 */

#define pr_fmt(fmt) "secsg: " fmt

#include <linux/err.h>
#include <linux/platform_drivers/mm_ion.h>
#include <linux/sizes.h>

#include "mm_ion_priv.h"
#include "ion.h"
#include "ion_tee_op.h"
#include "teek_client_api.h"
#include "teek_client_constants.h"
#include "teek_client_id.h"

#define ROOTID 2000
/* uuid to TA: f8028dca-aba0-11e6-80f5-76304dec7eb7 */
#define UUID_TEEOS_TZMP2_IONMENORYMANAGEMENT \
{ \
	0xf8028dca,\
	0xaba0,\
	0x11e6,\
	{ \
		0x80, 0xf5, 0x76, 0x30, 0x4d, 0xec, 0x7e, 0xb7 \
	} \
}

static DEFINE_MUTEX(g_ion_session_mutex);
int open_session_ta_init = 0;

/*
 * The maximum number of connections is 8,
 * using 9 ensures that every sessions can be occupied
 */
#define MAX_OPEN_SESSIONS_NUM 9
#define ION_SESSION_NAME_LEN 20
struct secmem_session_info {
	int is_init;
	int is_occupied;
	TEEC_Context context_secmem;
	TEEC_Session session_secmem;
};
struct secmem_session_info g_ion_session_info[MAX_OPEN_SESSIONS_NUM] = { 0 };

#ifdef CONFIG_MM_VLTMM
struct secmem_session_info g_ion_vltmm_session = {0};
#endif

int secmem_tee_init(TEEC_Context *context, TEEC_Session *session,
				const char *package_name)
{
	u32 root_id = ROOTID;
	TEEC_UUID svc_id = UUID_TEEOS_TZMP2_IONMENORYMANAGEMENT;
	TEEC_Operation op = {0};
	TEEC_Result result;
	u32 origin = 0;

	if (!context || !session) {
		pr_err("Invalid context or session\n");
		goto cleanup_1;
	}

	/* initialize TEE environment */
	result = TEEK_InitializeContext(NULL, context);
	if (result != TEEC_SUCCESS) {
		pr_err("InitializeContext failed, ReturnCode=0x%x\n", result);
		goto cleanup_1;
	} else {
		sec_debug("InitializeContext success\n");
	}
	/* operation params create  */
	op.started = 1;
	op.cancel_flag = 0;
	/* open session */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE,
			    TEEC_NONE,
			    TEEC_MEMREF_TEMP_INPUT,
			    TEEC_MEMREF_TEMP_INPUT);

	op.params[2].tmpref.buffer = (void *)&root_id;
	op.params[2].tmpref.size = sizeof(root_id);
	op.params[3].tmpref.buffer = (void *)package_name;
	op.params[3].tmpref.size = (size_t)(strlen(package_name) + 1);

	result = TEEK_OpenSession(context, session, &svc_id,
				  TEEC_LOGIN_IDENTIFY, NULL, &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("OpenSession fail, RC=0x%x, RO=0x%x\n", result, origin);
		goto cleanup_2;
	} else {
		sec_debug("OpenSession success\n");
	}

	return 0;
cleanup_2:
	TEEK_FinalizeContext(context);
cleanup_1:
	return -EINVAL;
}

static void check_tee_return(u32 cmd, struct mem_chunk_list *mcl,
				TEEC_Operation op)
{
	if (cmd == ION_SEC_CMD_ALLOC) {
		mcl->buff_id = op.params[1].value.b;
		sec_debug("TEE return secbuf id 0x%x\n", mcl->buff_id);
	}

	if (cmd == ION_SEC_CMD_MAP_IOMMU) {
		mcl->va = op.params[1].value.b;
		sec_debug("TEE return iova 0x%x\n", mcl->va);
	}

	if (cmd == ION_SEC_CMD_VLTMM) {
		mcl->nents = op.params[1].value.a;
		mcl->va = op.params[1].value.b;
		sec_debug("TEE return num 0x%x\n", mcl->nents);
	}
}

int secmem_tee_exec_cmd(TEEC_Session *session,
		       struct mem_chunk_list *mcl, u32 cmd)
{
	TEEC_Result result;
	TEEC_Operation op = {0};
	u32 protect_id = SEC_TASK_MAX;
	u32 origin = 0;

	if (!session || !mcl)
		return -EINVAL;

	protect_id = mcl->protect_id;

	op.started = 1;
	op.cancel_flag = 0;
	op.params[0].value.a = cmd;
	op.params[0].value.b = protect_id;

	switch (cmd) {
	case ION_SEC_CMD_ALLOC:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INOUT,
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_NONE);
		op.params[1].value.a = mcl->nents;
		/* op.params[1].value.b receive the return value */
		/* number of list in CMD buffer alloc/table set/table clean */
		op.params[2].tmpref.buffer = mcl->phys_addr;
		op.params[2].tmpref.size =
			mcl->nents * sizeof(struct tz_pageinfo);
		break;
	case ION_SEC_CMD_MAP_IOMMU:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INOUT,
			TEEC_NONE,
			TEEC_NONE);
		op.params[1].value.a = mcl->buff_id;
		op.params[1].value.b = mcl->size;
		break;
	case ION_SEC_CMD_FREE:
	case ION_SEC_CMD_UNMAP_IOMMU:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_NONE,
			TEEC_NONE);
		op.params[1].value.a = mcl->buff_id;
		break;
	case ION_SEC_CMD_TABLE_SET:
	case ION_SEC_CMD_TABLE_CLEAN:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_NONE);
		op.params[1].value.a = mcl->nents;
		op.params[2].tmpref.buffer = mcl->phys_addr;
		op.params[2].tmpref.size =
			mcl->nents * sizeof(struct tz_pageinfo);
		break;
	case ION_SEC_CMD_VLTMM:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INOUT,
			TEEC_MEMREF_TEMP_INOUT,
			TEEC_NONE);
		op.params[1].value.a = mcl->nents;
		op.params[1].value.b = mcl->va;
		op.params[2].tmpref.buffer = mcl->phys_addr;
		op.params[2].tmpref.size = mcl->size;
		break;
#ifdef CONFIG_SECMEM_TEST
	case ION_SEC_CMD_TEST:
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_MEMREF_TEMP_INPUT,
			TEEC_NONE);
		op.params[1].value.a = mcl->buff_id;
		op.params[1].value.b = mcl->size;
		op.params[2].tmpref.buffer = mcl->phys_addr;
		op.params[2].tmpref.size =
			mcl->nents * sizeof(struct tz_pageinfo);
		break;
#endif
	default:
		pr_err("Invalid cmd\n");
		return -EINVAL;
	}

	result = TEEK_InvokeCommand(session, SECBOOT_CMD_ID_MEM_ALLOCATE,
				    &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("Invoke CMD fail, RC=0x%x, RO=0x%x\n", result, origin);
		return -EFAULT;
	}

	sec_debug("Exec TEE CMD success.\n");
	check_tee_return(cmd, mcl, op);

	return 0;
}

void secmem_tee_destroy(TEEC_Context *context, TEEC_Session *session)
{
	if (!context || !session) {
		pr_err("Invalid context or session\n");
		return;
	}

	TEEK_CloseSession(session);
	TEEK_FinalizeContext(context);
	sec_debug("TA closed !\n");
}

static inline void __open_ion_session(int *ta_init, const char *ta_name,
	TEEC_Context *context, TEEC_Session *session, const char *package_name)
{
	int ret;

	if (!(*ta_init)) {
		ret = secmem_tee_init(context, session, package_name);
		if (ret)
			pr_info("%s %s TA session init failed\n", __func__, ta_name);
		else
			*ta_init = 1;
	}
	pr_info("%s %s TA session init done\n", __func__, ta_name);
}

static int open_ion_sessions(void)
{
	int ret;
	int i;

	mutex_lock(&g_ion_session_mutex);
	if (!open_session_ta_init) {
#ifdef CONFIG_MM_VLTMM
		__open_ion_session(&g_ion_vltmm_session.is_init, "vltmm",
			&g_ion_vltmm_session.context_secmem,
			&g_ion_vltmm_session.session_secmem,
			TEE_VLTMM_NAME);
#endif
		for (i = 0; i < MAX_OPEN_SESSIONS_NUM; i++) {
			char name[ION_SESSION_NAME_LEN] = {0};

			ret = snprintf_s(name, ION_SESSION_NAME_LEN,
				ION_SESSION_NAME_LEN - 1,
				"secmem-%d", i);
			if (ret < 0)
				continue;

			__open_ion_session(&g_ion_session_info[i].is_init, name,
				&g_ion_session_info[i].context_secmem,
				&g_ion_session_info[i].session_secmem,
				TEE_SECMEM_NAME);
		}

		open_session_ta_init = 1;
	}
	mutex_unlock(&g_ion_session_mutex);

	return 0;
}
device_initcall(open_ion_sessions);

static inline int __sec_tee_get_session(int ta_is_init, int *is_occupied,
	struct ion_sec_tee *tee, TEEC_Context *context, TEEC_Session *session)
{
	if ((ta_is_init == 0) || (*is_occupied))
		return -EINVAL;

	tee->context = context;
	tee->session = session;
	*is_occupied = 1;
	return 0;
}

int sec_tee_init(struct ion_sec_tee *tee, enum ion_sessions_type type)
{
	int ret = -EINVAL;
	int i;

	(void)open_ion_sessions();

	mutex_lock(&g_ion_session_mutex);
	switch (type) {
	case ION_SESSIONS_SECMEM:
		for (i = 0; i < MAX_OPEN_SESSIONS_NUM; i++) {
			ret = __sec_tee_get_session(g_ion_session_info[i].is_init,
				&g_ion_session_info[i].is_occupied, tee,
				&g_ion_session_info[i].context_secmem,
				&g_ion_session_info[i].session_secmem);
			if (ret)
				continue;
			break;
		}
		break;
#ifdef CONFIG_MM_VLTMM
	case ION_SESSIONS_VLTMM:
		ret = __sec_tee_get_session(g_ion_vltmm_session.is_init,
			&g_ion_vltmm_session.is_occupied, tee,
			&g_ion_vltmm_session.context_secmem,
			&g_ion_vltmm_session.session_secmem);
		break;
#endif
	default:
		pr_err("%s error session typs-%d\n", __func__, type);
		break;
	}
	mutex_unlock(&g_ion_session_mutex);

	pr_info("%s sessions_type is %d, ret-%d\n", __func__, type, ret);
	return ret;
}

int sec_tee_alloc(struct ion_sec_tee *tee)
{
	if (!tee) {
		pr_err("[%s]:tee is NULL!", __func__);
		return -EINVAL;
	}

	tee->context = kzalloc(sizeof(TEEC_Context), GFP_KERNEL);
	if (!tee->context)
		return -ENOMEM;

	tee->session = kzalloc(sizeof(TEEC_Session), GFP_KERNEL);
	if (!tee->session) {
		kfree(tee->context);
		tee->context = NULL;
		return -ENOMEM;
	}

	return 0;
}

void sec_tee_free(struct ion_sec_tee *tee)
{
	if (tee->session) {
		kfree(tee->session);
		tee->session = NULL;
	}
	if (tee->context) {
		kfree(tee->context);
		tee->context = NULL;
	}
}
