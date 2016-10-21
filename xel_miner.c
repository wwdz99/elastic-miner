/*
* Copyright 2010 Jeff Garzik
* Copyright 2012-2014 pooler
* Copyright 2014 Lucas Jones
* Copyright 2016 sprocket
*
* This program is free software; you can redistribuSte it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*/

#define _GNU_SOURCE

#include <curl/curl.h>
#include <getopt.h>
#include <jansson.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "miner.h"

#ifdef WIN32
#include "compat/winansi.h"
#endif

enum prefs {
	PREF_PROFIT,	// Estimate most profitable based on WCET vs reward
	PREF_WCET,		// Fewest cycles required by work item
	PREF_WORKID,	// Specify work ID
	PREF_COUNT
};

static const char *pref_type[] = {
	"profit",
	"wcet",
	"workid",
	"\0"
};

bool opt_debug = false;
bool opt_debug_epl = false;
bool opt_debug_vm = false;
bool opt_quiet = false;
bool opt_protocol = false;
bool use_colors = true;
static int opt_retries = -1;
static int opt_fail_pause = 10;
static int opt_scantime = 60;  // Get New Work From Server At Least Every 60s
bool opt_test_miner = false;
bool opt_test_compiler = false;
int opt_timeout = 30;
int opt_n_threads = 0;
static enum prefs opt_pref = PREF_WCET;
char pref_workid[32];

int num_cpus;
char *g_work_nm;
char *g_work_id;
uint64_t g_cur_work_id;
unsigned char g_pow_target[65];

pthread_mutex_t applog_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t work_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t submit_lock = PTHREAD_MUTEX_INITIALIZER;

char *rpc_url = NULL;
char *rpc_user = NULL;
char *rpc_pass = NULL;
char *rpc_userpass = NULL;
char *passphrase = NULL;
char *publickey = NULL;
char *test_filename;

static struct timeval g_miner_start_time;
static struct work g_work = { 0 };
static time_t g_work_time = 0;
static struct blacklisted_work *blacklist;
static int g_blacklist_cnt = 0;

struct submit_req *g_submit_req;
int g_submit_req_cnt = 0;

uint32_t g_bounty_accepted_cnt = 0;
uint32_t g_bounty_rejected_cnt = 0;
uint32_t g_bounty_timeout_cnt = 0;
uint32_t g_bounty_deprecated_cnt = 0;
uint32_t g_bounty_error_cnt = 0;
uint32_t g_pow_accepted_cnt = 0;
uint32_t g_pow_rejected_cnt = 0;

int work_thr_id;
struct thr_info *thr_info;
struct work_restart *work_restart = NULL;
struct instance* inst = NULL;

extern uint32_t swap32(int a) {
	return ((a << 24) | ((a << 8) & 0x00FF0000) | ((a >> 8) & 0x0000FF00) | ((a >> 24) & 0x000000FF));
}

static char const usage[] = "\
Usage: " PACKAGE_NAME " [OPTIONS]\n\
Options:\n\
  -c, --config <file>         Use JSON-formated configuration file\n\
  -D, --debug                 Display debug output\n\
  -h, --help                  Display this help text and exit\n\
  -k, --public <publickey>    Public Key for Elastic Account\n\
  -m, --mining PREF[:ID]      Mining preference for choosing work\n\
                                profit       (Default) Estimate most profitable based on WCET vs reward\n\
                                wcet         Fewest cycles required by work item \n\
                                workid		 Specify work ID\n\
      --no-color              Don't display colored output\n\
  -o, --url=URL               URL of mining server\n\
  -p, --pass <password>       Password for mining server\n\
  -P, --phrase <passphrase>   Secret Passphrase for Elastic account\n\
      --protocol              Display dump of protocol-level activities\n\
  -q, --quiet                 Display minimal output\n\
  -r, --retries <n>           Number of times to retry if a network call fails\n\
                              (Default: Retry indefinitely)\n\
  -R, --retry-pause <n>       Time to pause between retries (Default: 10 sec)\n\
  -s, --scan-time <n>         Max time to scan work before requesting new work (Default: 60 sec)\n\
      --test-miner <file>     Run the Miner using JSON formatted work in <file>\n\
      --test-compiler <file>  Run the Parser / Compiler using the ElasticPL source code in <file>\n\
  -t, --threads <n>           Number of miner threads (Default: Number of CPUs)\n\
  -u, --user <username>       Username for mining server\n\
  -T, --timeout <n>           Timeout for rpc calls (Default: 30 sec)\n\
  -V, --version               Display version information and exit\n\
Options while mining ----------------------------------------------------------\n\n\
   s + <enter>                Display mining summary\n\
   d + <enter>                Toggle Debug mode\n\
   q + <enter>                Toggle Quite mode\n\
";

static char const short_options[] = "c:D:k:h:m:o:p:P:q:r:R:s:t:T:u:V";

static struct option const options[] = {
	{ "config",			1, NULL, 'c' },
	{ "debug",			0, NULL, 'D' },
	{ "help",			0, NULL, 'h' },
	{ "mining",			1, NULL, 'm' },
	{ "no-color",		0, NULL, 1001 },
	{ "pass",			1, NULL, 'p' },
	{ "phrase",			1, NULL, 'P' },
	{ "protocol",	    0, NULL, 1002 },
	{ "public",			1, NULL, 'k' },
	{ "quiet",			0, NULL, 'q' },
	{ "retries",		1, NULL, 'r' },
	{ "retry-pause",	1, NULL, 'R' },
	{ "scan-time",		1, NULL, 's' },
	{ "test-miner",		1, NULL, 1003 },
	{ "test-compiler",	1, NULL, 1004 },
	{ "threads",		1, NULL, 't' },
	{ "timeout",		1, NULL, 'T' },
	{ "url",			1, NULL, 'o' },
	{ "user",			1, NULL, 'u' },
	{ "version",		0, NULL, 'V' },
	{ 0, 0, 0, 0 }
};

static void parse_cmdline(int argc, char *argv[])
{
	int key;

	while (1) {
		key = getopt_long(argc, argv, short_options, options, NULL);

		if (key < 0)
			break;

		parse_arg(key, optarg);
	}
	if (optind < argc) {
		fprintf(stderr, "%s: unsupported non-option argument -- '%s'\n",
			argv[0], argv[optind]);
		show_usage_and_exit(1);
	}
}

static void strhide(char *s)
{
	if (*s) *s++ = 'x';
	while (*s) *s++ = '\0';
}

