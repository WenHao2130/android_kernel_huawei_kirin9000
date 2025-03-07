/*
 *
 * (C) COPYRIGHT 2017 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wvisibility"
#ifndef _KBASE_IPA_DEBUGFS_H_
#define _KBASE_IPA_DEBUGFS_H_

enum kbase_ipa_model_param_type {
	PARAM_TYPE_S32 = 1,
	PARAM_TYPE_STRING,
};

#ifdef CONFIG_DFX_DEBUG_FS

void kbase_ipa_debugfs_init(struct kbase_device *kbdev);
int kbase_ipa_model_param_add(struct kbase_ipa_model *model, const char *name,
			      void *addr, size_t size,
			      enum kbase_ipa_model_param_type type);
void kbase_ipa_model_param_free_all(struct kbase_ipa_model *model);

/**
 * kbase_ipa_model_param_set_s32 - Set an integer model parameter
 *
 * @model:	pointer to IPA model
 * @name:	name of corresponding debugfs entry
 * @val:	new value of the parameter
 *
 * This function is only exposed for use by unit tests running in
 * kernel space. Normally it is expected that parameter values will
 * instead be set via debugfs.
 */
void kbase_ipa_model_param_set_s32(struct kbase_ipa_model *model,
	const char *name, s32 val);

#else /* CONFIG_DFX_DEBUG_FS */

static inline int kbase_ipa_model_param_add(struct kbase_ipa_model *model,
					    const char *name, void *addr,
					    size_t size,
					    enum kbase_ipa_model_param_type type)
{
	return 0;
}

static inline void kbase_ipa_model_param_free_all(struct kbase_ipa_model *model)
{ }

#endif /* CONFIG_DFX_DEBUG_FS */

#endif /* _KBASE_IPA_DEBUGFS_H_ */
#pragma GCC diagnostic pop
