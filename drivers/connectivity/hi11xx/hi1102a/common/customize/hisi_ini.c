

#include "hisi_ini.h"

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/of.h>

#include "nve_info_interface.h"

#include "board.h"
#include "hisi_ini.h"
#include "oam_ext_if.h"
#include "oal_util.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HISI_INI_C

/* Global Variable Definition */
#define PROC_NAME_INI_FILE_NAME "ini_file_name"
#define CUST_PATH_INI_CONN "/data/vendor/cust_conn/ini_cfg" /* 某运营商在不同产品的差异配置 */
/* mutex for open ini file */
struct mutex g_file_mutex;
int8 g_ini_file_name[INI_FILE_PATH_LEN] = {0};
static int8 g_ini_conn_file_name[INI_FILE_PATH_LEN] = {0};
#define INI_FILE_PATH g_ini_file_name

ini_board_version_stru g_board_version = {{0}};
ini_param_version_stru g_param_version = {{0}};
int64 g_ini_file_time_sec = -1;

uint8 g_ini_i3c_switch = INI_NO_I3C;

/*
 *  Prototype    : ko_read_line
 *  Description  : read one non-empty line .ini file
 */
static int32 ko_read_line(ini_file *fp, char *addr, int32 buf_len)
{
    int32 l_ret;
    int8 auc_tmp[MAX_READ_LINE_NUM] = {0};
    int32 cnt;
    int32 cnt_limit;

    l_ret = oal_file_read_ext(fp, fp->f_pos, auc_tmp, MAX_READ_LINE_NUM);
    if (l_ret < 0) {
        ini_error("kernel_line read l_ret < 0");
        return INI_FAILED;
    } else if (l_ret == 0) {
        /* end of file */
        return 0;
    }

    cnt = 0;

    // -2为预留换行符和结束符
    cnt_limit = (buf_len - 2) > MAX_READ_LINE_NUM ? MAX_READ_LINE_NUM : (buf_len - 2);
    while ((cnt < cnt_limit) && (auc_tmp[cnt] != '\n')) {
        *addr++ = auc_tmp[cnt++];
    }

    *addr++ = '\n';
    *addr = '\0';

    /* change file pos to next line */
    fp->f_pos += (cnt + 1);

    return l_ret;
}

/*
 *  Prototype    : ini_file_open
 *  Description  : open *.ini file
 */
static ini_file *ini_file_open(int8 *filename, int8 *para)
{
    mm_segment_t fs;
    ini_file *fp = NULL;

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(filename, O_RDONLY, 0);
    set_fs(fs);
    if (IS_ERR_OR_NULL(fp)) {
        fp = NULL;
    }

    return fp;
}

/*
 * Prototype    : ini_file_close
 * Description  : close *.ini file
 */
static void ini_file_close(ini_file *fp)
{
    mm_segment_t fs;

    fs = get_fs();
    set_fs(KERNEL_DS);
    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;
}

/*
 * Prototype    : ini_file_exist
 * Description  : check file exist or not
 * Input        : file_path ptr
 */
static bool ini_file_exist(int8 *file_path)
{
    ini_file *fp = NULL;

    if (file_path == NULL) {
        ini_error("para file_path is NULL\n");
        return false;
    }

    fp = ini_file_open(file_path, "rt");
    if (fp == NULL) {
        ini_debug("%s not exist\n", file_path);
        return false;
    }

    ini_file_close(fp);

    ini_debug("%s exist\n", file_path);

    return true;
}

/*
 * Prototype    : ini_file_seek
 * Description  : set f_pos in curr file ptr
 * Input        : ini_file *fp, int16 fp_pos
 */
static int32 ini_file_seek(ini_file *fp, long fp_pos)
{
    fp->f_pos += fp_pos;
    return INI_SUCC;
}

/*
 * Prototype    : ini_readline_func
 * Description  : set f_pos in curr file ptr
 * Input        : ini_file *fp, int16 fp_pos
 */
static int32 ini_readline_func(ini_file *fp, int8 *rd_buf, uint32 buf_len)
{
    int8 auc_tmp[MAX_READ_LINE_NUM];
    int32 l_ret;

    memset_s(auc_tmp, sizeof(auc_tmp), 0, sizeof(auc_tmp));
    l_ret = ko_read_line(fp, auc_tmp, sizeof(auc_tmp));
    if (l_ret == INI_FAILED) {
        ini_error("ko_read_line failed!!!");
        return INI_FAILED;
    } else if (l_ret == 0) {
        ini_error("end of .ini file!!!");
        return INI_FAILED;
    }

    l_ret = strcpy_s(rd_buf, buf_len, auc_tmp);
    if (l_ret != EOK) {
        ini_error("ini_readline_func strcpy_s failed. please check!\n");
        return INI_FAILED;
    }

    return INI_SUCC;
}