void parse_arg(int key, char *arg)
{
	char *p;
	int v, i;
	uint32_t *d32;

	switch (key) {
	case 'D':
		opt_debug = true;
		break;
	case 'h':
		show_usage_and_exit(0);
	case 'k':
		if (strlen(arg) != 64) {
			fprintf(stderr, "Invalid Public Key - %s\n", arg);
			show_usage_and_exit(1);
		}
		publickey = malloc(32);
		d32 = (uint32_t *)publickey;
		hex2ints(d32, 8, arg, 64);
		for (i = 0; i < 8; i++)
			d32[i] = swap32(d32[i]);
		break;
	case 'm':
		p = strchr(arg, ':');
		if (!p) {
			fprintf(stderr, "Invalid MiningPreference:ID pair -- '%s'\n", arg);
			show_usage_and_exit(1);
		}
		for (i = 0; i < PREF_COUNT; i++) {
			v = (int)strlen(pref_type[i]);
			if (!strncmp(arg, pref_type[i], v)) {
				opt_pref = (enum prefs) i;
				break;
			}
		}
		if (i == PREF_COUNT) {
			applog(LOG_ERR, "Unknown mining preference '%s'", arg);
			show_usage_and_exit(1);
		}
		if (p)
			strcpy(pref_workid, ++p);
		break;
	case 'o': {		// URL
		char *ap, *nm;
		ap = strstr(arg, "://");
		ap = ap ? ap + 3 : arg;
		nm = strstr(arg, "/nxt");
		if (ap != arg) {
			if (strncasecmp(arg, "http://", 7) && strncasecmp(arg, "https://", 8)) {
				fprintf(stderr, "ERROR: Invalid protocol -- '%s'\n", arg);
				show_usage_and_exit(1);
			}
			free(rpc_url);
			rpc_url = strdup(arg);
		}
		else {
			if (*ap == '\0' || *ap == '/') {
				fprintf(stderr, "ERROR: Invalid URL -- '%s'\n", arg);
				show_usage_and_exit(1);
			}
			free(rpc_url);
			rpc_url = (char*)malloc(strlen(ap) + 8);
			sprintf(rpc_url, "http://%s", ap);
		}

		if (!nm) {
			rpc_url = realloc(rpc_url, strlen(rpc_url) + 5);
			sprintf(rpc_url, "%s/nxt", rpc_url);
		}

		break;
	}
	case 'p':
		free(rpc_pass);
		rpc_pass = strdup(arg);
		strhide(arg);
		break;
	case 'P':
		passphrase = strdup(arg);
		strhide(arg);
		break;
	case 'q':
		opt_quiet = true;
		break;
	case 'r':
		v = atoi(arg);
		if (v < -1 || v > 9999)
			show_usage_and_exit(1);
		opt_retries = v;
		break;
	case 'R':
		v = atoi(arg);
		if (v < 1 || v > 9999)
			show_usage_and_exit(1);
		opt_fail_pause = v;
		break;
	case 's':
		v = atoi(arg);
		if (v < 1 || v > 9999)
			show_usage_and_exit(1);
		opt_scantime = v;
		break;
	case 't':
		v = atoi(arg);
		if (v < 1 || v > 9999)
			show_usage_and_exit(1);
		opt_n_threads = v;
		break;
	case 'T':
		v = atoi(arg);
		if (v < 1 || v > 9999)
			show_usage_and_exit(1);
		opt_timeout = v;
		break;
	case 'u':
		free(rpc_user);
		rpc_user = strdup(arg);
		break;
	case 'V':
		show_version_and_exit();
	case 1001:
		use_colors = false;
		break;
	case 1002:
		opt_protocol = true;
		break;
	case 1003:
		if (!arg)
			show_usage_and_exit(1);
		test_filename = malloc(strlen(arg) + 1);
		strcpy(test_filename, arg);
		opt_test_miner = true;
		break;
	case 1004:
		if (!arg)
			show_usage_and_exit(1);
		test_filename = malloc(strlen(arg) + 1);
		strcpy(test_filename, arg);
		opt_test_compiler = true;
		opt_debug = true;
		opt_debug_epl = true;
		opt_debug_vm = true;
		break;
	default:
		show_usage_and_exit(1);
	}
}

static void show_usage_and_exit(int status)
{
	if (status)
		fprintf(stderr, "Try `" PACKAGE_NAME " --help' for more information.\n");
	else
		printf(usage);
	exit(status);
}

static void show_version_and_exit(void)
{
	printf("\n built on " __DATE__
#ifdef _MSC_VER
		" with VC++ 2015\n");
#elif defined(__GNUC__)
		" with GCC");
	printf(" %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif

	printf(" features:"
#if defined(USE_ASM) && defined(__i386__)
		" i386"
#endif
#if defined(USE_ASM) && defined(__x86_64__)
		" x86_64"
#endif
#if defined(USE_ASM) && (defined(__i386__) || defined(__x86_64__))
		" SSE2"
#endif
#if defined(__x86_64__) && defined(USE_AVX)
		" AVX"
#endif
#if defined(__x86_64__) && defined(USE_AVX2)
		" AVX2"
#endif
#if defined(__x86_64__) && defined(USE_XOP)
		" XOP"
#endif
#if defined(USE_ASM) && defined(__arm__) && defined(__APCS_32__)
		" ARM"
#if defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) || \
	defined(__ARM_ARCH_5TEJ__) || defined(__ARM_ARCH_6__) || \
	defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || \
	defined(__ARM_ARCH_6M__) || defined(__ARM_ARCH_6T2__) || \
	defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || \
	defined(__ARM_ARCH_7__) || \
	defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || \
	defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
		" ARMv5E"
#endif
#if defined(__ARM_NEON__)
		" NEON"
#endif
#endif
		"\n\n");

	// Dependencies Versions
	printf("%s\n", curl_version());
#ifdef JANSSON_VERSION
	printf("jansson/%s ", JANSSON_VERSION);
#endif
#ifdef PTW32_VERSION
	printf("pthreads/%d.%d.%d.%d ", PTW32_VERSION);
#endif
	printf("\n");
	exit(0);
}

static bool load_test_file(char *buf) {
	size_t i, len, bytes;
	FILE *fp;

	fp = fopen(test_filename, "r");

	if (!fp) {
		fprintf(stderr, "ERROR: Unable to open test file: '%s'\n", test_filename);
		return false;
	}

	if (0 != fseek(fp, 0, SEEK_END)) {
		fprintf(stderr, "ERROR: Unable to determine size of test file: '%s'\n", test_filename);
		fclose(fp);
		return false;
	}

	len = ftell(fp);

	if (len > MAX_SOURCE_SIZE - 4) {
		fprintf(stderr, "ERROR: Test file exceeds max size (%d): %d bytes\n", MAX_SOURCE_SIZE, len);
		fclose(fp);
		return false;
	}

	rewind(fp);
	bytes = fread(buf, 1, len, fp);
	fclose(fp);

	if (bytes == 0)
		fprintf(stderr, "ERROR: Unable to read test file: '%s'\n", test_filename);

	buf[bytes] = 0;

	for (i = 0; i < strlen(buf); i++)
		buf[i] = tolower(buf[i]);

	return true;
}

