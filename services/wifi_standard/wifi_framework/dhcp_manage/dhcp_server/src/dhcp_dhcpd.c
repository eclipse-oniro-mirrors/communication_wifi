/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dhcp_server.h"
#include "dhcp_define.h"
#include "dhcp_logger.h"
#include "dhcp_ipv4.h"
#include "hash_table.h"
#include "address_utils.h"
#include "securec.h"
#include "dhcp_config.h"

#undef LOG_TAG
#define LOG_TAG "DhcpServerMain"

#define NO_ARG 0
#define REQUIRED_ARG 1
#define OPTIONAL_ARG 2

#define ARGUMENT_NAME_SIZE 32
#define ARGUMENT_VALUE_SIZE 256
#define INIT_ARGS_SIZE 4
#define USAGE_DESC_MAX_LENGTH 32
#define DEFAUTL_NET_MASK "255.255.255.0"

static HashTable g_argumentsTable;
static DhcpConfig g_dhcpConfig;

static PDhcpServerContext g_dhcpServer = 0;

typedef struct DhcpUsage DhcpUsage;
struct DhcpUsage {
    struct option *opt;
    const char *params;
    const char *desc;
    const char *example;
    int required;
    int (*dealOption)(const char *, const char *);
};

typedef struct ArgumentInfo ArgumentInfo;
struct ArgumentInfo {
    char name[ARGUMENT_NAME_SIZE];
    char value[ARGUMENT_VALUE_SIZE];
};

enum SignalEvent {
    EXIT = 0,
    RELOAD,
    RESTART,
};

static struct option longOptions[] = {
    {"ifname", REQUIRED_ARG, 0, 'i'},
    {"conf", REQUIRED_ARG, 0, 'c'},
    {"dns", REQUIRED_ARG, 0, 'd'},
    {"gateway", REQUIRED_ARG, 0, 'g'},
    {"server", REQUIRED_ARG, 0, 's'},
    {"netmask", REQUIRED_ARG, 0, 'n'},
    {"pool", REQUIRED_ARG, 0, 'P'},
    {"lease", REQUIRED_ARG, 0, 0},
    {"renewal", REQUIRED_ARG, 0, 0},
    {"rebinding", REQUIRED_ARG, 0, 0},
    {"version", NO_ARG, 0, 'v'},
    {"help", NO_ARG, 0, 'h'},
    {0, 0, 0, 0},
};

typedef struct DhcpOptionField {
    const char *field;
    int code;
    const char *explain;
    const char *format;
    int supported;
    int (*parseOption)(DhcpOption *, int, char *);

} DhcpOptionField;

