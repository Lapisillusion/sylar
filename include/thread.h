//
// Created by nepture on 2024/3/24.
//

#ifndef SYLAR_THREAD_H
#define SYLAR_THREAD_H

#include "mutex.h"

namespace sylar {
    class Thread {
    private:
        pid_t m_id = -1;
        pthread_t m_thread = 0;
        std::function<void()> m_handler;
        std::string m_name;
        Semaphore m_semaphore;

        static void* run(void *arg);
    public:
        typedef std::shared_ptr<Thread> ptr;

        Thread(const std::function<void()> &mHandler, const std::string &mName);

        virtual ~Thread();

        /**
         * @brief 线程ID
         */
        pid_t getId() const { return m_id; }

        /**
         * @brief 线程名称
         */
        const std::string &getName() const { return m_name; }

        /**
         * @brief 等待线程执行完成
         */
        void join();

        /**
         * @brief 获取当前的线程指针
         */
        static Thread *GetThis();

        /**
         * @brief 获取当前的线程名称
         */
        static const std::string &GetName();

        /**
         * @brief 设置当前线程名称
         * @param[in] name 线程名称
         */

        static void SetName(const std::string &name);
    };
}

#endif //SYLAR_THREAD_H