static void *test_compiler_thread(void *userdata) {
	struct thr_info *mythr = (struct thr_info *) userdata;
	int thr_id = mythr->id;
	char test_code[MAX_SOURCE_SIZE];

	applog(LOG_DEBUG, "DEBUG: Loading Test File");
	if (!load_test_file(test_code))
				exit(EXIT_FAILURE);

	fprintf(stdout, "%s\n\n", test_code);

	// Convert The Source Code Into ElasticPL AST
	if (!create_epl_vm(test_code)) {
		applog(LOG_ERR, "ERROR: Exiting 'test_compiler'");
		exit(EXIT_FAILURE);
	}

	// Convert The ElasticPL Source Into A C Program Library
	if (!compile_and_link(test_code)) {
		applog(LOG_ERR, "ERROR: Exiting 'test_compiler'");
		exit(EXIT_FAILURE);
	}

	// Link To The C Program Library
	if (inst)
		free_compiler(inst);
	inst = calloc(1, sizeof(struct instance));
	create_instance(inst);
	free_compiler(inst);

	applog(LOG_NOTICE, "DEBUG: Compiler Test Complete");
	exit(EXIT_SUCCESS);
}

static bool get_vm_input(struct work *work) {
	int i;
	char msg[80];
	char hash[16];
	uint32_t *msg32 = (uint32_t *)msg;
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *workid32 = (uint32_t *)&work->work_id;
	uint32_t *blockid32 = (uint32_t *)&work->block_id;

	memcpy(&msg[0], work->multiplicator, 32);
	memcpy(&msg[32], publickey, 32);
	memcpy(&msg[64], &workid32[1], 4);	// Swap First 4 Bytes Of Long
	memcpy(&msg[68], &workid32[0], 4);	// With Second 4 Bytes Of Long
	memcpy(&msg[72], &blockid32[1], 4);
	memcpy(&msg[76], &blockid32[0], 4);
	msg32[16] = swap32(msg32[16]);
	msg32[17] = swap32(msg32[17]);
	msg32[18] = swap32(msg32[18]);
	msg32[19] = swap32(msg32[19]);

	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, msg, 80);
	MD5_Final(hash, &ctx);

	// Randomize The Inputs
	for (i = 0; i < 12; i++) {
		work->vm_input[i] = swap32(hash32[i % 4]);
		if (i > 4)
			work->vm_input[i] = work->vm_input[i] ^ work->vm_input[i - 3];
	}

	return true;
}

static int scanhash(int thr_id, struct work *work, long *hashes_done) {
	int i, rc;
	time_t t_start = time(NULL);
	char msg[64];
	char hash[32];
	uint32_t *msg32 = (uint32_t *)msg;
	uint32_t *hash32 = (uint32_t *)hash;
	uint32_t *mult32 = (uint32_t *)work->multiplicator;

	mult32[6] = genrand_int32();
	mult32[7] = genrand_int32();

	while (1) {
		// Check If New Work Is Available
		if (work_restart[thr_id].restart) {
			applog(LOG_DEBUG, "CPU%d: New work detected", thr_id);
			return 0;
		}

		// Increment mult32
		mult32[7] = mult32[7] + 1;
		if (mult32[7] == INT32_MAX) {
			mult32[7] = 0;
			mult32[6] = mult32[6] + 1;
		}

		// Get Values For VM Inputs
		get_vm_input(work);
		inst->fill_ints(work->vm_input);
		rc = inst->execute();

		// Hee, we have found a bounty, exit immediately
		if (rc == 1)
			return rc;

		// Check For POW Result
		memcpy(&msg[0], inst->vm_state1, 4);
		memcpy(&msg[4], inst->vm_state2, 4);
		memcpy(&msg[8], inst->vm_state3, 4);
		memcpy(&msg[12], inst->vm_state4, 4);
		msg32[0] = swap32(msg32[0]);
		msg32[1] = swap32(msg32[1]);
		msg32[2] = swap32(msg32[2]);
		msg32[3] = swap32(msg32[3]);

		for (i = 0; i < VM_INPUTS; i++)
			msg32[i + 4] = swap32(work->vm_input[i]);

		sha256(msg, 64, hash);

		// POW Solution Found
		if (swap32(hash32[0]) <= work->pow_target[0])
			rc = 2;
		else
			rc = 0;

		(*hashes_done)++;

		if (rc == 2) {
			return rc;
		}

		// Only Run For 1s Before Returning To Miner Thread
		if ((time(NULL) - t_start) >= 1)
			break;
	}
	return 0;
}

static bool get_work(struct thr_info *thr, struct work *work) {
	struct workio_cmd *wc;
	struct work *work_heap;

	// Fill Out Work Request Message
	wc = (struct workio_cmd *) calloc(1, sizeof(*wc));
	if (!wc)
		return false;

	wc->cmd = WC_GET_WORK;
	wc->thr = thr;

	// Send Work Request To workio Thread
	if (!tq_push(thr_info[work_thr_id].q, wc)) {
		workio_cmd_free(wc);
		return false;
	}

	// Wait For Work To Be Returned
	work_heap = (struct work*) tq_pop(thr->q, NULL);
	if (!work_heap)
		return false;

	// Copy Work From Heap
	memcpy(work, work_heap, sizeof(*work));
	free(work_heap);

	return true;
}

static bool workio_get_work(struct workio_cmd *wc, CURL *curl) {
	struct work *ret_work;
	int rc, failures = 0;

	ret_work = (struct work*) calloc(1, sizeof(*ret_work));
	if (!ret_work)
		return false;

	// Obtain New Work From Server via JSON_RPC
	while (1) {
		rc = get_upstream_work(curl, ret_work);

		if (rc)
			break;
		else {
			if ((opt_retries >= 0) && (++failures > opt_retries)) {
				applog(LOG_ERR, "ERROR: 'json_rpc_call' failed...terminating workio thread");
				free(ret_work);
				return false;
			}

			/* pause, then restart work-request loop */
			applog(LOG_ERR, "ERROR: 'json_rpc_call' failed...retrying in %d seconds", opt_fail_pause);
			sleep(opt_fail_pause);
		}
	}

	// Return Work To Request Thread
	if (!tq_push(wc->thr->q, ret_work))
		free(ret_work);

	return true;
}

