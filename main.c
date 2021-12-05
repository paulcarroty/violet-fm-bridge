/*
 * Build instructions:

ndk-build APP_BUILD_SCRIPT=./Android.mk NDK_PROJECT_PATH=.

 */

#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fm_const.h"

#define DEBUG(format, ...) \
    fprintf(stderr, format "\n", ##__VA_ARGS__)

#define ERROR(format, ...) \
    fprintf(stderr, format "\n", ##__VA_ARGS__)

#define EMIT_OPEN(format, ...) \
    emit("%s " format, __func__, ##__VA_ARGS__)

#define EMIT(format, ...) \
    do { \
        EMIT_OPEN(format, ##__VA_ARGS__); \
        emit_end(); \
    } while (0)

#define RESPOND(format, ...) \
    do { \
        fprintf(out, format, ##__VA_ARGS__); \
        emit_end(); \
    } while (0)

#define FM_JNI_SUCCESS 0L
#define FM_JNI_FAILURE -1L

#define STD_BUF_SIZE 256

static char FM_LIBRARY_NAME[] = "fm_helium.so";
static char FM_LIBRARY_SYMBOL_NAME[] = "FM_HELIUM_LIB_INTERFACE";

FILE *out = NULL;

void emit_end();
void emit(char *format, ...);

typedef void (*enb_result_cb)();
typedef void (*tune_rsp_cb)(int Freq);
typedef void (*seek_rsp_cb)(int Freq);
typedef void (*scan_rsp_cb)();
typedef void (*srch_list_rsp_cb)(uint16_t *scan_tbl);
typedef void (*stereo_mode_cb)(bool status);
typedef void (*rds_avl_sts_cb)(bool status);
typedef void (*af_list_cb)(uint16_t *af_list);
typedef void (*rt_cb)(char *rt);
typedef void (*ps_cb)(char *ps);
typedef void (*oda_cb)();
typedef void (*rt_plus_cb)(char *rt_plus);
typedef void (*ert_cb)(char *ert);
typedef void (*disable_cb)();
typedef void (*callback_thread_event)(unsigned int evt);
typedef void (*rds_grp_cntrs_cb)(char *rds_params);
typedef void (*rds_grp_cntrs_ext_cb)(char *rds_params);

typedef void (*fm_peek_cb)(char *peek_rsp);
typedef void (*fm_ssbi_peek_cb)(char *ssbi_peek_rsp);
typedef void (*fm_agc_gain_cb)(char *agc_gain_rsp);
typedef void (*fm_ch_det_th_cb)(char *ch_det_rsp);
typedef void (*fm_ecc_evt_cb)(char *ecc);
typedef void (*fm_sig_thr_cb)(int val, int status);
typedef void (*fm_get_ch_det_thrs_cb) (int val, int status);
typedef void (*fm_def_data_rd_cb) (int val, int status);
typedef void (*fm_get_blnd_cb) (int val, int status);
typedef void (*fm_set_ch_det_thrs_cb) (int status);
typedef void (*fm_def_data_wrt_cb) (int status);
typedef void (*fm_set_blnd_cb) (int status);
typedef void (*fm_get_stn_prm_cb) (int val, int status);
typedef void (*fm_get_stn_dbg_prm_cb) (int val, int status);
typedef void (*fm_enable_sb_cb) (int status);
typedef void (*fm_enable_sm_cb) (int status);

typedef struct {
   size_t  size;

   enb_result_cb  enabled_cb;
   tune_rsp_cb tune_cb;
   seek_rsp_cb  seek_cmpl_cb;
   scan_rsp_cb  scan_next_cb;
   srch_list_rsp_cb  srch_list_cb;
   stereo_mode_cb  stereo_status_cb;
   rds_avl_sts_cb  rds_avail_status_cb;
   af_list_cb  af_list_update_cb;
   rt_cb  rt_update_cb;
   ps_cb  ps_update_cb;
   oda_cb  oda_update_cb;
   rt_plus_cb  rt_plus_update_cb;
   ert_cb  ert_update_cb;
   disable_cb  disabled_cb;
   rds_grp_cntrs_cb rds_grp_cntrs_rsp_cb;
   rds_grp_cntrs_ext_cb rds_grp_cntrs_ext_rsp_cb;
   fm_peek_cb fm_peek_rsp_cb;
   fm_ssbi_peek_cb fm_ssbi_peek_rsp_cb;
   fm_agc_gain_cb fm_agc_gain_rsp_cb;
   fm_ch_det_th_cb fm_ch_det_th_rsp_cb;
   fm_ecc_evt_cb   ext_country_code_cb;
   callback_thread_event thread_evt_cb;
   fm_sig_thr_cb fm_get_sig_thres_cb;
   fm_get_ch_det_thrs_cb fm_get_ch_det_thr_cb;
   fm_def_data_rd_cb fm_def_data_read_cb;
   fm_get_blnd_cb fm_get_blend_cb;
   fm_set_ch_det_thrs_cb fm_set_ch_det_thr_cb;
   fm_def_data_wrt_cb fm_def_data_write_cb;
   fm_set_blnd_cb fm_set_blend_cb;
   fm_get_stn_prm_cb fm_get_station_param_cb;
   fm_get_stn_dbg_prm_cb fm_get_station_debug_param_cb;
   fm_enable_sb_cb fm_enable_slimbus_cb;
   fm_enable_sm_cb fm_enable_softmute_cb;
} fm_vendor_callbacks_t;

void fm_enabled_cb()
{
    EMIT("");
}

void fm_tune_cb(int freq)
{
    EMIT("%d", freq);
}

void fm_seek_cmpl_cb(int freq)
{
    EMIT("%d", freq);
}

void fm_scan_next_cb()
{
    EMIT("");
}

void fm_srch_list_cb(uint16_t *scan_tbl)
{
    /* The original JNI code at
     *
     * https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/fm-commonsys/tree/jni/android_hardware_fm.cpp?h=LA.UM.7.9.r1-11400-sm6150.0#n217
     *
     * would forward this event to the JAVA side, as an array of bytes, but
     * then the JAVA code doesn't seem to be doing anything with it. So, for
     * the time being, let's do the same.
     */
    EMIT("");

    DEBUG("Got list:");
    for (int i = 0; i < STD_BUF_SIZE / 2; i++) {
        DEBUG("  - %d", scan_tbl[i]);
    }
}

void fm_stereo_status_cb(bool stereo)
{
    EMIT("%d", stereo);
}

void fm_rds_avail_status_cb(bool rds_avl)
{
    EMIT("%d", rds_avl);
}

/* Handling of the data is copied from getAFInfo() in
 * https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/fm-commonsys/tree/qcom/fmradio/FmReceiver.java?h=LA.UM.7.9.r1-11400-sm6150.0#n1752
 */
void fm_af_list_update_cb(uint16_t *af_list)
{
    uint8_t *buff = (uint8_t*)af_list;

    uint32_t tuned_freq = (buff[0] & 0xFF) |
        ((buff[1] & 0xFF) << 8) |
        ((buff[2] & 0xFF) << 16) |
        ((buff[3] & 0xFF) << 24);

    uint16_t PI = (buff[4] & 0xFF) |
        ((buff[5] & 0xFF) << 8);

    uint8_t count = buff[6] & 0xFF;
    DEBUG("AF list for %d (PI = %d), size = %d", tuned_freq, PI, count);

    EMIT_OPEN("%u %hu", tuned_freq, PI);
    for (int i = 0; i < count; i++) {
        int af =
            (buff[6 + i * 4 + 1] & 0xFF) |
            ((buff[6 + i * 4 + 2] & 0xFF) << 8) |
            ((buff[6 + i * 4 + 3] & 0xFF) << 16) |
            ((buff[6 + i * 4 + 4] & 0xFF) << 24) ;
        DEBUG("AF: %d", af);
        emit(" %d", af);
    }

    emit_end();
}

/* Handling of the data is copied from getRTInfo() in
 * https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/fm-commonsys/tree/qcom/fmradio/FmReceiver.java?h=LA.UM.7.9.r1-11400-sm6150.0#n1602
 */
void fm_rt_update_cb(char *rt)
{
    int len = (int)(rt[0] & 0xFF);
    DEBUG("RT data length: %d", len);
    len = len+5; // the original Qualcomm code does this, don't ask me

    uint8_t *buff = (uint8_t *)rt;
    uint8_t piLower = buff[3];
    uint8_t piHigher = buff[2];
    int pi = (piHigher << 8) | piLower; // program ID
    int program_type = buff[1] & 0x1F;
    EMIT("%hd %d %*s", pi, program_type, len, rt + 5);
}

/* Handling of the data is copied from getPSInfo() in
 * https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/fm-commonsys/tree/qcom/fmradio/FmReceiver.java?h=LA.UM.7.9.r1-11400-sm6150.0#n1541
 */
void fm_ps_update_cb(char *ps)
{
    /* it's weird that the next line uses a mask of 0xff in the JNI file, but
     * 0x0f in the FmReceiver.java
     */
    int numPs = (int)(ps[0] & 0xFF);
    int len = numPs * 8;

    DEBUG("PS data length: %d", len);

    uint8_t *buff = (uint8_t *)ps;
    uint8_t piLower = buff[3];
    uint8_t piHigher = buff[2];
    int pi = (piHigher << 8) | piLower; // program ID
    int program_type = buff[1] & 0x1F;
    EMIT("%hd %d %*s", pi, program_type, len, ps + 5);
}

void fm_oda_update_cb()
{
    EMIT("");
}

/* Handling of the data is copied from getRTPlusInfo() in
 * https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/fm-commonsys/tree/qcom/fmradio/FmReceiver.java?h=LA.UM.7.9.r1-11400-sm6150.0#n1656
 */
void fm_rt_plus_update_cb(char *rt_plus)
{
    uint8_t *buff = (uint8_t*)rt_plus;
    int len = buff[0];

    DEBUG("RT+ data length: %d", len);
    if (len == 0) {
        EMIT("");
    } else {
        EMIT_OPEN("");
        int j = 2;
        for (int i = 1; (i <= 2) && (j < len); i++) {
            uint8_t tag_code = buff[j++];
            uint8_t tag_start_pos = buff[j++];
            uint8_t tag_len = buff[j++];
            if (tag_code > 0) {
                emit(" %hhu:%*s", tag_code, tag_len, rt_plus + tag_start_pos);
            }
        }
        emit_end();
    }
}

/* Handling of the data is copied from getERTInfo() in
 * https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/fm-commonsys/tree/qcom/fmradio/FmReceiver.java?h=LA.UM.7.9.r1-11400-sm6150.0#n1698
 */
void fm_ert_update_cb(char *ert)
{
    const int ENCODE_TYPE_IND = 1;
    const int ERT_DIR_IND = 2;

    uint8_t *buff = (uint8_t*)ert;
    int len = buff[0];

    DEBUG("ERT data length: %d", len);

    if (len == 0) {
        EMIT("");
    } else {
        len = len+3;
        const char *encoding = buff[ENCODE_TYPE_IND] == 1 ?
            "UTF-8" : "UCS-2";
        int format_dir = buff[ERT_DIR_IND];

        EMIT_OPEN("%s %d ", encoding, format_dir);

        for (int i = 0; i < len; i++) {
            emit("%02x", buff[i + 3]);
        }
        emit_end();
    }
}

/* Handling of the data is copied from getECCInfo() in
 * https://source.codeaurora.org/quic/la/platform/vendor/qcom-opensource/fm-commonsys/tree/qcom/fmradio/FmReceiver.java?h=LA.UM.7.9.r1-11400-sm6150.0#n1737
 */
void fm_ext_country_code_cb(char *ecc)
{
    // ECC = Extended Country Code
    uint8_t *buff = (uint8_t*)ecc;
    int len = buff[0];

    DEBUG("ECC data length: %d", len);

    if (len > 0) {
        int ecc_code = buff[9];
        EMIT("%d", ecc_code);
    }
}


void rds_grp_cntrs_rsp_cb(char *evt_buffer)
{
    DEBUG("rds_grp_cntrs_rsp_cb %s", evt_buffer);
}

void rds_grp_cntrs_ext_rsp_cb(char *evt_buffer)
{
    DEBUG("rds_grp_cntrs_ext_rsp_cb %s", evt_buffer);
}

void fm_disabled_cb()
{
    EMIT("");
}

void fm_peek_rsp_cb(char *peek_rsp) {
    DEBUG("fm_peek_rsp_cb %s", peek_rsp);
}

void fm_ssbi_peek_rsp_cb(char *ssbi_peek_rsp){
    DEBUG("fm_ssbi_peek_rsp_cb %s", ssbi_peek_rsp);
}

void fm_agc_gain_rsp_cb(char *agc_gain_rsp){
    DEBUG("fm_agc_gain_rsp_cb %s", agc_gain_rsp);
}

void fm_ch_det_th_rsp_cb(char *ch_det_rsp){
    DEBUG("fm_ch_det_th_rsp_cb %s", ch_det_rsp);
}

static void fm_thread_evt_cb(unsigned int event) {
    DEBUG("fm_thread_evt_cb %d", event);
}

static void fm_get_sig_thres_cb(int val, int status)
{
    // Signal threshold
    EMIT("%d %d", val, status);
}

static void fm_get_ch_det_thr_cb(int val, int status)
{
    EMIT("%d %d", val, status);
}

static void fm_set_ch_det_thr_cb(int status)
{
    EMIT("%d", status);
}

static void fm_def_data_read_cb(int val, int status)
{
    EMIT("%d %d", val, status);
}

static void fm_def_data_write_cb(int status)
{
    EMIT("%d", status);
}

static void fm_get_blend_cb(int val, int status)
{
    EMIT("%d %d", val, status);
}

static void fm_set_blend_cb(int status)
{
    EMIT("%d", status);
}

static void fm_get_station_param_cb(int val, int status)
{
    EMIT("%d %d", val, status);
}

static void fm_get_station_debug_param_cb(int val, int status)
{
    EMIT("%d %d", val, status);
}

static void fm_enable_slimbus_cb(int status)
{
    EMIT("%d", status);
}

static void fm_enable_softmute_cb(int status)
{
    EMIT("%d", status);
}

static fm_vendor_callbacks_t fm_callbacks = {
    sizeof(fm_callbacks),
    fm_enabled_cb,
    fm_tune_cb,
    fm_seek_cmpl_cb,
    fm_scan_next_cb,
    fm_srch_list_cb,
    fm_stereo_status_cb,
    fm_rds_avail_status_cb,
    fm_af_list_update_cb,
    fm_rt_update_cb,
    fm_ps_update_cb,
    fm_oda_update_cb,
    fm_rt_plus_update_cb,
    fm_ert_update_cb,
    fm_disabled_cb,
    rds_grp_cntrs_rsp_cb,
    rds_grp_cntrs_ext_rsp_cb,
    fm_peek_rsp_cb,
    fm_ssbi_peek_rsp_cb,
    fm_agc_gain_rsp_cb,
    fm_ch_det_th_rsp_cb,
    fm_ext_country_code_cb,
    fm_thread_evt_cb,
    fm_get_sig_thres_cb,
    fm_get_ch_det_thr_cb,
    fm_def_data_read_cb,
    fm_get_blend_cb,
    fm_set_ch_det_thr_cb,
    fm_def_data_write_cb,
    fm_set_blend_cb,
    fm_get_station_param_cb,
    fm_get_station_debug_param_cb,
    fm_enable_slimbus_cb,
    fm_enable_softmute_cb
};

typedef struct {
    int (*hal_init)(fm_vendor_callbacks_t *p_cb);
    int (*set_fm_ctrl)(int ioctl, int val);
    int (*get_fm_ctrl) (int ioctl, int *val);
} fm_interface_t;

static fm_interface_t *vendor_interface = NULL;

static int handle_getFreq()
{
    int freq = 0;
    int err = vendor_interface->get_fm_ctrl(V4L2_CID_PRV_IRIS_FREQ, &freq);
    if (err == 0) {
        RESPOND("%d", freq);
    }
    return err;
}

static int handle_setFreq(const char *input)
{
    int freq = 0;
    sscanf(input, " %d", &freq);
    return vendor_interface->set_fm_ctrl(V4L2_CID_PRV_IRIS_FREQ, freq);
}

static int handle_setControl(const char *input)
{
    int id = 0, value = 0;
    sscanf(input, " %i %i", &id, &value);
    return vendor_interface->set_fm_ctrl(id, value);
}

static int handle_getControl(const char *input)
{
    int id = 0, val = 0;
    sscanf(input, " %i", &id);
    int err = vendor_interface->get_fm_ctrl(id, &val);
    if (err >= 0) {
        RESPOND("%d", val);
    }
    return err;
}

static int handle_startSearch(const char *input)
{
    int dir = 0;
    sscanf(input, " %d", &dir);
    return vendor_interface->set_fm_ctrl(V4L2_CID_PRV_IRIS_SEEK, dir);
}

static int handle_cancelSearch()
{
    return vendor_interface->set_fm_ctrl(V4L2_CID_PRV_SRCHON, 0);
}

static int handle_getRSSI()
{
    int rmssi;
    int err = vendor_interface->get_fm_ctrl(V4L2_CID_PRV_IRIS_RMSSI, &rmssi);
    if (err == 0) {
        RESPOND("%d", rmssi);
    }
    return err;
}

static int handle_setBand(const char *input)
{
    int low = 0, high = 0;
    sscanf(input, " %i %i", &low, &high);
    int err = vendor_interface->set_fm_ctrl(V4L2_CID_PRV_IRIS_UPPER_BAND, high);
    if (err < 0) {
        ERROR("set band failed, high: %d", high);
        return err;
    }
    err = vendor_interface->set_fm_ctrl(V4L2_CID_PRV_IRIS_LOWER_BAND, low);
    if (err < 0) {
        ERROR("set band failed, low: %d", low);
        return err;
    }
    return 0;
}

static int handle_getLowerBand()
{
    int freq = 0;
    int err = vendor_interface->get_fm_ctrl(V4L2_CID_PRV_IRIS_LOWER_BAND, &freq);
    if (err == 0) {
        RESPOND("%d", freq);
    }
    return err;
}

static int handle_getUpperBand()
{
    int freq = 0;
    int err = vendor_interface->get_fm_ctrl(V4L2_CID_PRV_IRIS_UPPER_BAND, &freq);
    if (err == 0) {
        RESPOND("%d", freq);
    }
    return err;
}

static int handle_setMonoStereo(const char *input)
{
    int val = 0;
    sscanf(input, " %d", &val);
    return vendor_interface->set_fm_ctrl(V4L2_CID_PRV_IRIS_AUDIO_MODE, val);
}

static int handle_enableSlimbus(const char *input)
{
    int val = 0;
    sscanf(input, " %d", &val);
    return vendor_interface->set_fm_ctrl(V4L2_CID_PRV_ENABLE_SLIMBUS, val);
}

static int handle_enableSoftMute(const char *input)
{
    int val = 0;
    sscanf(input, " %d", &val);
    return vendor_interface->set_fm_ctrl(V4L2_CID_PRV_SOFT_MUTE, val);
}

typedef struct {
    char *name;
    int (*handler)(const char *input);
} Handler;

static const Handler handlers[] = {
    { "getFreq", handle_getFreq },
    { "setFreq", handle_setFreq },
    { "getControl", handle_getControl },
    { "setControl", handle_setControl },
    { "startSearch", handle_startSearch },
    { "cancelSearch", handle_cancelSearch },
    { "getRSSI", handle_getRSSI },
    { "setBand", handle_setBand },
    { "getLowerBand", handle_getLowerBand },
    { "getUpperBand", handle_getUpperBand },
    { "setMonoStereo", handle_setMonoStereo },
    { "enableSlimbus", handle_enableSlimbus },
    { "enableSoftMute", handle_enableSoftMute },
    { NULL, NULL },
};

void emit_end()
{
    fputc('\n', out);
    fflush(out);
}

void emit(char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(out, format, ap);
    va_end(ap);
}

int invoke_handler(const char *command, const char *parameters)
{
    const Handler *handler;
    for (handler = handlers; handler->name != NULL; handler++) {
        if (strcmp(command, handler->name) == 0) {
            DEBUG("invoking %s with %s", command, parameters);
            int err = handler->handler(parameters);
            if (err < 0) {
                DEBUG("handler failed: %d", err);
            }
            return 0;
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    out = stdout;

    void *lib_handle = dlopen(FM_LIBRARY_NAME, RTLD_NOW);
    if (!lib_handle) {
        ERROR("Failed to open %s: %s", FM_LIBRARY_NAME, dlerror());
        return EXIT_FAILURE;
    }

    DEBUG("Opened %s as %p", FM_LIBRARY_NAME, lib_handle);

    vendor_interface =
        (fm_interface_t *)dlsym(lib_handle, FM_LIBRARY_SYMBOL_NAME);
    if (!vendor_interface) {
        ERROR("Failed to load vendor interface: %s", dlerror());
        return EXIT_FAILURE;
    }

    DEBUG("Loaded vendor interface");

    int status = vendor_interface->hal_init(&fm_callbacks);
    if (status != 0) {
        ERROR("Failed to intialize vendor library (%d)", status);
        return EXIT_FAILURE;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Get rid of the \n
        char *newline = strchr(buffer, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }
        /* The first word is the command, the rest is parameters */
        const char *command = buffer;
        const char *parameters = NULL;
        char *space = strchr(buffer, ' ');
        if (space != NULL) {
            *space = '\0'; // to terminate the command
            parameters = space + 1;
        }

        int err = invoke_handler(command, parameters);
        if (err < 0) {
            ERROR("Command not found: '%s'", command);
            RESPOND("%d", -ENOENT);
        }
    }
    return EXIT_SUCCESS;
}