/*
 * Prototype    : ini_check_str
 * Description  : check search str is of not ok
 * Input        : fp, int8 * auc_tmp int8 * puc_value
 */
int32 ini_check_str(ini_file *fp, int8 *auc_tmp, uint32 buf_len, const char *puc_var)
{
    uint16 auc_len;
    uint16 curr_var_len;
    uint16 search_var_len;

    if ((fp == NULL) || (puc_var == NULL) || (puc_var[0] == '\0')) {
        ini_error("check if puc_var is NULL or blank");
        return INI_FAILED;
    }

    do {
        auc_len = strlen(auc_tmp);
        curr_var_len = 0;

        while ((curr_var_len < buf_len) &&
               (auc_tmp[curr_var_len] != '\r') &&
               (auc_tmp[curr_var_len] != '\n') &&
               (auc_tmp[curr_var_len] != 0)) {
            curr_var_len++;
        }

        if ((auc_tmp[0] == '#') || (auc_tmp[0] == ' ') || (auc_tmp[0] == '\n') || (auc_tmp[0] == '\r')) {
            break;
        }
        search_var_len = strlen(puc_var);
        if (search_var_len > curr_var_len) {
            search_var_len = curr_var_len;
        }
        if (strncmp(auc_tmp, puc_var, search_var_len) == 0) {
            return INI_SUCC;
        } else {
            break;
        }
    } while (0);

    if (ini_file_seek(fp, -auc_len) == INI_FAILED) {
        ini_error("file seek failed!!!");
        return INI_FAILED;
    }
    if (ini_file_seek(fp, curr_var_len + 1) == INI_FAILED) {
        ini_error("file seek failed!!!");
        return INI_FAILED;
    }
    if (auc_tmp[curr_var_len + 1] == '\n') {
        if (ini_file_seek(fp, 1) == INI_FAILED) {
            ini_error("file seek failed!!!");
            return INI_FAILED;
        }
    }
    return INI_FAILED;
}

/*
 * Prototype    : ini_check_value
 * Description  : check the value of config behind =
 */
static int32 ini_check_value(int8 *puc_value, uint32 value_len)
{
    const uint32 ul_value_min_len = 2;

    if (value_len < ul_value_min_len) {
        ini_error("ini_check_value fail, puc_value length %u < 2(min len)\n", value_len);
        return INI_FAILED;
    }

    if (puc_value[0] == ' ' || puc_value[value_len - 1] == ' ' || puc_value[0] == '\n') {
        puc_value[0] = '\0';
        ini_error("::%s has blank space or is blank::", puc_value);
        return INI_FAILED;
    }

    /* check \n of line，如果是以'\n'结尾，需要把'\n'替换成'\0' */
    if (puc_value[value_len - 1] == '\n') {
        puc_value[value_len - 1] = '\0';
    }

    /* check \r of line，如果是以"\r\n"结尾，需要把'\r'替换成'\0' */
    if (puc_value[value_len - 2] == '\r') {
        puc_value[value_len - 2] = '\0';
    }

    return INI_SUCC;
}

static int32 is_modu_exist(ini_file *fp, const char *modu_name)
{
    int32 ret = INI_FAILED;
    int8 auc_tmp[MAX_READ_LINE_NUM] = {0};
    /*
     * find the value of mode var, such as ini_wifi_mode
     * every mode except PLAT mode has only one mode var
     */
    for (;;) {
        ret = ini_readline_func(fp, auc_tmp, sizeof(auc_tmp));
        if (ret == INI_FAILED) {
            ini_error("have end of .ini file!!!");
            return INI_FAILED;
        }

        if (strstr(auc_tmp, INI_STR_DEVICE_BFG_PLAT) != NULL) {
            ini_error("not find %s!!!", modu_name);
            return INI_FAILED;
        }

        ret = ini_check_str(fp, auc_tmp, sizeof(auc_tmp), modu_name);
        if (ret == INI_SUCC) {
            ini_debug("have found %s", modu_name);
            break;
        } else {
            continue;
        }
    }

    return ret;
}

/*
 * Prototype    : ini_find_modu
 * Description  : find moduler by mode value puc_value
 * Input        : ini_file *fp, int32 modu, int8 * puc_var, int8 *puc_value
 */
