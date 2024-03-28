//
// Created by nepture on 2024/3/28.
//
#include <utility>

#include "../../include/fiber.h"
#include "../../include/log.h"
#include "atomic"
#include "../../include/config.h"
#include "../../include/macro.h"

namespace sylar{
    static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
    static std::atomic<uint64_t> next_fiber_id {1};
    static std::atomic<uint64_t> total_fibers{0};
    
    static thread_local Fiber* running_fiber;
    static thread_local std::shared_ptr<Fiber> parent_fiber;
    
    static ConfigVar<uint64_t>::ptr config_stack_size = Config::Lookup<uint64_t>("fiber.stack_size",
                                                                                 256*1024,
                                                                       "fiber.stack_size");

    /**
     * @brief malloc栈内存分配器
     */
    class MallocStackAllocator{
    public:
         static void* alloc(size_t size){
             return calloc(size,1);
         }
         static void dealloc(void* p){
             free(p);
         }
    };

    //using StackAllocator = MallocStackAllocator;
    typedef MallocStackAllocator StackAllocator;

    Fiber::Fiber(std::function<void()> handler, uint64_t stack_size): f_id(next_fiber_id++),f_handler(std::move(handler)) {
        total_fibers++;
        f_stack_size = stack_size>0 ? stack_size:config_stack_size->getValue();
        f_stack_ptr = StackAllocator ::alloc(f_stack_size);

        if (getcontext(&f_ctx)){
            SYLAR_ASSERT2(false, "getcontext");
        }

        f_ctx.uc_link= nullptr;
        f_ctx.uc_stack.ss_sp = f_stack_ptr;
        f_ctx.uc_stack.ss_size = f_stack_size;

        makecontext(&f_ctx,Fiber::mainFunc,0);

        SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber() id = " << f_id;
    }

    Fiber::~Fiber() {
        SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber() id = " << f_id;
        if (f_stack_ptr){
            StackAllocator::dealloc(f_stack_ptr);
            f_state = STOPPED;
            SYLAR_LOG_DEBUG(g_logger) << "dealloc stack, id = " << f_id << "stack_size: "<< f_stack_size;
        }else{
            if (running_fiber== this){
                running_fiber= nullptr;
                SYLAR_LOG_INFO(g_logger) << "main fiber destructed";
            }else{
                SYLAR_LOG_WARN(g_logger) << "main fiber ~Fiber occur some error";
            }
        }
    }

    void Fiber::resetHandler(std::function<void()> handler) {
        SYLAR_ASSERT(f_stack_ptr)
        SYLAR_ASSERT(f_state==STOPPED)
        f_handler=std::move(handler);
        if (getcontext(&f_ctx)) {
            SYLAR_ASSERT2(false, "getcontext");
        }
        f_ctx.uc_stack.ss_size=f_stack_size;
        f_ctx.uc_stack.ss_sp=f_stack_ptr;
        f_ctx.uc_link= nullptr;

        makecontext(&f_ctx,Fiber::mainFunc,0);
        f_state=READY;
    }

    void Fiber::resume() {
        SYLAR_ASSERT(f_state==READY)
        running_fiber= this;
        f_state=RUNNING;

        if (swapcontext(&parent_fiber->f_ctx,&f_ctx)){
            SYLAR_ASSERT2(false, "resume swapcontext error");
        }
    }

    void Fiber::yield() {
        SYLAR_ASSERT(f_state!=READY)
        running_fiber=parent_fiber.get();
        if (f_state==RUNNING){
            f_state=READY;
        }
        if (swapcontext(&f_ctx,&parent_fiber->f_ctx)){
            SYLAR_ASSERT2(false, "swapcontext error yield");
        }
    }

    uint64_t Fiber::getId() {
        return f_id;
    }

    Fiber::State Fiber::getState() {
        return f_state;
    }

    void Fiber::setRunningFiber(Fiber* f) {
        running_fiber = f;
    }

    Fiber::ptr Fiber::getThis() {
        if (running_fiber){
            return running_fiber->shared_from_this();
        }

        Fiber::ptr f(new Fiber());
        SYLAR_ASSERT(running_fiber==f.get())
        parent_fiber=f;
        return running_fiber->shared_from_this();
    }

    uint64_t Fiber::totalFibers() {
        return total_fibers;
    }

    void Fiber::mainFunc() {
        Fiber::ptr current = getThis();
        current->f_handler();
        current->f_handler= nullptr;
        current->f_state=STOPPED;
        auto raw_ptr = current.get();
        current.reset();
        raw_ptr->yield();
    }

    uint64_t Fiber::getRunId() {
        if (running_fiber){
            return running_fiber->f_id;
        }
        return 0;
    }

    Fiber::Fiber() {
        setRunningFiber(this);
        f_state = RUNNING;
        if (getcontext(&f_ctx)){
            SYLAR_ASSERT2(false, "getcontext");
        }
        total_fibers++;
        f_id = next_fiber_id++;

        SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber(no args) main id = " << f_id;
    }
}


