//
// Created by nepture on 2024/3/28.
//

#include "../include/sylar.h"
#include <memory>
#include <string>
#include <vector>

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber2() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber2 begin";
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber2 end";
}

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";

    SYLAR_LOG_INFO(g_logger) << "before run_in_fiber yield";
    sylar::Fiber::getThis()->yield();
    SYLAR_LOG_INFO(g_logger) << "after run_in_fiber yield";

    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";
    // fiber结束之后会自动返回主协程运行
}

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "test_fiber begin";

    // 初始化线程主协程
    sylar::Fiber::getThis();

    sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber, 0));
    SYLAR_LOG_INFO(g_logger) << "use_count:" << fiber.use_count(); // 1

    SYLAR_LOG_INFO(g_logger) << "before test_fiber resume";
    fiber->resume();
    SYLAR_LOG_INFO(g_logger) << "after test_fiber resume";


    SYLAR_LOG_INFO(g_logger) << "use_count:" << fiber.use_count();

    SYLAR_LOG_INFO(g_logger) << "fiber status: " << fiber->getState(); // READY

    SYLAR_LOG_INFO(g_logger) << "before test_fiber resume again";
    fiber->resume();
    SYLAR_LOG_INFO(g_logger) << "after test_fiber resume again";

    SYLAR_LOG_INFO(g_logger) << "use_count:" << fiber.use_count(); // 1
    SYLAR_LOG_INFO(g_logger) << "fiber status: " << fiber->getState(); // TERM

    fiber->resetHandler(run_in_fiber2); // 上一个协程结束之后，复用其栈空间再创建一个新协程
    fiber->resume();

    SYLAR_LOG_INFO(g_logger) << "use_count:" << fiber.use_count(); // 1
    SYLAR_LOG_INFO(g_logger) << "test_fiber end";
}

int test_fiber_main_func(int argc, char *argv[]) {
    sylar::EnvMgr::GetInstance()->init(argc, argv);
    sylar::Config::LoadFromConfDir(sylar::EnvMgr::GetInstance()->getConfigPath());

    sylar::SetThreadName("main_thread");
    SYLAR_LOG_INFO(g_logger) << "main begin";

    std::vector<sylar::Thread::ptr> thrs;
    for (int i = 0; i < 2; i++) {
        thrs.push_back(std::make_shared<sylar::Thread>(
                &test_fiber, "thread_" + std::to_string(i)));
    }

    for (const auto& i : thrs) {
        i->join();
    }

    SYLAR_LOG_INFO(g_logger) << "main end";
    return 0;
}