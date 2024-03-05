#ifndef __INCLUDE_LOG_H__
#define __INCLUDE_LOG_H__

#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdarg>
#include <list>
#include <map>
#include "util.h"
#include "mutex.h"
#include "singleton.h"

namespace sylar
{
    /**
     * @brief 日志级别
     */
    class LogLevel
    {
    private:
    public:
        LogLevel(/* args */);
        ~LogLevel();

        enum Level
        {
            /// 致命情况，系统不可用
            FATAL = 0,
            /// 高优先级情况，例如数据库系统崩溃
            ALERT = 100,
            /// 严重错误，例如硬盘错误
            CRIT = 200,
            /// 错误
            ERROR = 300,
            /// 警告
            WARN = 400,
            /// 正常但值得注意
            NOTICE = 500,
            /// 一般信息
            INFO = 600,
            /// 调试信息
            DEBUG = 700,
            /// 未设置
            NOTSET = 800,
        };

        /**
         * @brief 日志级别转字符串
         * @param[in] level 日志级别
         * @return 字符串形式的日志级别
         */
        static const char *toString(LogLevel::Level level);
    };

        /**
     * @brief 日志事件
     */
    class LogEvent
    {
    private:
        /* data */
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(/* args */);
        ~LogEvent();
    };

    /**
     * @brief 日志格式化
     */
    class LogFormatter
    {

    public:
        LogFormatter(/* args */);
        ~LogFormatter();

        class FormatItem
        {

        private:
            /* data */
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            // FormatItem(/* args */);
            virtual ~FormatItem();

            virtual void format(std::ostream &os, LogEvent::ptr event) = 0;
        };

    private:
        /// 日志格式模板
        std::string m_pattern;
        /// 解析后的格式模板数组
        std::vector<FormatItem::ptr> m_items;
        /// 是否出错
        bool m_error = false;
    };

    /**
     * @brief 日志输出地，虚基类，用于派生出不同的LogAppender
     */
    class LogAppender
    {
    private:
        /* data */
    public:
        LogAppender(/* args */);
        ~LogAppender();
    };

    class StdoutLogAppender : public LogAppender
    {
    private:
        /* data */
    public:
        StdoutLogAppender(/* args */);
        ~StdoutLogAppender();
    };

    class FileLogAppender : public LogAppender
    {
    private:
        /* data */
    public:
        FileLogAppender(/* args */);
        ~FileLogAppender();
    };

    /**
     * @brief 日志器类
     */
    class Logger
    {
    private:
        /* data */
    public:
        Logger(/* args */);
        ~Logger();
    };

    /**
     * @brief 日志事件包装器，方便宏定义，内部包含日志事件和日志器
     * @details 栈上自动创建和析构，完成日志输出
     */
    class LogEventWrap
    {
    private:
        /* data */
    public:
        LogEventWrap(/* args */);
        ~LogEventWrap();
    };

    class LoggerManager
    {
    private:
        /* data */
    public:
        LoggerManager(/* args */);
        ~LoggerManager();
    };

} // namespace sylar

#endif