static int32 ini_find_modu(ini_file *fp, int32 tag_index, int8 *puc_var, int8 *puc_value)
{
    int8 auc_modu[INI_STR_MODU_LEN] = {0};
    int32 ret = INI_FAILED;

    unref_param(puc_var);
    unref_param(puc_value);

    switch (tag_index) {
        case INI_MODU_WIFI:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_WIFI_NORMAL);
            break;
        case INI_MODU_POWER_FCC:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_POWER_FCC);
            break;
        case INI_MODU_POWER_ETSI:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_POWER_ETSI);
            break;
        case INI_MODU_POWER_JP:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_POWER_JP);
            break;
        case INI_MODU_GNSS:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_GNSS_NORMAL);
            break;
        case INI_MODU_BT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_BT_NORMAL);
            break;
        case INI_MODU_FM:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_FM_NORMAL);
            break;
        case INI_MODU_PLAT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_PLAT);
            break;
        case INI_MODU_HOST_VERSION:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INT_STR_HOST_VERSION);
            break;
        case INI_MODU_WIFI_MAC:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_WIFI_MAC);
            break;
        case INI_MODU_COEXIST:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_COEXIST);
            break;
        case INI_MODU_DEV_WIFI:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_WIFI);
            break;
        case INI_MODU_DEV_GNSS:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_GNSS);
            break;
        case INI_MODU_DEV_BT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_BT);
            break;
        case INI_MODU_DEV_FM:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_FM);
            break;
        case INI_MODU_DEV_BFG_PLAT:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_DEVICE_BFG_PLAT);
            break;
        case INI_MODU_HOST_IR:
            ret = strcpy_s(auc_modu, sizeof(auc_modu), INI_STR_HOST_IR);
            break;
        default:
            ini_error("not suport tag type:%x!!!", tag_index);
            break;
    }
    if (ret != EOK) {
        oam_error_log2(0, OAM_SF_INI, "ini_find_modu:find failed tag type:%x!!! errno:%d", tag_index, ret);
        return INI_FAILED;
    }

    return is_modu_exist(fp, auc_modu);
}

/*
 * Prototype    : ini_find_var
 * Description  : find difference mode variable value, and return puc_value
 * Input        : ini_file *fp, int32 modu, int8 * puc_var, int8 *puc_value
 */
static int32 ini_find_var(ini_file *fp, int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    int32 ret;
    int8 auc_tmp[MAX_READ_LINE_NUM] = {0};
    size_t search_var_len;

    /* find the modu of var, such as [HOST_WIFI_NORMAL] of wifi moduler */
    ret = ini_find_modu(fp, tag_index, puc_var, puc_value);
    if (ret == INI_FAILED) {
        return INI_FAILED;
    }

    /* find the var in modu, such as [HOST_WIFI_NORMAL] of wifi moduler */
    while (1) {
        ret = ini_readline_func(fp, auc_tmp, sizeof(auc_tmp));
        if (ret == INI_FAILED) {
            ini_error("have end of .ini file!!!");
            return INI_FAILED;
        }

        if (auc_tmp[0] == '[') {
            ini_error("not find %s!!!, check if var in correct mode", puc_var);
            return INI_FAILED;
        }

        search_var_len = strlen(puc_var);
        ret = ini_check_str(fp, auc_tmp, sizeof(auc_tmp), puc_var);
        if ((ret == INI_SUCC) && (auc_tmp[search_var_len] == '=')) {
            if (strncpy_s(puc_value, size, &auc_tmp[search_var_len + 1], size - 1)) {
                OAM_ERROR_LOG0(0, OAM_SF_INI, "ini_find_var:strncpy_s space is not enough");
                continue;
            }
            break;
        } else {
            continue;
        }
    }

    return INI_SUCC;
}

/*
 * Prototype    : print_device_version
 * Description  : print device version info form ini file.
 */
void print_device_version(void)
{
    ini_file *fp = NULL;
    int8 version_buff[INI_VERSION_STR_LEN] = {0};
    int32 l_ret;

    ini_mutex_lock(&g_file_mutex);

    fp = ini_file_open(INI_FILE_PATH, "rt");
    if (fp == 0) {
        fp = NULL;
        ini_error("open %s failed!!!", INI_FILE_PATH);
        goto open_ini_file_fail;
    }

    l_ret = ini_find_var(fp, INI_MODU_PLAT, INI_VAR_PLAT_BOARD, version_buff, sizeof(version_buff));
    if (l_ret == INI_FAILED) {
        version_buff[0] = '\0';
        goto read_ini_var_fail;
    }

    if (ini_check_value(version_buff, strlen(version_buff)) == INI_FAILED) {
        goto read_ini_var_fail;
    }

    l_ret = strcpy_s(g_board_version.board_version, sizeof(g_board_version.board_version), version_buff);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_INI, "print_device_version:board version strcpy_s failed!");
        goto read_ini_var_fail;
    }

    ini_info("::g_board_version.board_version = %s::", g_board_version.board_version);
    fp->f_pos = 0;

    l_ret = ini_find_var(fp, INI_MODU_PLAT, INI_VAR_PLAT_PARAM, version_buff, sizeof(version_buff));
    if (l_ret == INI_FAILED) {
        version_buff[0] = '\0';
        goto read_ini_var_fail;
    }

    if (ini_check_value(version_buff, strlen(version_buff)) == INI_FAILED) {
        goto read_ini_var_fail;
    }

    l_ret = strcpy_s(g_param_version.param_version, sizeof(g_param_version.param_version), version_buff);
    if (l_ret != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_INI, "print_device_version:param version strcpy_s failed!");
        goto read_ini_var_fail;
    }

    ini_info("::g_param_version.param_version = %s::", g_param_version.param_version);
    fp->f_pos = 0;
    ini_file_close(fp);
    ini_mutex_unlock(&g_file_mutex);

    return;

