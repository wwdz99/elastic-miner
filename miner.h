#ifndef __MINER_H__
#define __MINER_H__

#define PACKAGE_NAME "xel_miner"
#define PACKAGE_VERSION "0.2"

#define USER_AGENT PACKAGE_NAME "/" PACKAGE_VERSION
#define MAX_CPUS 16

#include <curl/curl.h>
#include <jansson.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* memset */

#ifdef WIN32
#include <windows.h>
#define sleep(secs) Sleep((secs) * 1000)
#else
#include <unistd.h> /* close */
#endif

#include "elist.h"
#include "ElasticPL\ElasticPL.h"
#include "crypto/md5.h"
#include "crypto/sha2.h"

#ifdef _MSC_VER
#define strdup(...) _strdup(__VA_ARGS__)
#define strcasecmp(x,y) _stricmp(x,y)
#define strncasecmp(x,y,z) _strnicmp(x,y,z)
#define __thread __declspec(thread)
#endif

#if JANSSON_MAJOR_VERSION >= 2
#define JSON_LOADS(str, err_ptr) json_loads(str, 0, err_ptr)
#define JSON_LOAD_FILE(path, err_ptr) json_load_file(path, 0, err_ptr)
#else
#define JSON_LOADS(str, err_ptr) json_loads(str, err_ptr)
#define JSON_LOAD_FILE(path, err_ptr) json_load_file(path, err_ptr)
#endif

#define MAX_SOURCE_SIZE 4096
#define VM_INPUTS 12

extern bool opt_debug;
extern bool opt_debug_epl;
extern bool opt_debug_vm;
extern bool opt_protocol;
extern bool opt_quiet;
extern int opt_timeout;
extern int opt_n_threads;

extern struct work_restart *work_restart;

static const int BASE85_POW[] = {
	1,
	85,
	85 * 85,
	85 * 85 * 85,
	85 * 85 * 85 * 85
};

enum submit_commands {
	SUBMIT_BTY_ANN,
	SUBMIT_BTY_CONF,
	SUBMIT_BOUNTY,
	SUBMIT_POW,
	SUBMIT_COMPLETE
};

struct work {
	int thr_id;
	uint64_t block_id;
	uint64_t work_id;
	uint32_t pow_target[8];
	int32_t vm_input[12];					// Value Of Random VM Inputs
	unsigned char multiplicator[32];
	unsigned char announcement_hash[32];
};

struct cpu_info {
	int thr_id;
	int accepted;
	int rejected;
	double khashes;
};

struct thr_info {
	int id;
	pthread_t pth;
	pthread_attr_t attr;
	struct thread_q	*q;
	struct cpu_info cpu;
	struct work work;
	char* c_code;
};

struct work_restart {
	volatile uint8_t restart;
	char padding[128 - sizeof(uint8_t)];
};

enum workio_commands {
	WC_GET_WORK,
	WC_SUBMIT_WORK,
};

struct submit_req {
	int thr_id;
	enum submit_commands req_type;
	time_t start_tm;	// Time Request Was Submitted
	time_t delay_tm;	// If Populated, Time When Next Request Can Be Sent
	int retries;
	char work_id[22];	// Work ID As A String
	char hash[65];		// Announcment Hash In Hex
	char mult[65];		// Multiplicator In Hex
};

struct workio_cmd {
	enum workio_commands cmd;
	struct thr_info *thr;
	struct submit_req *req;
};

struct header_info {
	char		*lp_path;
	char		*reason;
	char		*stratum_url;
};

struct data_buffer {
	void		*buf;
	size_t		len;
};

struct upload_buffer {
	const void	*buf;
	size_t		len;
	size_t		pos;
};

struct tq_ent {
	void				*data;
	struct list_head	q_node;
};

struct thread_q {
	struct list_head	q;

	bool frozen;

	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
};

enum blacklist_reason {
	BLST_UNSUPORTED_FUNCTION,
	BLST_SYNTAX_ERROR,
	BLST_RUNTIME_ERROR,
	BLST_INVALID_PACKAGE,
	BLST_NO_BOUNTIES
};

struct blacklisted_work {
	char work_id[22];
	enum blacklist_reason reason;
};

extern bool use_colors;
extern struct thr_info *thr_info;
extern pthread_mutex_t applog_lock;

enum {
	LOG_ERR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG,
	LOG_BLUE = 0x10,	// Custom Notices
};

