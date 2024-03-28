//
// Created by nepture on 2024/3/28.
//

#ifndef SYLAR_FIBER_H
#define SYLAR_FIBER_H

#include <functional>
#include <memory>
#include <ucontext.h>
#include "thread.h"

namespace sylar{
class Fiber:public std::enable_shared_from_this<Fiber>{
    public:
        enum State{
            READY,
            RUNNING,
            STOPPED
        };

        typedef std::shared_ptr<Fiber> ptr;

        Fiber(std::function<void()> handler,uint64_t stack_size);

        ~Fiber();
        /**
         * @brief 重置协程状态和入口函数，复用栈空间，不重新创建栈
         */
        void resetHandler(std::function<void()> handler);
        /**
         * @brief 将当前协程切到到执行状态
         * @details 当前协程和正在运行的协程进行交换，调用者状态变为RUNNING，当前运行者状态变为READY
         */
        void resume();
        /**
         * @brief 当前协程让出执行权
         * @details 当前协程与上次resume时退到后台的协程进行交换，前者状态变为READY，后者状态变为RUNNING
         */
        void yield();

        uint64_t getId();

        State getState();

        static void setRunningFiber(Fiber* f);
        /**
         * @brief 返回当前线程正在执行的协程
         * @details 如果当前线程还未创建协程，则创建线程的第一个协程，
         * 且该协程为当前线程的主协程，其他协程都通过这个协程来调度，也就是说，其他协程
         * 结束时,都要切回到主协程，由主协程重新选择新的协程进行resume
         * @attention 线程如果要创建协程，那么应该首先执行一下Fiber::GetThis()操作，以初始化主函数协程
         */
        static Fiber::ptr getThis();

        static uint64_t totalFibers();

        static void mainFunc();

        static uint64_t getRunId();
    private:
        uint64_t f_id;
        uint64_t f_stack_size;
        State f_state = READY;
        ucontext_t f_ctx{0};
        void* f_stack_ptr = nullptr;
        std::function<void()> f_handler;
        Fiber();
    };

}

#endif //SYLAR_FIBER_H