read_ini_var_fail:
    fp->f_pos = 0;
    ini_file_close(fp);
open_ini_file_fail:
    ini_mutex_unlock(&g_file_mutex);
    return;
}

int32 find_download_channel(uint8 *buff, int8 *puc_var)
{
    ini_file *fp = NULL;
    int8 version_buff[DOWNLOAD_CHANNEL_LEN] = {0};
    int32 l_ret;

    ini_mutex_lock(&g_file_mutex);
    ini_info("ini file_name is %s", INI_FILE_PATH);
    fp = ini_file_open(INI_FILE_PATH, "rt");
    if (fp == 0) {
        fp = NULL;
        ini_error("open %s failed!!!", INI_FILE_PATH);
        goto open_ini_file_fail;
    }

    /* find wlan download channel */
    l_ret = ini_find_var(fp, INI_MODU_PLAT, puc_var, version_buff, DOWNLOAD_CHANNEL_LEN);
    if (l_ret == INI_FAILED) {
        version_buff[0] = '\0';
        goto read_ini_var_fail;
    }
    if (ini_check_value(version_buff, strlen(version_buff)) == INI_FAILED) {
        goto read_ini_var_fail;
    }
    if (strcpy_s((int8 *)buff, DOWNLOAD_CHANNEL_LEN, version_buff) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_INI, "find_download_channel:strcpy_s space is not enough");
        goto read_ini_var_fail;
    }
    buff[DOWNLOAD_CHANNEL_LEN - 1] = '\0';

    fp->f_pos = 0;
    ini_file_close(fp);
    ini_mutex_unlock(&g_file_mutex);
    return INI_SUCC;

read_ini_var_fail:
    fp->f_pos = 0;
    ini_file_close(fp);
open_ini_file_fail:
    ini_mutex_unlock(&g_file_mutex);
    return INI_FAILED;
}

int32 ini_find_var_value_by_path(int8 *path, int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    ini_file *fp = NULL;

#ifdef INI_TIME_TEST
    oal_timespec_stru tv[INI_TIMEVAL_NUM];
    oal_timespec_stru *tv_ptr = NULL;
#endif

    int32 l_ret;

    if ((puc_var == NULL) || puc_var[0] == '\0' || (puc_value == NULL)) {
        ini_error("check if puc_var and puc_value is NULL or blank");
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    tv_ptr = &tv[0];
    oal_do_gettimeofday(tv_ptr);
#endif

    ini_mutex_lock(&g_file_mutex);

    fp = ini_file_open(path, "rt");
    if (fp == 0) {
        fp = NULL;
        ini_error("open %s failed!!!", path);
        ini_mutex_unlock(&g_file_mutex);
        return INI_FAILED;
    }

    /* find puc_var in .ini return puc_value */
    l_ret = ini_find_var(fp, tag_index, puc_var, puc_value, size);
    if (l_ret == INI_FAILED) {
        puc_value[0] = '\0';
        ini_file_close(fp);
        ini_mutex_unlock(&g_file_mutex);
        return INI_FAILED;
    }

#ifdef INI_TIME_TEST
    tv_ptr = &tv[1];
    oal_do_gettimeofday(tv_ptr);
    ini_debug("time take = %llu",
        (tv[1].tv_sec - tv[0].tv_sec) * MSEC_PER_SEC + (oal_get_usec(&tv[1]) - oal_get_usec(&tv[0])) / USEC_PER_MSEC);
#endif

    ini_file_close(fp);
    ini_mutex_unlock(&g_file_mutex);

    /* check blank space of puc_value */
    if (ini_check_value(puc_value, strlen(puc_value)) == INI_SUCC) {
        ini_debug("::return %s:%s::", puc_var, puc_value);
        return INI_SUCC;
    }

    return INI_FAILED;
}

/*
 * Prototype    : ini_find_var_value
 * Description  : get var value from .ini file
 */
int32 ini_find_var_value(int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    /* read spec if exist */
    if (ini_file_exist(g_ini_conn_file_name)) {
        if (ini_find_var_value_by_path(g_ini_conn_file_name, tag_index, puc_var, puc_value, size) == INI_SUCC) {
            return INI_SUCC;
        }
    }

    if (ini_file_exist(INI_FILE_PATH) == 0) {
        ini_error(" %s not exist!!!", INI_FILE_PATH);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_INI, CHR_PLAT_DRV_ERROR_INI_MISS);
        return INI_FAILED;
    }

    return ini_find_var_value_by_path(INI_FILE_PATH, tag_index, puc_var, puc_value, size);
}