#define JSON_RPC_LONGPOLL	(1 << 0)
#define JSON_RPC_QUIET_404	(1 << 1)
#define JSON_RPC_IGNOREERR  (1 << 2)
#define JSON_BUF_LEN 512

// Colors For Text Output
#define CL_N    "\x1B[0m"
#define CL_RED  "\x1B[31m"
#define CL_GRN  "\x1B[32m"
#define CL_YLW  "\x1B[33m"
#define CL_BLU  "\x1B[34m"
#define CL_MAG  "\x1B[35m"
#define CL_CYN  "\x1B[36m"
#define CL_BLK  "\x1B[22;30m" /* black */
#define CL_RD2  "\x1B[22;31m" /* red */
#define CL_GR2  "\x1B[22;32m" /* green */
#define CL_BRW  "\x1B[22;33m" /* brown */
#define CL_BL2  "\x1B[22;34m" /* blue */
#define CL_MA2  "\x1B[22;35m" /* magenta */
#define CL_CY2  "\x1B[22;36m" /* cyan */
#define CL_SIL  "\x1B[22;37m" /* gray */
#ifdef WIN32
#define CL_GRY  "\x1B[01;30m" /* dark gray */
#else
#define CL_GRY  "\x1B[90m"    /* dark gray selectable in putty */
#endif
#define CL_LRD  "\x1B[01;31m" /* light red */
#define CL_LGR  "\x1B[01;32m" /* light green */
#define CL_YL2  "\x1B[01;33m" /* yellow */
#define CL_LBL  "\x1B[01;34m" /* light blue */
#define CL_LMA  "\x1B[01;35m" /* light magenta */
#define CL_LCY  "\x1B[01;36m" /* light cyan */
#define CL_WHT  "\x1B[01;37m" /* white */

struct thread_q;

struct thread_q *tq_new(void);
void tq_free(struct thread_q *tq);
bool tq_push(struct thread_q *tq, void *data);
void *tq_pop(struct thread_q *tq, const struct timespec *abstime);
void tq_freeze(struct thread_q *tq);
void tq_thaw(struct thread_q *tq);

static void *submit_thread(void *userdata);
static void *key_monitor_thread(void *userdata);
static void *test_vm_thread(void *userdata);

static void *workio_thread(void *userdata);
static void workio_cmd_free(struct workio_cmd *wc);

extern uint32_t swap32(int a);
static void parse_cmdline(int argc, char *argv[]);
static void strhide(char *s);
static void parse_arg(int key, char *arg);
static void show_usage_and_exit(int status);
static void show_version_and_exit(void);
static bool load_test_file(char *test_source);
static int scanhash(int thr_id, struct work *work, long *hashes_done);

static bool get_work(struct thr_info *thr, struct work *work);
static int work_decode(const json_t *val, struct work *work, char *source_code);
static int get_upstream_work(CURL *curl, struct work *work);
static bool blacklist_work(char *work_id, enum blacklist_reason reason);

static bool submit_work(struct thr_info *thr, struct submit_req *req);
static bool submit_upstream_work(CURL *curl, struct submit_req *req);
static bool delete_submit_req(int idx);
static bool add_submit_req(struct work *work, enum submit_commands req_type);

// Function Prototypes - util.c
extern void applog(int prio, const char *fmt, ...);
extern int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
extern bool hex2ints(uint32_t *p, int array_sz, const char *hex, int len);
extern bool ascii85dec(unsigned char *str, int strsz, const char *ascii85);
static void databuf_free(struct data_buffer *db);
static size_t all_data_cb(const void *ptr, size_t size, size_t nmemb, void *user_data);
extern json_t* json_rpc_call(CURL *curl, const char *url, const char *userpass, const char *req, int *curl_err);

void sha256_epl(const unsigned char *message, unsigned int len, unsigned char *digest);
void sha256d(unsigned char *hash, const unsigned char *data, int len);

extern unsigned long genrand_int32(void);
extern void init_genrand(unsigned long s);

struct instance {

#ifdef WIN32
	HINSTANCE hndl;
	int(__cdecl* fill_ints)(int *);
	int(__cdecl* execute)();
#else
	void *hndl;
	int(*fill_ints)(int input[]);
	int(*execute)();
#endif

	uint32_t* vm_state1;
	uint32_t* vm_state2;
	uint32_t* vm_state3;
	uint32_t* vm_state4;

};

bool compile_and_link(char* source_code);
void create_instance(struct instance* inst);
void free_compiler(struct instance* inst);

#endif /* __MINER_H__ */