static bool blacklist_work(char *work_id, enum blacklist_reason reason) {

	blacklist = realloc(blacklist, sizeof(struct blacklisted_work) * (g_blacklist_cnt + 1));
	if (!blacklist) {
		applog(LOG_ERR, "ERROR: Unable to allocate memory for blacklist");
		return false;
	}

	strcpy(blacklist[g_blacklist_cnt].work_id, work_id);
	blacklist[g_blacklist_cnt].reason = reason;
	g_blacklist_cnt++;

	return true;
}

static int get_upstream_work(CURL *curl, struct work *work) {
	int err, rc;
	char source_code[MAX_SOURCE_SIZE];
	json_t *val;
	struct timeval tv_start, tv_end, diff;

	memset(work, 0, sizeof(struct work));
	gettimeofday(&tv_start, NULL);

	if (!opt_test_miner) {
		val = json_rpc_call(curl, rpc_url, rpc_userpass, "requestType=getMineableWork&n=1", &err);
	}
	else {
		json_error_t err_val;
		val = JSON_LOAD_FILE(test_filename, &err_val);
		if (!json_is_object(val)) {
			if (err_val.line < 0)
				applog(LOG_ERR, "%s\n", err_val.text);
			else
				applog(LOG_ERR, "%s:%d: %s\n", test_filename, err_val.line, err_val.text);
			return 0;
		}

		char *str = json_dumps(val, JSON_INDENT(3));
		applog(LOG_DEBUG, "DEBUG: JSON Response -\n%s", str);
		free(str);
	}

	if (!val)
		return 0;

	gettimeofday(&tv_end, NULL);
	if (opt_protocol) {
		timeval_subtract(&diff, &tv_end, &tv_start);
		applog(LOG_DEBUG, "DEBUG: Time to get work: %.2f ms", (1000.0 * diff.tv_sec) + (0.001 * diff.tv_usec));
	}

	rc = work_decode(val, work, source_code);
	json_decref(val);

	// If We Are On The Same Work ID No Need To Recompile
	if (work->work_id && work->work_id == g_cur_work_id)
		return 1;

	g_cur_work_id = work->work_id;

	if (rc < 0)
		return -1;
	else if (rc > 0 && source_code) {
		gettimeofday(&tv_start, NULL);
		if (opt_protocol) {
			timeval_subtract(&diff, &tv_start, &tv_end);
			applog(LOG_DEBUG, "DEBUG: Time to decode work: %.2f ms", (1000.0 * diff.tv_sec) + (0.001 * diff.tv_usec));
		}

		applog(LOG_DEBUG, "DEBUG: Running ElasticPL Compiler");

		if (opt_debug_epl)
			applog(LOG_DEBUG, "DEBUG: ElasticPL Source Code -\n%s", source_code);

		if (!create_epl_vm(source_code)) {
			blacklist_work(g_work_id, BLST_SYNTAX_ERROR);
			return 0;
		}

		// Convert The ElasticPL Source Into A C Program Library
		if (!compile_and_link(source_code)) {
			blacklist_work(g_work_id, BLST_SYNTAX_ERROR);
			return 0;
		}

		// Link To The C Program Library
		if(inst)
			free_compiler(inst);
		inst = calloc(1, sizeof(struct instance));
		create_instance(inst);

		gettimeofday(&tv_end, NULL);
		if (opt_protocol) {
			timeval_subtract(&diff, &tv_end, &tv_start);
			applog(LOG_DEBUG, "DEBUG: Time to parse ElasticPL code: %.2f ms", (1000.0 * diff.tv_sec) + (0.001 * diff.tv_usec));
		}
	}
	else
		return 0;

	return 1;
}

static int work_decode(const json_t *val, struct work *work, char *source_code) {
	int i, j, idx, rc, num_pkg, best_pkg;
	int bty_limit = 0, bty_rcvd = 0, bty_pend = 0;
	float best_profit = 0.0;
	unsigned int wcet = 1, best_wcet = 0xFFFFFFFF;
	bool blacklisted = false;
	char *blk_id, *tgt, *wrk_id, *src, *str;
	long xel_per_pow;
	json_t *wrk, *pkg;

	memset(work, 0, sizeof(struct work));
	if (g_work_nm) {
		free(g_work_nm);
		g_work_nm = NULL;
	}
	if (g_work_id) {
		free(g_work_id);
		g_work_id = NULL;
	}
	if (g_pow_target) strcpy(g_pow_target, "");

	if (opt_protocol) {
		str = json_dumps(val, JSON_INDENT(3));
		applog(LOG_DEBUG, "DEBUG: JSON Response -\n%s", str);
		printf("DEBUG: JSON Response -\n%s", str);
		free(str);
	}

	wrk = json_object_get(val, "work_packages");
	tgt = (char *)json_string_value(json_object_get(val, "pow_target"));

	if (!wrk || !tgt) {
		applog(LOG_ERR, "Invalid JSON response to getwork request");
		return 0;
	}

	num_pkg = json_array_size(wrk);
	if (num_pkg == 0) {
		applog(LOG_INFO, "No work available...retrying in 15s");
		return -1;
	}

	strcpy(g_pow_target, tgt);
	rc = hex2ints(work->pow_target, 8, g_pow_target, strlen(tgt));
	if (!rc) {
		applog(LOG_ERR, "Invalid Target in JSON response to getwork request");
		return 0;
	}

	idx = 0;
	best_pkg = -1;

	for (i = 0; i<num_pkg; i++) {
		pkg = json_array_get(wrk, i);

		blk_id = (char *)json_string_value(json_object_get(pkg, "block_id"));
		wrk_id = (char *)json_string_value(json_object_get(pkg, "work_id"));
		xel_per_pow = (long)json_number_value(json_object_get(pkg, "xel_per_pow"));
		bty_limit = (int)json_integer_value(json_object_get(pkg, "bounty_limit"));
		bty_rcvd = (int)json_integer_value(json_object_get(pkg, "received_bounties"));
		if (!blk_id | !wrk_id || !xel_per_pow || !bty_limit) {
			applog(LOG_ERR, "Unable to parse work package");
			return 0;
		}

		// Check If Work Has Been Blacklisted
		blacklisted = false;
		for (j = 0; j < g_blacklist_cnt; j++) {
			//			applog(LOG_NOTICE, "checking: %s vs list: %s", wrk_id, blacklist[j].work_id);
			if (!strcmp(wrk_id, blacklist[j].work_id)) {
				blacklisted = true;
				break;
			}
		}

		if (blacklisted) {
			applog(LOG_DEBUG, "DEBUG: Skipping blacklisted work_id: %s", wrk_id);
			idx++;
			continue;
		}

		// Check If Work Has Available Bounties
		for (j = 0; j < g_submit_req_cnt; j++) {
			if ((g_submit_req[j].req_type == SUBMIT_BTY_ANN ||
				g_submit_req[j].req_type == SUBMIT_BTY_CONF ||
				g_submit_req[j].req_type == SUBMIT_BOUNTY) && (!strcmp(g_submit_req[j].work_id, wrk_id))) {
				bty_pend++;
			}
		}
		if ((bty_rcvd + bty_pend) >= bty_limit) {
			applog(LOG_DEBUG, "DEBUG: Skipping work_id: %s - No Bounties Left", wrk_id);
			return -1;
		}

		// Calculate WCET - Hopefully This Will Get Added To Response
		// TBD

		// Select Best Work Package
		if (opt_pref == PREF_WCET && wcet < best_wcet) {
			best_pkg = i;
			best_wcet = wcet;
		}
		else if (opt_pref == PREF_PROFIT && ((float)xel_per_pow / (float)wcet) > best_profit) {
			best_pkg = i;
			best_profit = ((float)xel_per_pow / (float)wcet);
		}
		else if (opt_pref == PREF_WORKID && (!strcmp(wrk_id, pref_workid))) {
			best_pkg = i;
			break;
		}
	}

	// If No Work Matched Current Preference Switch To Profit Mode
	if (best_pkg < 0) {
		opt_pref = PREF_PROFIT;
		applog(LOG_INFO, "No work available that matches preference...retrying in 15s");
		return -1;
	}

	// Copy Data From Best Package Into Work
	pkg = json_array_get(wrk, best_pkg);
	g_work_nm = strdup((char *)json_string_value(json_object_get(pkg, "title")));
	g_work_id = strdup((char *)json_string_value(json_object_get(pkg, "work_id")));
	blk_id = (char *)json_string_value(json_object_get(pkg, "block_id"));
	wrk_id = (char *)json_string_value(json_object_get(pkg, "work_id"));
	work->block_id = strtoull(blk_id, NULL, 10);
	work->work_id = strtoull(wrk_id, NULL, 10);

	// Extract The ElasticPL Source Code
	src = (char *)json_string_value(json_object_get(pkg, "source"));
	if (!src || strlen(src) > MAX_SOURCE_SIZE) {
		applog(LOG_ERR, "Invalid 'source' for work_id: %s", g_work_id);

		// Blacklist This Work ID
		blacklist_work(g_work_id, BLST_INVALID_PACKAGE);

		return 0;
	}

	rc = ascii85dec(source_code, MAX_SOURCE_SIZE, src);
	if (!rc) {
		applog(LOG_ERR, "Unable to decode 'source' for work_id: %s\n\n%s\n", g_work_id, src);

		// Blacklist This Work ID
		blacklist_work(g_work_id, BLST_INVALID_PACKAGE);

		return 0;
	}

	return 1;
}