uint8 get_i3c_switch_mode(void)
{
    return g_ini_i3c_switch;
}
oal_module_symbol(get_i3c_switch_mode);

void set_i3c_switch_mode(void)
{
    g_ini_i3c_switch = !!ini_read_i3c_switch();
    ini_info("i3c_mode is %d\n", g_ini_i3c_switch);
}

uint8 ini_read_i3c_switch(void)
{
    ini_file *pst_fp = NULL;
    int32 l_ret;
    uint8 uc_i3c_switch;
    uint8 auc_i3c_switch[INI_I3C_SWITCH_MODE_LEN] = {0};

    ini_mutex_lock(&g_file_mutex);

    pst_fp = ini_file_open(INI_FILE_PATH, "rt");
    if (pst_fp == 0) {
        pst_fp = NULL;
        ini_error("open %s failed!!!", INI_FILE_PATH);
        goto open_ini_file_fail;
    }

    l_ret = ini_find_var(pst_fp, INI_MODU_PLAT, INI_I3C_SWITCH, auc_i3c_switch, sizeof(auc_i3c_switch));
    if (l_ret == INI_FAILED) {
        uc_i3c_switch = 0;
        goto read_ini_var_fail;
    }
    /* 读出来的是字符串，需要转换成十进制数字 */
    auc_i3c_switch[INI_I3C_SWITCH_MODE_LEN - 1] = '\0';
    uc_i3c_switch = (uint8)oal_atoi(auc_i3c_switch);

    pst_fp->f_pos = 0;
    ini_file_close(pst_fp);
    ini_mutex_unlock(&g_file_mutex);

    return uc_i3c_switch;

read_ini_var_fail:
    pst_fp->f_pos = 0;
    ini_file_close(pst_fp);
open_ini_file_fail:
    ini_mutex_unlock(&g_file_mutex);
    uc_i3c_switch = 0;

    return uc_i3c_switch;
}

int32 get_ini_file_name_from_dts(int8 *dts_prop, int8 *prop_value, uint32 size)
{
    int32 ret;
    struct device_node *np;
    int32 len;
    int8 out_str[HISI_CUST_NVRAM_LEN] = {0};

    np = of_find_compatible_node(NULL, NULL, COST_HI110X_COMP_NODE);
    if (np == NULL) {
        ini_error("dts node %s not found", COST_HI110X_COMP_NODE);
        return INI_FAILED;
    }

    len = of_property_count_u8_elems(np, dts_prop);
    if (len < 0) {
        ini_error("can't get len of dts prop(%s)", dts_prop);
        return INI_FAILED;
    }

    len = ini_min(len, (int32)sizeof(out_str));
    ini_debug("read len of dts prop %s is:%d", dts_prop, len);
    ret = of_property_read_u8_array(np, dts_prop, out_str, len);
    if (ret < 0) {
        ini_error("read dts prop (%s) fail", dts_prop);
        return INI_FAILED;
    }

    len = ini_min(len, (int32)size);
    ret = memcpy_s(prop_value, (size_t)size, out_str, (size_t)len);
    if (ret != EOK) {
        oam_error_log2(0, OAM_SF_INI, "get_ini_file_name_from_dts:memcpy_s error, destlen=%d, srclen=%d", size, len);
        return INI_FAILED;
    }
    ini_debug("dts prop %s value is:%s", dts_prop, prop_value);

    return INI_SUCC;
}

/*
 * Prototype    : get_ini_file
 * Description  : get *.ini file
 */
static int32 get_ini_file(int8 *file_path, ini_file **fp)
{
    if (file_path == NULL) {
        ini_info("para file_path is NULL\n");
        return INI_FAILED;
    }

    *fp = ini_file_open(file_path, "rt");
    if (*fp == NULL) {
        ini_info("inifile %s not exist\n", file_path);
        return INI_FAILED;
    }

    return INI_SUCC;
}