static int InitArguments(void)
{
    if (CreateHashTable(&g_argumentsTable, ARGUMENT_NAME_SIZE, sizeof(ArgumentInfo), INIT_ARGS_SIZE) != HASH_SUCCESS) {
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

static void FreeArguments(void)
{
    if (!Initialized(&g_argumentsTable)) {
        return;
    }
    DestroyHashTable(&g_argumentsTable);
}

static int HasArgument(const char *argument)
{
    char name[ARGUMENT_NAME_SIZE] = {'\0', 0};
    if (!argument) {
        return 0;
    }
    size_t ssize = strlen(argument);
    if (ssize > ARGUMENT_NAME_SIZE) {
        ssize = ARGUMENT_NAME_SIZE;
    }
    if (memcpy_s(name, ARGUMENT_NAME_SIZE, argument, ssize) != EOK) {
        LOGE("failed to set argument name.");
        return 0;
    }
    if (ContainsKey(&g_argumentsTable, (uintptr_t)name)) {
        return 1;
    }
    return 0;
}

static ArgumentInfo *GetArgument(const char *name)
{
    char argName[ARGUMENT_NAME_SIZE] = {'\0'};
    size_t ssize = strlen(name);
    if (ssize > ARGUMENT_NAME_SIZE) {
        ssize = ARGUMENT_NAME_SIZE;
    }
    if (memcpy_s(argName, ARGUMENT_NAME_SIZE, name, ssize) != EOK) {
        LOGE("failed to set argument name.");
        return NULL;
    }
    if (ContainsKey(&g_argumentsTable, (uintptr_t)argName)) {
        ArgumentInfo *arg = (ArgumentInfo *)At(&g_argumentsTable, (uintptr_t)argName);
        return arg;
    }
    return NULL;
}

static int DefaultArgument(const char *argument, const char *val)
{
    LOGD("Input argument is: [%s], value is [%s]", (argument == NULL) ? "" : argument,
        (val == NULL) ? "" : val);
    return RET_SUCCESS;
}

static int PutArgument(const char *argument, const char *val)
{
    if (!argument) {
        return 0;
    }
    if (!val) {
        return RET_FAILED;
    }

    if (HasArgument(argument)) {
        return RET_FAILED;
    }

    ArgumentInfo arg;
    size_t ssize = strlen(argument);
    if (ssize >= ARGUMENT_NAME_SIZE) {
        ssize = ARGUMENT_NAME_SIZE -1;
    }
    size_t vlen = strlen(val);
    if (memset_s(arg.name, ARGUMENT_NAME_SIZE, '\0', ARGUMENT_NAME_SIZE) != EOK) {
        LOGE("failed to reset argument name.");
        return RET_ERROR;
    }
    if (memcpy_s(arg.name, ARGUMENT_NAME_SIZE, argument, ssize) != EOK) {
        LOGE("failed to set argument name.");
        return RET_ERROR;
    }
    if (vlen < 0) {
        return RET_ERROR;
    }
    if (vlen >= ARGUMENT_VALUE_SIZE) {
        LOGE("value string too long.");
        return RET_ERROR;
    }
    if (memset_s(arg.value, ARGUMENT_VALUE_SIZE, '\0', ARGUMENT_NAME_SIZE) != EOK) {
        LOGE("failed to reset argument value.");
        return RET_ERROR;
    }
    if (memcpy_s(arg.value, ARGUMENT_VALUE_SIZE, val, vlen) != EOK) {
        LOGE("failed to set argument value.");
        return RET_ERROR;
    }
    int ret = Insert(&g_argumentsTable, (uintptr_t)arg.name, (uintptr_t)&arg);
    if (ret == HASH_INSERTED) {
        return RET_SUCCESS;
    }
    return RET_FAILED;
}

static int PutIpArgument(const char *argument, const char *val)
{
    if (!ParseIpAddr(val)) {
        LOGE("%s format error.", argument);
        return RET_FAILED;
    }
    return PutArgument(argument, val);
}

static int PutPoolArgument(const char *argument, const char *val)
{
    if (!val) {
        return 0;
    }
    if (strchr(val, ',') == NULL) {
        LOGE("too few pool option arguments.");
        return RET_FAILED;
    }
    return PutArgument(argument, val);
}

static int ShowVersion(const char *argument, const char *val)
{
    if (argument && PutArgument(argument, val) != RET_SUCCESS) {
        LOGD("failed to put argument 'version'.");
    }
    printf("version:%s\n", DHCPD_VERSION);
    return RET_BREAK;
}

const char *optionString = "i:c:d:g:s:n:P:S:Bp:o:lb:rvhD";
static DhcpUsage usages[] = {
    {&longOptions[NUM_ZERO], "<interface>", "network interface name.", "--ifname eth0", 1, PutArgument},
    {&longOptions[NUM_ONE], "<file>", "configure file name.", "--conf /etc/conf/dhcp_server.conf", 0, PutArgument},
    {&longOptions[NUM_TWO], "<dns1>[,dns2][,dns3][...]", "domain name server IP address list.", "", 0, PutArgument},
    {&longOptions[NUM_THREE], "<gateway>", "gateway option.", "", 0, PutIpArgument},
    {&longOptions[NUM_FOUR], "<server>", "server identifier.", "", 1, PutIpArgument},
    {&longOptions[NUM_FIVE], "<netmask>", "default subnet mask.", "", 1, PutIpArgument},
    {&longOptions[NUM_SIX], "<beginip>,<endip>", "pool address range.", "", 0,
        PutPoolArgument},
    {&longOptions[NUM_SEVEN], "<leaseTime>", "set lease time value, the value is in units of seconds.", "", 0,
        PutArgument},
    {&longOptions[NUM_EIGHT], "<renewalTime>", "set renewal time value, the value is in units of seconds.", "", 0,
        PutArgument},
    {&longOptions[NUM_NINE], "<rebindingTime>", "set rebinding time value, the value is in units of seconds.", "", 0,
        PutArgument},
    {&longOptions[NUM_TEN], "", "show version information.", "", 0, ShowVersion},
    {&longOptions[NUM_ELEVEN], "", "show help information.", "", 0, DefaultArgument},
    {0, "", "", ""},
};

int findIndex(int c)
{
    int size = sizeof(longOptions) / sizeof(longOptions[0]);
    for (int i = 0; i < size; ++i) {
        if (longOptions[i].val == c) {
            return i;
        }
    }
    return -1;
}

void ShowUsage(const DhcpUsage *usage)
{
    if (!usage) {
        return;
    }
    if (usage->opt->val) {
        printf("-%c,--%s ", usage->opt->val, usage->opt->name);
    } else {
        printf("   --%s ", usage->opt->name);
    }
    if (usage->params[0] == '\0') {
        printf("\t%s\n", usage->desc);
    } else {
        int plen = strlen(usage->params) + strlen(usage->params);
        if (plen < USAGE_DESC_MAX_LENGTH) {
            printf("\t\t%s\t\t%s\n", usage->params, usage->desc);
        } else {
            printf("\t\t%s\n", usage->params);
            printf("\t\t\t%s\n\n", usage->desc);
        }
    }
}

void PrintRequiredArguments(void)
{
    size_t argc = sizeof(usages) / sizeof(DhcpUsage);
    printf("required parameters:");
    int idx = 0;
    for (size_t i = 0; i < argc; i++) {
        DhcpUsage usage = usages[i];
        if (!usage.opt) {
            break;
        }
        if (usage.required) {
            if (idx == 0) {
                printf("\"%s\"", usage.opt->name);
            } else {
                printf(", \"%s\"", usage.opt->name);
            }
            idx++;
        }
    }
    printf(".\n\n");
    printf("Usage: dhcp_server [options] \n");
    printf("e.g: dhcp_server -i eth0 -c /data/dhcp/dhcp_server.conf \n");
    printf("     dhcp_server --help \n\n");
}

void PrintUsage(void)
{
    printf("Usage: dhcp_server [options] \n\n");

    size_t argc = sizeof(usages) / sizeof(DhcpUsage);
    for (size_t i = 0; i < argc; i++) {
        DhcpUsage usage = usages[i];
        if (!usage.opt) {
            break;
        }
        ShowUsage(&usage);
    }
    printf("\n");
}

void ShowHelp(int argc)
{
    if (argc == NUM_TWO) {
        PrintUsage();
        return;
    }
}

int ParseArguments(int argc, char *argv[])
{
    int ret;
    opterr = 0;
    size_t optsc = sizeof(usages) / sizeof(DhcpUsage);
    int index = -1;
    int rst = RET_SUCCESS;

    while ((ret = getopt_long(argc, argv, optionString, longOptions, &index)) != -1) {
        if (ret == '?') {
            LOGW("unknown input arguments! ret = ?");
            index = -1;
            continue;
        }
        if (index < 0) {
            index = findIndex(ret);
        }
        if (index < 0 || index >= (int)optsc) {
            LOGD("unknown input arguments! ret = %c, index = %d", ret, index);
            index = -1;
            continue;
        }
        DhcpUsage *usage = &usages[index];
        rst = usage->dealOption(usage->opt->name, optarg);
        if (rst != RET_SUCCESS) {
            break;
        }
        index = -1;
    }


    return rst;
}

void LoadLocalConfig(DhcpAddressPool *pool)
{
    LOGD("loading local configure ...");
}

void ReloadLocalConfig(DhcpAddressPool *pool)
{
    LOGD("reloading local configure ...");
}

static int InitNetworkAbout(DhcpConfig *config)
{
    ArgumentInfo *arg = GetArgument("netmask");
    if (arg) {
        LOGI("subnet mask:%s", arg->value);
        uint32_t argNetmask = ParseIpAddr(arg->value);
        if (argNetmask) {
            config->netmask = argNetmask;
        } else {
            LOGW("error netmask argument.");
            return RET_FAILED;
        }
    } else {
        if (!config->netmask) {
            config->netmask = ParseIpAddr(DEFAUTL_NET_MASK);
        }
    }
    arg = GetArgument("gateway");
    if (arg) {
        LOGI("gateway:%s", arg->value);
        uint32_t argGateway = ParseIpAddr(arg->value);
        if (argGateway) {
            config->gateway = argGateway;
        } else {
            LOGE("error gateway argument.");
            return RET_FAILED;
        }
    }
    return RET_SUCCESS;
}

static int PareseAddreesRange(DhcpConfig *config)
{
    ArgumentInfo *arg = GetArgument("pool");
    if (arg) {
        LOGD("pool info:%s", arg->value);
        int index = 0;
        char *src = arg->value;
        char *delim = ",";
        char *poolPartArg;
        poolPartArg = strtok(src, delim);
        while (poolPartArg) {
            if (index == 0) {
                config->pool.beginAddress = ParseIpAddr(poolPartArg);
                LOGD("address range begin of: %s", poolPartArg);
            } else if (index == 1) {
                config->pool.endAddress = ParseIpAddr(poolPartArg);
                LOGD("address range end of: %s", poolPartArg);
            }
            index++;
            poolPartArg = strtok(NULL, delim);
        }
        if (!config->pool.beginAddress || !config->pool.endAddress) {
            LOGE("'pool' argument format error.");
            return RET_FAILED;
        }
        return RET_SUCCESS;
    }
    LOGW("failed to get 'pool' argument.");
    return RET_FAILED;
}

static int InitAddressRange(DhcpConfig *config)
{
    if (HasArgument("pool")) {
        if (PareseAddreesRange(config) != RET_SUCCESS) {
            LOGW("dhcp range config error.");
            return RET_FAILED;
        }
    } else {
        if (!config->pool.beginAddress || !config->pool.endAddress) {
            config->pool.beginAddress = FirstIpAddress(config->serverId, config->netmask);
            config->pool.endAddress = LastIpAddress(config->serverId, config->netmask);
        }
    }
    return RET_SUCCESS;
}

static int InitDomainNameServer(DhcpConfig *config)
{
    ArgumentInfo *arg = GetArgument("dns");
    if (arg) {
        DhcpOption argOpt = {DOMAIN_NAME_SERVER_OPTION, 0, {0}};

        char *pSave = NULL;
        char *pTok = strtok_r(arg->value, ",", &pSave);
        if ((pTok == NULL) || (strlen(pTok) == 0)) {
            LOGE("strtok_r pTok NULL or len is 0!");
            return RET_FAILED;
        }
        uint32_t dnsAddress;
        while (pTok != NULL) {
            if ((dnsAddress = ParseIpAddr(pTok)) == 0) {
                LOGE("ParseIpAddr %s failed, code:%d", pTok, argOpt.code);
                return RET_FAILED;
            }
            if (AppendAddressOption(&argOpt, dnsAddress) != RET_SUCCESS) {
                LOGW("failed to append dns option.");
            };
            pTok = strtok_r(NULL, ",", &pSave);
        }

        if (GetOption(&config->options, argOpt.code) != NULL) {
            RemoveOption(&config->options, DOMAIN_NAME_SERVER_OPTION);
        }
        PushBackOption(&config->options, &argOpt);
    }
    return RET_SUCCESS;
}

static int InitServerId(DhcpConfig *config)
{
    ArgumentInfo *arg = GetArgument("server");
    if (arg) {
        LOGI("server id:%s", arg->value);
        uint32_t argServerId = ParseIpAddr(arg->value);
        if (argServerId) {
            config->serverId = argServerId;
        } else {
            LOGE("error server argument.");
            return RET_FAILED;
        }
    } else {
        if (!config->serverId) {
            LOGE("failed to get 'server' argument or config item.");
            return RET_FAILED;
        }
    }
    return RET_SUCCESS;
}

static int InitLeaseTime(DhcpConfig *config)
{
    ArgumentInfo *arg = GetArgument("lease");
    if (arg) {
        config->leaseTime = atoi(arg->value);
    } else {
        if (!config->leaseTime) {
            config->leaseTime = DHCP_LEASE_TIME;
        }
    }
    return RET_SUCCESS;
}

static int InitRenewalTime(DhcpConfig *config)
{
    ArgumentInfo *arg = GetArgument("renewal");
    if (arg) {
        config->renewalTime = atoi(arg->value);
    } else {
        if (!config->rebindingTime) {
            config->rebindingTime = DHCP_RENEWAL_TIME;
        }
        config->renewalTime = DHCP_RENEWAL_TIME;
    }
    return RET_SUCCESS;
}

static int InitRebindingTime(DhcpConfig *config)
{
    ArgumentInfo *arg = GetArgument("rebinding");
    if (arg) {
        config->rebindingTime = atoi(arg->value);
    } else {
        if (!config->rebindingTime) {
            config->rebindingTime =  DHCP_REBINDING_TIME;
        }
    }
    return RET_SUCCESS;
}
static int InitConfigByArguments(DhcpConfig *config)
{
    if (!config) {
        LOGE("dhcp configure pointer is null.");
        return RET_FAILED;
    }
    if (InitServerId(config) != RET_SUCCESS) {
        return RET_FAILED;
    }
    if (InitNetworkAbout(config) != RET_SUCCESS) {
        return RET_FAILED;
    }
    if (InitAddressRange(config) != RET_SUCCESS) {
        return RET_FAILED;
    }
    if (InitLeaseTime(config) != RET_SUCCESS) {
        return RET_FAILED;
    }
    if (InitRenewalTime(config) != RET_SUCCESS) {
        return RET_FAILED;
    }
    if (InitRebindingTime(config) != RET_SUCCESS) {
        return RET_FAILED;
    }
    if (InitDomainNameServer(config) != RET_SUCCESS) {
        return RET_FAILED;
    }
    return RET_SUCCESS;
}

int ServerActionCallback(int state, int code, const char *ifname)
{
    int ret = 0;
    switch (state) {
        case ST_STARTING: {
            if (code == 0) {
                LOGD(" callback[%s] ==> server starting ...", ifname);
            } else if (code == 1) {
                LOGD(" callback[%s] ==> server started.", ifname);
            } else if (code == NUM_TWO) {
                LOGD(" callback[%s] ==> server start failed.", ifname);
            }
            break;
        }
        case ST_RELOADNG: {
            LOGD(" callback[%s] ==> reloading ...", ifname);
            break;
        }
        case ST_STOPED: {
            LOGD(" callback[%s] ==> server stoped.", ifname);
            exit(0);
            break;
        }
        default:
            break;
    }
    return ret;
}

static void SignalHandler(int signal)
{
    switch (signal) {
        case SIGTERM: {
            StopDhcpServer(g_dhcpServer);
            break;
        }
        case SIGUSR1:
            break;
        default:
            break;
    }
}

static int RegisterSignalHandle(void)
{
    if (signal(SIGTERM, SignalHandler) == SIG_ERR) {
        LOGE("RegisterSignalHandle() failed, signal SIGTERM err:%s!", strerror(errno));
        return RET_FAILED;
    }

    if (signal(SIGUSR1, SignalHandler) == SIG_ERR) {
        LOGE("RegisterSignalHandle() failed, signal SIGUSR1 err:%s!", strerror(errno));
        return RET_FAILED;
    }

    return RET_SUCCESS;
}

static int InitializeDhcpConfig(const char *ifname, DhcpConfig *config)
{
    if (!config) {
        LOGE("dhcp configure pointer is null.");
        return RET_FAILED;
    }
    if (InitOptionList(&config->options) != RET_SUCCESS) {
        LOGE("failed to initialize options.");
        return RET_FAILED;
    }
    char *configFile = DHCPD_CONFIG_FILE;
    if (HasArgument("conf")) {
        ArgumentInfo *configArg = GetArgument("conf");
        if (configArg) {
            configFile = configArg->value;
        } else {
            LOGE("failed to get config file name.");
            return RET_FAILED;
        }
    }
    LOGD("load local config file:%s", configFile);
    if (LoadConfig(configFile, ifname, config) != RET_SUCCESS) {
        LOGE("faile to load configure file.");
        return RET_FAILED;
    }
    if (InitConfigByArguments(config) != RET_SUCCESS) {
        LOGE("faile to parse arguments.");
        return RET_FAILED;
    }

    return RET_SUCCESS;
}

static void FreeLocalConfig(void)
{
    FreeOptionList(&g_dhcpConfig.options);
}

void FreeSeverResources(void)
{
    FreeArguments();
    FreeLocalConfig();
    FreeServerContex(g_dhcpServer);
}
int main(int argc, char *argv[])
{
    if (argc == 1) {
        PrintRequiredArguments();
        return 1;
    }
    if (strcmp("-h", argv[argc - 1]) == 0 || strcmp("--help", argv[argc - 1]) == 0) {
        ShowHelp(argc);
        return 0;
    }
    if (InitArguments() != RET_SUCCESS) {
        LOGE("failed to init arguments table.");
        return 1;
    }
    int ret = ParseArguments(argc, argv);
    if (ret != RET_SUCCESS) {
        FreeArguments();
        return 1;
    }
    ArgumentInfo *ifaceName = GetArgument("ifname");
    if (!ifaceName || strlen(ifaceName->value) == 0) {
        printf("missing required parameters:\"ifname\"\n");
        FreeArguments();
        return 1;
    }
    if (InitializeDhcpConfig(ifaceName->value, &g_dhcpConfig) != RET_SUCCESS) {
        LOGW("failed to initialize config.");
    }
    g_dhcpServer = InitializeServer(&g_dhcpConfig);
    if (g_dhcpServer == NULL) {
        LOGE("failed to initialize dhcp server.");
        FreeArguments();
        return 1;
    }
    if (RegisterSignalHandle() != RET_SUCCESS) {
        FreeSeverResources();
        return 1;
    }
    RegisterDhcpCallback(g_dhcpServer, ServerActionCallback);
    if (StartDhcpServer(g_dhcpServer)) {
        FreeSeverResources();
        return 1;
    }
    FreeSeverResources();
    return 0;
}