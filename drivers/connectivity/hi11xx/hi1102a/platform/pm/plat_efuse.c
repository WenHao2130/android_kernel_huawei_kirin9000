

/* 头文件包含 */
#include "plat_efuse.h"
#include "board.h"
#include "securec.h"
/* 全局变量定义 */
uint8 g_hi110x_ec_version = V100;

/*
 * 函 数 名  : read_efuse_ec_version
 * 功能描述  : 从device EFUSE中读取EC版本号
 */
void read_efuse_ec_version(void)
{
    int32 ret;
    uint8 buff[EFUSE_REG_WIDTH] = { 0x00 };
    uint8 uc_ec_version;
    uint32 i = 0;

    if (get_hi110x_subchip_type() != BOARD_VERSION_HI1102) {
        ps_print_info("no need read device ec version\n");
        return;
    }

    ret = number_type_cmd_send(RMEM_CMD_KEYWORD, GET_EFUSE_EC_VERSION);
    if (ret < 0) {
        ps_print_warning("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, GET_EFUSE_EC_VERSION, ret);
        return;
    }

    ret = read_msg((uint8 *)buff, sizeof(buff));
    if (ret < 0) {
        ps_print_warning("read efuse ec version fail, read_len = %d, return = %d\n", (int32)sizeof(buff), ret);
        return;
    }

    for (i = 0; i < sizeof(buff); i++) {
        ps_print_dbg("ec version[%d]=0x%x\n", i, buff[i]);
    }

    uc_ec_version = buff[1];         // Byte24(0x50000771)对应bit[191:184]
    uc_ec_version &= ((uint8)0x03);  // bit[185:184]标示EC version

    if (uc_ec_version == V100) {
        ps_print_info("hi110x read efuse V100[0x%x]\n", uc_ec_version);
        g_hi110x_ec_version = V100;
    } else {
        ps_print_info("hi110x read efuse V120[0x%x]\n", uc_ec_version);
        g_hi110x_ec_version = V120;
    }

    return;
}

/*
 * 函 数 名  : get_ec_version
 * 功能描述  : 获取hi110x芯片EC版本号
 * 返 回 值  : EC版本号
 */
uint8 get_ec_version(void)
{
    return g_hi110x_ec_version;
}

/*
 * 函 数 名  : mask_bits
 * 功能描述  : 将位段置为零
 */
static void mask_bits(uint32 value[], uint32 start_bits, uint32 end_bits)
{
    uint32 index = 0;
    uint32 i = 0;
    uint32 j = 0;

    if (value == NULL) {
        return;
    }

    for (index = start_bits; index <= end_bits; index++) {
        i = index / EFUSE_VALUE_WIDTH;
        j = index % EFUSE_VALUE_WIDTH;
        value[i] &= ~(1u << j);
    }
}
/*
 * 函 数 名  : check_efuse_file_exist
 * 功能描述  : 检查文件是否存在
 */
static int32 check_efuse_file_exist(void)
{
    mm_segment_t fs;
    struct file *fp = NULL;

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(EFUSE_FILE_PATH, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        set_fs(fs);
        return -EFAIL;
    }

    filp_close(fp, NULL);
    set_fs(fs);

    return SUCC;
}

/*
 * 函 数 名  : get_efuse_from_device
 * 功能描述  : 从device获取efuse信息
 */
static int32 get_efuse_from_device(uint32 *buff, int32 len)
{
    int32 ret;

    if (buff == NULL) {
        ps_print_err("efuse buff is NULL\n");
        return -EFAIL;
    }

    ret = number_type_cmd_send(RMEM_CMD_KEYWORD, GET_EFUSE_CMD);
    if (ret < 0) {
        ps_print_warning("send cmd %s:%s fail,ret = %d\n", RMEM_CMD_KEYWORD, GET_EFUSE_CMD, ret);
        return -EFAIL;
    }

    ret = read_msg((uint8 *)buff, len);
    if (ret < 0) {
        ps_print_warning("read efuse fail, read_len = %d, return = %d\n", len, ret);
        return -EFAIL;
    }

    return SUCC;
}
/*
 * 函 数 名  : store_efuse_into_file
 * 功能描述  : 将efuse信息保存在下
 */
static int32 store_efuse_into_file(uint32 *buff, uint32 buff_len)
{
    struct file *fp = NULL;
    loff_t pos;
    uint32 index = 0;
    ssize_t ret;
    mm_segment_t fs;

    if (buff == NULL) {
        ps_print_err("efuse buff is NULL\n");
        return -EFAIL;
    }

    mask_bits(buff, DIEID_BIT_4, DIEID_BIT_21);
    mask_bits(buff, DIEID_BIT_45, DIEID_BIT_45);
    mask_bits(buff, DIEID_BIT_53, DIEID_BIT_53);
    mask_bits(buff, DIEID_BIT_79, DIEID_BIT_95);

    memset_s(&fs, sizeof(fs), 0x00, sizeof(fs));

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(EFUSE_FILE_PATH, O_CREAT | O_RDWR, 0644);
    if (IS_ERR(fp)) {
        set_fs(fs);
        ps_print_err("open %s fail, errno = %ld\n", EFUSE_FILE_PATH, PTR_ERR(fp));
        return -EFAIL;
    }

    pos = 0;
    for (index = 0; index < buff_len; index++) {
        ret = vfs_write(fp, (uint8 *)(&buff[index]), sizeof(uint16), &pos);
        if (ret < 0) {
            ps_print_err("write %s fail, ret = %d\n", EFUSE_FILE_PATH, (int32)ret);
            filp_close(fp, NULL);
            set_fs(fs);
            return -EFAIL;
        }
    }

    filp_close(fp, NULL);
    set_fs(fs);

    return SUCC;
}

/*
 * 函 数 名  : store_efuse_info
 * 功能描述  : 存储efuse信息
 */
void store_efuse_info(void)
{
    int32 ret;
    uint32 buff[EFUSE_REG_NUM] = { 0x00 };
    static int32 retry_count = 0;

    if (retry_count >= EFUSE_RETRY) {
        return;
    }
    retry_count++;

    read_efuse_ec_version();

    ret = check_efuse_file_exist();
    if (ret == SUCC) {
        retry_count = EFUSE_RETRY;
        return;
    }

    ret = get_efuse_from_device(buff, sizeof(buff));
    if (ret != SUCC) {
        ps_print_err("get efuse from device fail\n");
        return;
    }

    ret = store_efuse_into_file(buff, EFUSE_REG_NUM);
    if (ret != SUCC) {
        ps_print_err("store efuse into %s fail\n", EFUSE_FILE_PATH);
        return;
    }

    retry_count = EFUSE_RETRY;
}
