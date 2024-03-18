//
// Created by nepture on 2024/3/16.
//

#include "../include/sylar.h"

extern sylar::Logger::ptr g_logger;

void test2() {
    std::cout << sylar::BacktraceToString() << std::endl;
}
void test1() {
    test2();
}

void test_backtrace() {
    test1();
}

int test_util() {
    SYLAR_LOG_INFO(g_logger) << sylar::GetCurrentMS();
    SYLAR_LOG_INFO(g_logger) << sylar::GetCurrentUS();
    SYLAR_LOG_INFO(g_logger) << sylar::ToUpper("hello");
    SYLAR_LOG_INFO(g_logger) << sylar::ToLower("HELLO");
    SYLAR_LOG_INFO(g_logger) << sylar::Time2Str();
    SYLAR_LOG_INFO(g_logger) << sylar::Str2Time("1970-01-01 00:00:00"); // -28800

    std::vector<std::string> files;
    sylar::FSUtil::ListAllFile(files, "./", ".cpp");
    for (auto &i : files) {
        SYLAR_LOG_INFO(g_logger) << i;
    }

    // todo, more...

    test_backtrace();

    SYLAR_ASSERT2(false, "assert222");
    return 0;
}