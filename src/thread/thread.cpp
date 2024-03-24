//
// Created by nepture on 2024/3/24.
//
#include "../../include/thread.h"
#include "../../include/log.h"

namespace sylar{
    static thread_local Thread* t_thread= nullptr;
    static thread_local std::string t_thread_name = "default thread name";
    static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    Thread::Thread(const std::function<void()> &mHandler, const std::string &mName) : m_handler(mHandler){
        if (mName.empty()){
            m_name = "default thread name";
        } else{
            m_name = mName;
        }
        int rt = pthread_create(&m_thread, nullptr,Thread::run,this);
        if (rt) {
            SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                                      << " name=" << mName;
            throw std::logic_error("pthread_create error");
        }
        m_semaphore.wait();
    }

    Thread::~Thread() {
        if (m_thread){
            pthread_detach(m_thread);
        }
    }

    void *Thread::run(void *arg) {
        auto* thread = (Thread*)arg;
        t_thread = thread;
        t_thread_name = t_thread->m_name;
        thread->m_id = sylar::GetThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        //std::cout << pthread_self() << '\t' << thread->m_id << std::endl;
        //SYLAR_LOG_FMT_INFO(g_logger,"pid %d %d",pthread_self(),thread->m_id);

        std::function<void()> handler;
        handler.swap(thread->m_handler);

        thread->m_semaphore.notify();

        handler();
        return nullptr;
    }

    void Thread::join() {
        if (m_thread){
            int rt = pthread_join(m_thread, nullptr);
            if (rt){
                SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                                          << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread=0;
        }
    }

    Thread *Thread::GetThis() {
        return t_thread;
    }

    const std::string &Thread::GetName() {
        return t_thread_name;
    }

    void Thread::SetName(const std::string &name) {
        if (name.empty()){
            return;
        }
        if (t_thread){
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }
}