static bool submit_work(struct thr_info *thr, struct submit_req *req) {
	struct workio_cmd *wc;

	// Fill Out Work Request Message
	wc = (struct workio_cmd *) calloc(1, sizeof(*wc));
	if (!wc)
		return false;

	wc->cmd = WC_SUBMIT_WORK;
	wc->thr = thr;
	wc->req = req;

	// Send Solution To workio Thread
	if (!tq_push(thr_info[work_thr_id].q, wc))
		goto err_out;

	return true;

err_out:
	workio_cmd_free(wc);
	return false;
}

static bool workio_submit_work(struct workio_cmd *wc, CURL *curl)
{
	int failures = 0;

	// Sumbit Solutions via JSON-RPC
	while (!submit_upstream_work(curl, wc->req)) {

		if ((opt_retries >= 0) && (++failures > opt_retries)) {
			applog(LOG_ERR, "...terminating workio thread");
			return false;
		}

		applog(LOG_ERR, "...retry after %d seconds", opt_fail_pause);
		sleep(opt_fail_pause);
	}

	return true;
}

static bool submit_upstream_work(CURL *curl, struct submit_req *req) {
	int err;
	json_t *val;
	struct timeval tv_start, tv_end, diff;
	char data[1000];
	char *url, *err_desc, *accepted;

	url = calloc(1, strlen(rpc_url) + 50);
	if (!url) {
		applog(LOG_ERR, "ERROR: Unable to allocate memory for submit work url");
		return false;
	}

	if (req->req_type == SUBMIT_BTY_ANN) {
		sprintf(url, "%s?requestType=bountyAnnouncement", rpc_url);
		sprintf(data, "deadline=3&feeNQT=0&amountNQT=5000&work_id=%s&hash_announcement=%s&secretPhrase=%s", req->work_id, req->hash, passphrase);
	}
	else if (req->req_type == SUBMIT_BTY_CONF) {
		sprintf(url, "%s?requestType=getApprovedBounties", rpc_url);
		sprintf(data, "work_id=%s&hash_announcement=%s&secretPhrase=%s", req->work_id, req->hash, passphrase);
	}
	else if (req->req_type == SUBMIT_BOUNTY) {
		sprintf(url, "%s?requestType=createPoX", rpc_url);
		sprintf(data, "deadline=3&feeNQT=0&amountNQT=0&work_id=%s&multiplicator=%s&is_pow=false&secretPhrase=%s", req->work_id, req->mult, passphrase);
	}
	else if (req->req_type == SUBMIT_POW) {
		sprintf(url, "%s?requestType=createPoX", rpc_url);
		sprintf(data, "deadline=3&feeNQT=0&amountNQT=0&work_id=%s&multiplicator=%s&is_pow=true&secretPhrase=%s", req->work_id, req->mult, passphrase);
	}
	else {
		applog(LOG_ERR, "ERROR: Unknown request type");
		req->req_type = SUBMIT_COMPLETE;
		free(url);
		return true;
	}

	gettimeofday(&tv_start, NULL);
	val = json_rpc_call(curl, url, rpc_userpass, data, &err);

	if (!val)
		return false;

	gettimeofday(&tv_end, NULL);
	if (opt_protocol) {
		timeval_subtract(&diff, &tv_end, &tv_start);
		applog(LOG_DEBUG, "DEBUG: Time to submit solution: %.2f ms", (1000.0 * diff.tv_sec) + (0.001 * diff.tv_usec));
	}

	applog(LOG_DEBUG, "DEBUG: Submit request - %s %s", url, data);

	accepted = (char *)json_string_value(json_object_get(val, "approved"));
	err_desc = (char *)json_string_value(json_object_get(val, "errorDescription"));

	if (err_desc)
		applog(LOG_DEBUG, "DEBUG: Submit response error - %s", err_desc);

	if (req->req_type == SUBMIT_BTY_ANN) {
		if (err_desc) {
			if (strstr(err_desc, "duplicate")) {
				applog(LOG_DEBUG, "Work ID: %s is not accepting bounties", req->work_id);
				req->delay_tm = time(NULL) + 30;  // Retry In 30s
			}
			else {
				req->req_type = SUBMIT_COMPLETE;
				g_bounty_error_cnt++;
			}
		}
		else
			req->req_type = SUBMIT_BTY_CONF;
	}
	else if (req->req_type == SUBMIT_BTY_CONF) {
		if (accepted && !strcmp(accepted, "true")) {
			req->req_type = SUBMIT_BOUNTY;
		}
		else if (accepted && !strcmp(accepted, "deprecated")) {
			applog(LOG_NOTICE, "CPU%d: %s***** Bounty Rejected - Deprecated *****", req->thr_id, CL_RED);
			g_bounty_deprecated_cnt++;
			req->req_type = SUBMIT_COMPLETE;
		}
		else if (req->retries++ > 20) {		// Timeout After 10 Min
			applog(LOG_NOTICE, "CPU%d: %s***** Bounty Timed Out *****", req->thr_id, CL_RED);
			g_bounty_timeout_cnt++;
			req->req_type = SUBMIT_COMPLETE;
		}
		else {
			req->delay_tm = time(NULL) + 30;  // Retry In 30s
			applog(LOG_DEBUG, "DEBUG: Retry confirmation in 30s");
		}
	}
	else if (req->req_type == SUBMIT_BOUNTY) {
		if (err_desc) {
			applog(LOG_NOTICE, "CPU%d: %s***** Bounty Rejected! *****", req->thr_id, CL_RED);
			g_bounty_rejected_cnt++;
		}
		else {
			applog(LOG_NOTICE, "CPU%d: %s***** Bounty Claimed! *****", req->thr_id, CL_GRN);
			g_bounty_accepted_cnt++;
		}
		req->req_type = SUBMIT_COMPLETE;
	}
	else if (req->req_type == SUBMIT_POW) {
		if (err_desc) {
			applog(LOG_NOTICE, "CPU%d: %s***** POW Rejected! *****", req->thr_id, CL_RED);
			g_pow_rejected_cnt++;
		}
		else {
			applog(LOG_NOTICE, "CPU%d: %s***** POW Accepted! *****", req->thr_id, CL_CYN);
			g_pow_accepted_cnt++;
		}
		req->req_type = SUBMIT_COMPLETE;
	}

	json_decref(val);
	free(url);
	return true;
}