/*
 * Prototype    : ini_file_check_timespec
 * Description  : get *.ini file timespec
 */
static uint32 ini_file_check_timespec(ini_file *fp)
{
    if (fp == NULL) {
        ini_error("para file is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (fp->f_path.dentry == NULL) {
        ini_error("file dentry is NULL\n");
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (g_ini_file_time_sec != inf_file_get_ctime(fp->f_path.dentry)) {
        ini_info("ini_file time_secs changed from [%ld]to[%ld]\n",
                 g_ini_file_time_sec, inf_file_get_ctime(fp->f_path.dentry));
        g_ini_file_time_sec = inf_file_get_ctime(fp->f_path.dentry);

        return INI_FILE_TIMESPEC_RECONFIG;
    } else {
        ini_info("ini file is not upadted time_secs[%ld]\n", g_ini_file_time_sec);
        return INI_FILE_TIMESPEC_UNRECONFIG;
    }
}

/*
 * Prototype    : ini_file_check_conf_update
 * Description  : check *.ini file is updated or not
 */
int32 ini_file_check_conf_update(void)
{
    ini_file *fp = NULL;
    int32 ret;

    /* read spec if exist */
    if ((get_ini_file(g_ini_conn_file_name, &fp) == INI_SUCC) &&
             (ini_file_check_timespec(fp) == INI_FILE_TIMESPEC_RECONFIG)) {
        ini_info("%s ini file is updated\n", g_ini_conn_file_name);
        ret = (int32)INI_FILE_TIMESPEC_RECONFIG;
    } else if ((get_ini_file(INI_FILE_PATH, &fp) == INI_SUCC) &&
               (ini_file_check_timespec(fp) == INI_FILE_TIMESPEC_RECONFIG)) {
        ini_info("%s ini file is updated\n", INI_FILE_PATH);
        ret = (int32)INI_FILE_TIMESPEC_RECONFIG;
    } else {
        ini_info("no ini file is updated\n");
        ret = (int32)INI_FILE_TIMESPEC_UNRECONFIG;
    }

    if (fp != NULL) {
        ini_file_close(fp);
    }

    return ret;
}
oal_module_symbol(ini_file_check_conf_update);

/*
 * Prototype    : read_conf_from_nvram
 * Description  : read nv buff from nvram
 * Input        : pc_out, pointer to save key value
 * Return Value : 0 on success,or -1 on failed.
 */
int32 read_conf_from_nvram(uint8 *pc_out, uint32 size, uint32 nv_number, const char *nv_name)
{
    struct external_nve_info_user info;
    int32 ret;

    memset_s(&info, sizeof(info), 0, sizeof(info));
    memset_s(pc_out, size, 0, size);
    info.valid_size = HISI_CUST_NVRAM_LEN;
    info.nv_operation = HISI_CUST_NVRAM_READ;
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), nv_name) != EOK) {
        ini_error("read nvm failed nv_name size[%lu] less than input[%s]", sizeof(info.nv_name), nv_name);
        OAM_ERROR_LOG1(0, OAM_SF_INI, "read_conf_from_nvram:read nvm failed nv_name size[%lu] less than input",
                       sizeof(info.nv_name));
        return INI_FAILED;
    }
    info.nv_number = nv_number;

    ret = external_nve_direct_access_interface(&info);
    if (size <= OAL_STRLEN(info.nv_data)) {
        ini_error("read nvm{%s}lenth[%d] longer than input[%d]", info.nv_data, (uint32)OAL_STRLEN(info.nv_data), size);
        return INI_FAILED;
    }
    if (ret == INI_SUCC) {
        if (memcpy_s(pc_out, size, info.nv_data, sizeof(info.nv_data)) != EOK) {
            ini_error("read nvm{%s}lenth[%d] longer than input[%d]",
                      info.nv_data, (uint32)OAL_STRLEN(info.nv_data), size);
            oam_error_log2(0, OAM_SF_INI, "read_conf_from_nvram:read nvm lenth[%d] longer than input[%d]",
                           (uint32)OAL_STRLEN(info.nv_data), size);
            return INI_FAILED;
        }
        OAL_IO_PRINT("read_conf_from_nvram::nvram id[%d] nv name[%s] get data{%s}, size[%d]\r\n!",
                     nv_number, nv_name, info.nv_data, size);
    } else {
        ini_error("read nvm [%d] %s failed", nv_number, nv_name);
        return INI_FAILED;
    }

    return INI_SUCC;
}
oal_module_symbol(read_conf_from_nvram);

