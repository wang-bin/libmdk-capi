/*
 * Copyright (c) 2019 WangBin <wbsecg1 at gmail.com>
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

} // extern "C"