static void *miner_thread(void *userdata) {
	struct thr_info *mythr = (struct thr_info *) userdata;
	int thr_id = mythr->id;
	struct work work;
	char s[16];
	long hashes_done;
	struct timeval tv_start, tv_end, diff;
	int rc = 0;
	unsigned char msg[41];
	uint32_t *msg32 = (uint32_t *)msg;
	uint32_t *workid32;
	double eval_rate;

	hashes_done = 0;
	memset(&work, 0, sizeof(work));
	workid32 = (uint32_t *)&work.work_id;
	gettimeofday((struct timeval *) &tv_start, NULL);

	while (1) {

		// Obtain New Work From workio Thread
		pthread_mutex_lock(&work_lock);

		if ((time(NULL) - g_work_time) >= opt_scantime) {
			if (!get_work(mythr, &g_work)) {
				applog(LOG_ERR, "Unable to retrieve work...exiting mining thread #%d", mythr->id);
				pthread_mutex_unlock(&work_lock);
				goto out;
			}
			g_work_time = time(NULL);
			if (!g_work.work_id) {
				sleep(15);	// Wait 15s Before Checking For Work Again
				g_work_time = 0;
				pthread_mutex_unlock(&work_lock);
				continue;
			}
		}

		// Check If We Are Mining The Most Current Work
		if (work.work_id != g_work.work_id)
			memcpy((void *)&work, (void *)&g_work, sizeof(struct work));

		pthread_mutex_unlock(&work_lock);
		work_restart[thr_id].restart = 0;

		// Scan Work For POW Hash & Bounties
		rc = scanhash(thr_id, &work, &hashes_done);

		// Record Elapsed Time
		gettimeofday(&tv_end, NULL);
		timeval_subtract(&diff, &tv_end, &tv_start);
		if (diff.tv_sec >= 5) {
			eval_rate = (double)(hashes_done / (diff.tv_sec + diff.tv_usec * 1e-6));
			if (!opt_quiet) {
				sprintf(s, eval_rate >= 1000.0 ? "%0.2f kEval/s" : "%0.2f Eval/s", (eval_rate >= 1000.0) ? eval_rate / 1000 : eval_rate);
				applog(LOG_INFO, "CPU%d: %s", thr_id, s);
			}
			gettimeofday((struct timeval *) &tv_start, NULL);
			hashes_done = 0;
		}

		// Submit Work That Meets Bounty Criteria
		if (rc == 1) {
			applog(LOG_NOTICE, "CPU%d: Submitting Bounty Solution", thr_id);

			// Create Announcement Message
			memcpy(&msg[0], &workid32[1], 4);	// Swap First 4 Bytes Of Long
			memcpy(&msg[4], &workid32[0], 4);	// With Second 4 Bytes Of Long
			memcpy(&msg[8], work.multiplicator, 32);
			msg[40] = 1;
			msg32[0] = swap32(msg32[0]);
			msg32[1] = swap32(msg32[1]);

			// Create Announcment Hash
			sha256(msg, 41, work.announcement_hash);

			// Create Submit Request
			if (!add_submit_req(&work, SUBMIT_BTY_ANN))
				break;


			// Force g_work To Refresh
			pthread_mutex_lock(&work_lock);
			g_work_time = 0;
			pthread_mutex_unlock(&work_lock);
		}

		// Submit Work That Meets POW Target
		if (rc == 2) {
			applog(LOG_NOTICE, "CPU%d: Submitting POW Solution", thr_id);
			if (!add_submit_req(&work, SUBMIT_POW))
				break;

			// Force g_work To Refresh
			pthread_mutex_lock(&work_lock);
			g_work_time = 0;
			pthread_mutex_unlock(&work_lock);
		}
	}

out:
	tq_freeze(mythr->q);

	return NULL;
}

void restart_threads(void)
{
	int i;

	for (i = 0; i < opt_n_threads; i++)
		work_restart[i].restart = 1;
}