/*
 * Prototype    : write_conf_to_nvram
 * Description  : change value and write to file
 * Input        : key_name, new key_value
 * Return Value : 0 on success, or -1 on failed
 */
int32 write_conf_to_nvram(int8 *name, int8 *pc_arr)
{
    struct external_nve_info_user info;
    int32 ret;

    memset_s(&info, sizeof(info), 0, sizeof(info));
    if (strcpy_s(info.nv_name, sizeof(info.nv_name), HISI_CUST_NVRAM_NAME) != EOK) {
        OAM_ERROR_LOG1(0, OAM_SF_INI,
                       "write_conf_to_nvram:write_conf_to_nvram failed nv_name size[%lu] less than input[WINVRAM]",
                       sizeof(info.nv_name));
        return INI_FAILED;
    }
    info.nv_name[strlen(HISI_CUST_NVRAM_NAME) - 1] = '\0';
    info.nv_number = HISI_CUST_NVRAM_NUM;
    info.valid_size = HISI_CUST_NVRAM_LEN;
    info.nv_operation = HISI_CUST_NVRAM_WRITE;
    if (memcpy_s(info.nv_data, sizeof(info.nv_data), pc_arr, HISI_CUST_NVRAM_LEN) != EOK) {
        OAM_ERROR_LOG0(0, OAM_SF_INI, "write_conf_to_nvram:write nvm[memcpy_s] failed");
        return INI_FAILED;
    }

    ret = external_nve_direct_access_interface(&info);
    if (ret < -1) {
        ini_error("write nvm failed");
        return INI_FAILED;
    }

    return INI_SUCC;
}

/*
 *  Prototype    : get_cust_conf_string
 *  Description  : get config form *.ini file or dts(kernel)
 */
int32 get_cust_conf_string(int32 tag_index, int8 *puc_var, int8 *puc_value, uint32 size)
{
    memset_s(puc_value, size, 0, size);
    return ini_find_var_value(tag_index, puc_var, puc_value, size);
}
oal_module_symbol(get_cust_conf_string);

/*
 *  Prototype    : set_cust_conf_string
 *  Description  : set key with new value to nv buff
 *  Input        : tag_index, key_name, new key_value
 *  Return Value : 0 on success,or -1 on failed.
 */
int32 set_cust_conf_string(int32 tag_index, int8 *name, int8 *var)
{
    int32 ret;

    if (tag_index != CUST_MODU_NVRAM) {
        ini_error("NOT SUPPORT MODU TO WRITE");
        return INI_FAILED;
    }

    ret = write_conf_to_nvram(name, var);

    return ret;
}

/*
 *  Prototype    : get_cust_conf_int32
 *  Description  : get config form *.ini file or dts(kernel)
 */
int32 get_cust_conf_int32(int32 tag_index, int8 *puc_var, int32 *puc_value)
{
    int32 ret;
    int8 out_str[INI_READ_VALUE_LEN] = {0};

    ret = ini_find_var_value(tag_index, puc_var, out_str, sizeof(out_str));
    if (ret < 0) {
        /* ini_find_var_value has error log, delete this log */
        ini_debug("cust modu didn't get var of %s.", puc_var);
        return INI_FAILED;
    }

    if (!strncmp(out_str, "0x", strlen("0x")) || !strncmp(out_str, "0X", strlen("0X"))) {
        ini_debug("get hex of:%s.", puc_var);
        ret = sscanf_s(out_str, "%x", puc_value);
    } else {
        ret = sscanf_s(out_str, "%d", puc_value);
    }

    if (ret < 0) {
        ini_error("%s trans to int failed", puc_var);
        return INI_FAILED;
    }
    ini_debug("conf %s get vale:%d", puc_var, *puc_value);

    return INI_SUCC;
}

oal_module_symbol(get_cust_conf_int32);

/*
 * 函 数 名  : bin_mem_check
 * 功能描述  : 二进制内存检查
 * 输入参数  : int8 *pc_dest需要对比的内存地址
 *             int8 *pc_src比较的内存地址
 *             int16 us_lenth需要比较的内存长度
 * 返 回 值:全部相同0，不同-1
 */
STATIC int32 bin_mem_check(int8 *pc_dest, const int8 *pc_src, uint16 us_lenth)
{
    uint16 loop;
    if ((pc_dest == NULL) || (pc_src == NULL)) {
        ini_error("pointer is NULL!");
        return INI_FAILED;
    }

    if (us_lenth == 0) {
        return INI_SUCC;
    }

    for (loop = 0; loop < us_lenth; loop++) {
        if (pc_dest[loop] != pc_src[loop]) {
            return INI_FAILED;
        }
    }
    return INI_SUCC;
}

