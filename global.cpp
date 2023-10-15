/*
 * Copyright (c) 2019-2023 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/global.h"
#include "mdk/global.h"
#include <string.h>
#if (_WIN32 + 0)
#include <intrin.h>
#endif
using namespace std;
using namespace MDK_NS;
extern "C" {

int MDK_version()
{
    return version();
}

void* MDK_javaVM(void* vm) {
    return javaVM(vm);
}

void MDK_setLogLevel(MDK_LogLevel value) {
    setLogLevel(LogLevel(value));
}

MDK_LogLevel MDK_logLevel() {
    return (MDK_LogLevel)logLevel();
}

void MDK_setLogHandler(mdkLogHandler h) {
    if (!h.opaque) {
       setLogHandler(nullptr);
       return;
    }
    setLogHandler([h](LogLevel value, const char* msg){
        h.cb(MDK_LogLevel(value), msg, h.opaque);
    });
}

void MDK_setGlobalOptionString(const char* key, const char* value)
{
#if (_MSC_VER + 0)
    SetGlobalOption("UserAddress", _ReturnAddress());
#else
    SetGlobalOption("UserAddress", __builtin_return_address(0));
#endif
    SetGlobalOption(key, string(value));
}

void MDK_setGlobalOptionInt32(const char* key, int value)
{
    SetGlobalOption(key, value);
}

void MDK_setGlobalOptionFloat(const char* key, float value)
{
    SetGlobalOption(key, value);
}

void MDK_setGlobalOptionPtr(const char* key, void* value)
{
    SetGlobalOption(key, value);
}

bool MDK_getGlobalOptionString(const char* key, const char** value)
{
    const auto& v = GetGlobalOption(key);
    if (auto pv = std::get_if<std::string>(&v)) {
        if (value)
            *value = pv->data();
        return true;
    }
    return false;
}

bool MDK_getGlobalOptionInt32(const char* key, int* value)
{
    const auto& v = GetGlobalOption(key);
    if (auto pv = std::get_if<int>(&v)) {
        if (value)
            *value = *pv;
        return true;
    }
    return false;
}

bool MDK_getGlobalOptionPtr(const char* key, void** value)
{
    const auto& v = GetGlobalOption(key);
    if (auto pv = std::get_if<void*>(&v)) {
        if (value)
            *value = *pv;
        return true;
    }
    return false;
}

char* MDK_strdup(const char* strSource)
{
#if defined(_MSC_VER)
    return _strdup(strSource);
#else
    return strdup(strSource);
#endif
}
} // extern "C"