static void *workio_thread(void *userdata)
{
	struct thr_info *mythr = (struct thr_info *) userdata;
	CURL *curl;
	bool ok = true;

	curl = curl_easy_init();
	if (!curl) {
		applog(LOG_ERR, "CURL initialization failed");
		return NULL;
	}

	while (ok) {
		struct workio_cmd *wc;

		// Wait For workio_cmd To Arrive In Queue
		wc = (struct workio_cmd *) tq_pop(mythr->q, NULL);
		if (!wc) {
			ok = false;
			break;
		}

		switch (wc->cmd) {
		case WC_GET_WORK:
			ok = workio_get_work(wc, curl);
			break;
		case WC_SUBMIT_WORK:
			ok = workio_submit_work(wc, curl);
			break;
		default:	// Should Not Happen
			ok = false;
			break;
		}

		workio_cmd_free(wc);
	}

	tq_freeze(mythr->q);
	curl_easy_cleanup(curl);

	return NULL;
}

static void workio_cmd_free(struct workio_cmd *wc)
{
	if (!wc)
		return;

	memset(wc, 0, sizeof(*wc));
	free(wc);
}

static bool add_submit_req(struct work *work, enum submit_commands req_type) {
	uint32_t *hash32 = (uint32_t *)work->announcement_hash;
	uint32_t *mult32 = (uint32_t *)work->multiplicator;

	pthread_mutex_lock(&submit_lock);

	g_submit_req = realloc(g_submit_req, (g_submit_req_cnt + 1) * sizeof(struct submit_req));
	if (!g_submit_req) {
		g_submit_req_cnt = 0;
		applog(LOG_ERR, "ERROR: Bounty request allocation failed");
		return false;
	}
	g_submit_req[g_submit_req_cnt].thr_id = work->thr_id;
	g_submit_req[g_submit_req_cnt].req_type = req_type;
	g_submit_req[g_submit_req_cnt].start_tm = time(NULL);
	g_submit_req[g_submit_req_cnt].delay_tm = 0;
	g_submit_req[g_submit_req_cnt].retries = 0;
	sprintf(g_submit_req[g_submit_req_cnt].work_id, "%llu", work->work_id);
	sprintf(g_submit_req[g_submit_req_cnt].hash, "%08X%08X%08X%08X%08X%08X%08X%08X", swap32(hash32[0]), swap32(hash32[1]), swap32(hash32[2]), swap32(hash32[3]), swap32(hash32[4]), swap32(hash32[5]), swap32(hash32[6]), swap32(hash32[7]));
	sprintf(g_submit_req[g_submit_req_cnt].mult, "%08X%08X%08X%08X%08X%08X%08X%08X", swap32(mult32[0]), swap32(mult32[1]), swap32(mult32[2]), swap32(mult32[3]), swap32(mult32[4]), swap32(mult32[5]), swap32(mult32[6]), swap32(mult32[7]));
	g_submit_req_cnt++;

	pthread_mutex_unlock(&submit_lock);
	return true;
}

static bool delete_submit_req(int idx) {
	struct submit_req *req = NULL;
	int i;

	pthread_mutex_lock(&submit_lock);
	if (g_submit_req_cnt > 0) {
		req = malloc((g_submit_req_cnt - 1) * sizeof(struct submit_req));
		if (!req) {
			applog(LOG_ERR, "ERROR: Bounty request allocation failed");
			pthread_mutex_unlock(&submit_lock);
			return false;
		}
	}
	else
		free(g_submit_req);

	for (i = 0; i < idx; i++)
		memcpy(&req[i], &g_submit_req[i], sizeof(struct submit_req));

	for (i = idx + 1; i < g_submit_req_cnt; i++)
		memcpy(&req[i - 1], &g_submit_req[i], sizeof(struct submit_req));

	free(g_submit_req);
	g_submit_req = req;
	g_submit_req_cnt--;
	pthread_mutex_unlock(&submit_lock);
	return true;
}

static void *submit_thread(void *userdata) {
	struct thr_info *mythr = (struct thr_info *) userdata;
	int thr_id = mythr->id;
	int i;

	while (1) {
		for (i = 0; i < g_submit_req_cnt; i++) {

			// Remove Completed Requests
			if (g_submit_req[i].req_type == SUBMIT_COMPLETE) {
				applog(LOG_DEBUG, "DEBUG: Submit complete...deleting request");
				delete_submit_req(i);
				break;
			}

			// Remove Stale Requests After 15min
			if (time(NULL) - g_submit_req[i].start_tm >= 900) {
				applog(LOG_DEBUG, "DEBUG: Submit request timed out after 15min");
				delete_submit_req(i);
				g_bounty_timeout_cnt++;
				break;
			}

			// Check If Request Is On Hold
			if (g_submit_req[i].delay_tm >= time(NULL))
				continue;

			// Submit Request
			if (!submit_work(mythr, &g_submit_req[i])) {
				applog(LOG_ERR, "ERROR: Submit bounty request failed");
			}

		}
		sleep(1);
	}

	return NULL;
}

static void *key_monitor_thread(void *userdata)
{
	int ch, day, hour, min, sec, total_sec;
	struct timeval now;

	while (true)
	{
		sleep(1);
		ch = getchar();
		ch = toupper(ch);
		if (ch == '\n')
			continue;

		switch (ch)
		{
		case 'S':
		{
			gettimeofday(&now, NULL);
			total_sec = now.tv_sec - g_miner_start_time.tv_sec;
			day = total_sec / 3600 / 24;
			hour = total_sec / 3600 - day * 24;
			min = total_sec / 60 - (day * 24 + hour) * 60;
			sec = total_sec % 60;

			applog(LOG_WARNING, "************************** Mining Summary **************************");
			applog(LOG_WARNING, "Run Time: %02d Days %02d:%02d:%02d\t\tWork Name: %s", day, hour, min, sec, g_work_nm ? g_work_nm : "");
			applog(LOG_WARNING, "Bounty Accept:\t%3d\t\tWork ID:   %s", g_bounty_accepted_cnt, g_work_id ? g_work_id : "");
			applog(LOG_WARNING, "Bounty Reject:\t%3d\t\tTarget:    %s", g_bounty_rejected_cnt, g_pow_target);
			applog(LOG_WARNING, "Bounty Deprct:\t%3d", g_bounty_deprecated_cnt);
			applog(LOG_WARNING, "Bounty Timeout:\t%3d\t\tPOW Accept:  %3d", g_bounty_timeout_cnt, g_pow_accepted_cnt);
			applog(LOG_WARNING, "Bounty Error:\t%3d\t\tPOW Reject:  %3d", g_bounty_error_cnt, g_pow_rejected_cnt);
			applog(LOG_WARNING, "********************************************************************");

		}
		break;
		case 'D':
			opt_debug = !opt_debug;
			applog(LOG_WARNING, "Debug Mode: %s", opt_debug ? "On" : "Off");
			break;
		case 'E':
			opt_debug_epl = !opt_debug_epl;
			applog(LOG_WARNING, "Debug ElasticPL Mode: %s", opt_debug_epl ? "On" : "Off");
			break;
		case 'P':
			opt_protocol = !opt_protocol;
			applog(LOG_WARNING, "Protocol Mode: %s", opt_protocol ? "On" : "Off");
			break;
		case 'Q':
			opt_quiet = !opt_quiet;
			applog(LOG_WARNING, "Quiet Mode: %s", opt_quiet ? "On" : "Off");
			break;
		case 'V':
			opt_debug_vm = !opt_debug_vm;
			applog(LOG_WARNING, "Debug VM Mode: %s", opt_debug_vm ? "On" : "Off");
			break;
		}
	}
	return 0;
}

