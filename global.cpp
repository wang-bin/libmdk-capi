/*
 * Copyright (c) 2019-2020 WangBin <wbsecg1 at gmail.com>
 */
#include "mdk/c/global.h"
#include "mdk/global.h"
using namespace MDK_NS;
extern "C" {
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
    SetGlobalOption(key, value);
}

void MDK_setGlobalOptionInt32(const char* key, int value)
{
    SetGlobalOption(key, value);
}

void MDK_setGlobalOptionPtr(const char* key, void* value)
{
    SetGlobalOption(key, value);
}
} // extern "C"