static int8 *search_target_str(int8* read_buf, int32 buf_len, const int8 *target_str)
{
    int32 loop;
    int16 target_len = (int16)OAL_STRLEN(target_str);

    for (loop = 0; loop < buf_len - target_len; loop++) {
        if (!bin_mem_check(&read_buf[loop], target_str, target_len)) {
            return &read_buf[loop];
        }
    }
    return NULL;
}

/*
 * 函 数 名  : get_str_from_file
 * 功能描述  : 从文件中获取固定字符串，保存到另外的位置
 * 输入参数  : char *file_path 文件绝对路径
 *             const int8 *target_str 查找内容
 */
int8 *get_str_from_file(int8 *pc_file_path, const int8 *target_str)
{
    ini_file *fp = NULL;
    uint32 str_len;
    int32 read_bytes;
    int8 read_buf[INI_KERNEL_READ_LEN + 1] = {0};
    int8 *version_str = NULL;
    int8 target_len;
    ini_info("%s", __func__);

    if (unlikely((pc_file_path == NULL) || (target_str == NULL))) {
        ini_error("arg is NULL!");
        return NULL;
    }

    target_len = OAL_STRLEN(target_str);
    if (target_len == 0) {
        ini_error("target_str is empty!");
        return NULL;
    }

    fp = ini_file_open(pc_file_path, "rt");
    if (unlikely(fp == NULL)) {
        ini_error("open file %s fail!", pc_file_path);
        return NULL;
    }
    ini_info("open file %s success to find str \"%s\"!", pc_file_path, target_str);

    /* 由于每次比较都会留uc_str_check_len不比较所以不是0 */
    do {
        read_bytes = oal_file_read_ext(fp, fp->f_pos, read_buf, INI_KERNEL_READ_LEN);
        if (read_bytes <= target_len) {
            ini_file_close(fp);
            ini_error("cann't find device sw version string in %s!", pc_file_path);
            return NULL;
        }

        read_buf[read_bytes] = '\0';
        version_str = search_target_str(read_buf, read_bytes, target_str);
        if (version_str != NULL) {
            fp->f_pos += (version_str - read_buf);
            fp->f_pos += target_len;
            fp->f_pos -= OAL_STRLEN(DEV_SW_VERSION_HEAD5BYTE);
            memset_s(read_buf, INI_KERNEL_READ_LEN, 0, INI_KERNEL_READ_LEN);
            /* 读取到‘\n’或者最大192B数据到ac_read_buf */
            ko_read_line(fp, read_buf, sizeof(read_buf));
            break;
        } else {
            fp->f_pos += (read_bytes - target_len);
        }
    } while (1);

    str_len = OAL_STRLEN(read_buf);
    version_str = (int8 *)kmalloc(str_len + 1, GFP_KERNEL);
    if (unlikely(version_str == NULL)) {
        ini_error("find device sw version, but memory alloc fail!");
    }else {
        if (memcpy_s(version_str, str_len + 1, read_buf, str_len + 1) != EOK) {
            ini_error("memcpy_s error");
            return NULL;
        }
        ini_info("find device sw version :%s", version_str);
    }

    ini_file_close(fp);
    return version_str;
}

int ini_cfg_init(void)
{
    int32 ret;
    int8 auc_dts_ini_path[INI_FILE_PATH_LEN] = {0};

    ini_info("hi110x ini config search init!\n");

    ret = get_ini_file_name_from_dts(PROC_NAME_INI_FILE_NAME, auc_dts_ini_path, sizeof(auc_dts_ini_path));
    if (ret < 0) {
        ini_error("can't find dts proc %s\n", PROC_NAME_INI_FILE_NAME);
        return INI_FAILED;
    }

    ini_init_mutex(&g_file_mutex);

    ret = snprintf_s(g_ini_file_name, sizeof(g_ini_file_name), sizeof(g_ini_file_name) - 1, "%s", auc_dts_ini_path);
    if (ret < 0) {
        ini_error("ini_cfg_init snprintf_s failed \n");
        return INI_FAILED;
    }

    if (snprintf_s(g_ini_conn_file_name, sizeof(g_ini_conn_file_name),
                   sizeof(g_ini_conn_file_name) - 1, "%s", CUST_PATH_INI_CONN) < 0) {
        ini_error("space is not enough\n");
        return INI_FAILED;
    }

    ini_info("%s@%s\n", PROC_NAME_INI_FILE_NAME, g_ini_file_name);

    print_device_version();

    read_tcxo_dcxo_ini_file();

    set_i3c_switch_mode();

    return INI_SUCC;
}

void ini_cfg_exit(void)
{
    ini_info("hi110x ini config search exit!\n");
}