#ifndef WIN32
static void signal_handler(int sig)
{
	switch (sig) {
	case SIGHUP:
		applog(LOG_INFO, "SIGHUP received");
		break;
	case SIGINT:
		applog(LOG_INFO, "SIGINT received, exiting");
		exit(0);
		break;
	case SIGTERM:
		applog(LOG_INFO, "SIGTERM received, exiting");
		exit(0);
		break;
	}
}
#else
BOOL WINAPI ConsoleHandler(DWORD dwType)
{
	switch (dwType) {
	case CTRL_C_EVENT:
		applog(LOG_INFO, "CTRL_C_EVENT received, exiting");
		exit(0);
		break;
	case CTRL_BREAK_EVENT:
		applog(LOG_INFO, "CTRL_BREAK_EVENT received, exiting");
		exit(0);
		break;
	default:
		return false;
	}
	return true;
}
#endif

static int thread_create(struct thr_info *thr, void* func)
{
	int err = 0;
	pthread_attr_init(&thr->attr);
	err = pthread_create(&thr->pth, &thr->attr, func, thr);
	pthread_attr_destroy(&thr->attr);
	return err;
}

int main(int argc, char **argv) {
	struct thr_info *thr;
	int i, err, thr_idx;

	fprintf(stdout, "** " PACKAGE_NAME " " PACKAGE_VERSION " **\n");

	pthread_mutex_init(&applog_lock, NULL);

#if defined(WIN32)
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	num_cpus = sysinfo.dwNumberOfProcessors;
#elif defined(_SC_NPROCESSORS_CONF)
	num_cpus = sysconf(_SC_NPROCESSORS_CONF);
#elif defined(CTL_HW) && defined(HW_NCPU)
	int req[] = { CTL_HW, HW_NCPU };
	size_t len = sizeof(num_cpus);
	sysctl(req, 2, &num_cpus, &len, NULL, 0);
#else
	num_cpus = 1;
#endif
	if (num_cpus < 1)
		num_cpus = 1;

	// Process Command Line Before Starting Any Threads
	parse_cmdline(argc, argv);

	if (!opt_n_threads)
		opt_n_threads = num_cpus;

	if (!rpc_url)
		rpc_url = strdup("http://127.0.0.1:6876/nxt");

	if (rpc_user && rpc_pass) {
		rpc_userpass = (char*)malloc(strlen(rpc_user) + strlen(rpc_pass) + 2);
		if (!rpc_userpass)
			return 1;
		sprintf(rpc_userpass, "%s:%s", rpc_user, rpc_pass);
	}

	if (!opt_test_compiler && !publickey) {
		applog(LOG_ERR, "ERROR: Public Key (option -k) is required");
		return 1;
	}

	// Seed Random Number Generator
	init_genrand((unsigned long)time(NULL));

#ifndef WIN32
	/* Always catch Ctrl+C */
	signal(SIGINT, signal_handler);
#else
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
#endif

	pthread_mutex_init(&work_lock, NULL);
	pthread_mutex_init(&submit_lock, NULL);

	work_restart = (struct work_restart*) calloc(opt_n_threads, sizeof(*work_restart));
	if (!work_restart)
		return 1;

	thr_info = (struct thr_info*) calloc(opt_n_threads + 3, sizeof(*thr));
	if (!thr_info)
		return 1;

	// Init workio Thread Info
	work_thr_id = opt_n_threads;
	thr = &thr_info[work_thr_id];
	thr->id = work_thr_id;
	thr->q = tq_new();
	if (!thr->q)
		return 1;

	// Start workio Thread
	if (thread_create(thr, workio_thread)) {
		applog(LOG_ERR, "work thread create failed");
		return 1;
	}

	// In Test Compiler Mode, Run Parser / Complier Using Source Code From Test File
	if (opt_test_compiler) {
		thr = &thr_info[0];
		thr->id = 0;
		thr->q = tq_new();
		if (!thr->q)
			return 1;
		if (thread_create(thr, test_compiler_thread)) {
			applog(LOG_ERR, "Test VM thread create failed!");
			return 1;
		}

		pthread_join(thr_info[work_thr_id].pth, NULL);
		free(test_filename);
		return 0;
	}

	applog(LOG_INFO, "Attempting to start %d miner threads", opt_n_threads);
	thr_idx = 0;

	// Otherwise, Start Mining Threads
	for (i = 0; i < opt_n_threads; i++) {
		thr = &thr_info[thr_idx];

		thr->id = thr_idx++;
		thr->q = tq_new();
		if (!thr->q)
			return 1;
		err = thread_create(thr, miner_thread);
		if (err) {
			applog(LOG_ERR, "CPU: %d mining thread create failed!", i);
			return 1;
		}
	}
	applog(LOG_INFO, "%d mining threads started", opt_n_threads);
	gettimeofday(&g_miner_start_time, NULL);

	// Bounty Submit Thread
	thr = &thr_info[opt_n_threads + 1];
	thr->id = opt_n_threads + 1;
	thr->q = tq_new();
	if (!thr->q)
		return 1;
	if (thread_create(thr, submit_thread)) {
		applog(LOG_ERR, "Bounty submit thread create failed");
		return 1;
	}

	// Start Key Monitor Thread
	thr = &thr_info[opt_n_threads + 2];
	thr->id = opt_n_threads + 2;
	thr->q = tq_new();
	if (!thr->q)
		return 1;
	if (thread_create(thr, key_monitor_thread)) {
		applog(LOG_ERR, "Key monitor thread create failed");
		return 1;
	}

	// Main Loop - Wait for workio thread to exit
	pthread_join(thr_info[work_thr_id].pth, NULL);

	applog(LOG_WARNING, "Exiting " PACKAGE_NAME);

	free(test_filename);
	return 0;
}