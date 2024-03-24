//
// Created by nepture on 2024/3/6.
//
#include "../../include/log.h"

#include "../../include/macro.h"
namespace sylar
{
    const char *LogLevel::toString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name)         \
    case LogLevel::name: \
        return #name;
            XX(FATAL)
            XX(ALERT)
            XX(CRIT)
            XX(ERROR)
            XX(WARN)
            XX(NOTICE)
            XX(INFO)
            XX(DEBUG)
#undef XX
            default:
                return "NOTSET";
        }
        return "NOTSET";
    }

    LogLevel::Level LogLevel::toLevel(const std::string &str)
    {
#define XX(level, v)            \
    if (str == #v)              \
    {                           \
        return LogLevel::level; \
    }
        XX(FATAL, fatal)
        XX(ALERT, alert)
        XX(CRIT, crit)
        XX(ERROR, error)
        XX(WARN, warn)
        XX(NOTICE, notice)
        XX(INFO, info)
        XX(DEBUG, debug)

        XX(FATAL, FATAL)
        XX(ALERT, ALERT)
        XX(CRIT, CRIT)
        XX(ERROR, ERROR)
        XX(WARN, WARN)
        XX(NOTICE, NOTICE)
        XX(INFO, INFO)
        XX(DEBUG, DEBUG)
#undef XX

        return LogLevel::NOTSET;
    }

    LogEvent::LogEvent(const std::string &logger_name, LogLevel::Level level, const char *file, int32_t line, int64_t elapse, uint32_t thread_id, uint64_t fiber_id, time_t time, const std::string &thread_name)
            : m_level(level), m_file(file), m_line(line), m_elapse(elapse), m_threadId(thread_id), m_fiberId(fiber_id), m_time(time), m_threadName(thread_name), m_loggerName(logger_name)
    {
    }

    void LogEvent::printf(const char *format, ...)
    {
        va_list ap;
        va_start(ap, format);
        vprintf(format, ap);
        va_end(ap);
    }

    void LogEvent::vprintf(const char *fmt, va_list ap)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, ap);
        if (len != -1)
        {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit MessageFormatItem(const std::string &str)
        {
        }

        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit LevelFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << LogLevel::toString(event->getLevel());
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit ElapseFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getElapse();
        }
    };

    class LoggerNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit LoggerNameFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getLoggerName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit ThreadIdFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit FiberIdFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getFiberId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit ThreadNameFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getThreadName();
        }
    };

    class DataTimeFormatItem : public LogFormatter::FormatItem
    {
    private:
        std::string m_format;

    public:
        explicit DataTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream &os, LogEvent::ptr event) override
        {
            struct tm tm{};
            time_t time = event->getTime();
            // 可重入的，即线程安全的 _r
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    };

    class FileNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit FileNameFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit LineFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit NewLineFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str)
                : m_string(str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << m_string;
        }

    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit TabFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << "\t";
        }
    };

    class PercentSignFormatItem : public LogFormatter::FormatItem
    {
    public:
        explicit PercentSignFormatItem(const std::string &str) {}
        void format(std::ostream &os, LogEvent::ptr event) override
        {
            os << "%";
        }
    };

    LogFormatter::LogFormatter(std::string pattern)
            : m_pattern(std::move(pattern))
    {
        init();
    }

    void LogFormatter::init()
    {
        static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {

#define XX(str, F)                                                               \
    {                                                                            \
        #str, [](const std::string &fmt) { return FormatItem::ptr(new F(fmt)); } \
    }
                XX(m, MessageFormatItem),     // m:消息
                XX(p, LevelFormatItem),       // p:日志级别
                XX(c, LoggerNameFormatItem),  // c:日志器名称
                XX(r, ElapseFormatItem),      // r:累计毫秒数
                XX(f, FileNameFormatItem),    // f:文件名
                XX(l, LineFormatItem),        // l:行号
                XX(t, ThreadIdFormatItem),    // t:编程号
                XX(F, FiberIdFormatItem),     // F:协程号
                XX(N, ThreadNameFormatItem),  // N:线程名称
                XX(%, PercentSignFormatItem), // %:百分号
                XX(T, TabFormatItem),         // T:制表符
                XX(n, NewLineFormatItem),     // n:换行符

#undef XX
        };

        std::vector<std::pair<int, std::string>>
                patterns;

        std::string temp;

        std::string dateformat;

        bool error = false;

        bool parsing_string = true;

        u_int32_t i = 0;
        while (i < m_pattern.size())
        {
            std::string c = std::string(1, m_pattern[i]);
            if (c == "%")
            {
                if (parsing_string)
                {
                    if (!temp.empty())
                    {
                        patterns.emplace_back(0, temp);
                    }
                    temp.clear();
                    parsing_string = false;
                    i++;
                    continue;
                }
                else
                {
                    patterns.emplace_back(1, c);
                    parsing_string = true; // %转义
                    i++;
                    continue;
                }
            }
            else
            {
                if (parsing_string)
                {
                    temp += c;
                    i++;
                    continue;
                }
                else
                {
                    patterns.emplace_back(1, c);
                    parsing_string = true;
                    i++;
                    if (c != "d")
                    {
                        continue;
                    }

                    if (i < m_pattern.size() && m_pattern[i] != '{')
                    {
                        continue;
                    }
                    i++;
                    while (i < m_pattern.size() && m_pattern[i] != '}')
                    {
                        dateformat.push_back(m_pattern[i]);
                        i++;
                    }
                    if (m_pattern[i] != '}')
                    {
                        // %d后面的大括号没有闭合，直接报错
                        std::cout << "[ERROR] LogFormatter::init() "
                                  << "pattern: [" << m_pattern << "] '{' not closed" << std::endl;
                        error = true;
                        break;
                    }
                    i++;
                }
            }
        }

        if (SYLAR_UNLIKELY(error))
        {
            m_error = true;
            return;
        }

        if (!temp.empty())
        {
            patterns.push_back(std::make_pair(0, temp));
        }

        for (auto &p : patterns)
        {
            if (p.first == 0)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(p.second)));
            }
            else if (p.second == "d")
            {
                m_items.push_back(FormatItem::ptr(new DataTimeFormatItem(dateformat)));
            }
            else
            {
                auto funcItem = s_format_items.find(p.second);
                if (funcItem == s_format_items.end())
                {
                    std::cout << "[ERROR] LogFormatter::init() pattern: [" << m_pattern << "] "
                              << "unknown format item: " << p.second << std::endl;
                    error = true;
                    break;
                }
                else
                {
                    m_items.push_back(funcItem->second(p.second));
                }
            }
        }

        if (error)
        {
            m_error = true;
            return;
        }
    }

    std::string LogFormatter::format(LogEvent::ptr event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
        {
            i->format(ss, event);
        }
        return ss.str();
    }

    std::ostream &LogFormatter::format(std::ostream &os, LogEvent::ptr event)
    {
        for (auto &i : m_items)
        {
            i->format(os, event);
        }
        return os;
    }

    LogAppender::LogAppender(LogFormatter::ptr default_formatter)
            : m_default_formatter(std::move(default_formatter))
    {
    }

    void LogAppender::setFormatter(LogFormatter::ptr val)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;
    }

    LogFormatter::ptr LogAppender::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    StdoutLogAppender::StdoutLogAppender()
            : LogAppender(LogFormatter::ptr(new LogFormatter()))
    {
    }

    void StdoutLogAppender::log(LogEvent::ptr event)
    {
        if (m_formatter)
        {
            m_formatter->format(std::cout, event);
        }
        else
        {
            m_default_formatter->format(std::cout, event);
        }
    }

    FileLogAppender::FileLogAppender(const std::string &file)
            : LogAppender(LogFormatter::ptr(new LogFormatter))
    {
        m_filename = file;
        reOpen();
    }

    /**
     * 如果一个日志事件距离上次写日志超过3秒，那就重新打开一次日志文件
     */
    void FileLogAppender::log(LogEvent::ptr event)
    {
        uint64_t now = event->getTime();
        if (now >= (m_lastTime + 3))
        {
            reOpen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
        if (m_formatter)
        {
            if (m_formatter->format(m_filestream, event).fail())
            {
                std::cout << "[ERROR] FileLogAppender::log() format error" << std::endl;
            }
        }
        else
        {
            if (m_default_formatter->format(m_filestream, event).fail())
            {
                std::cout << "[ERROR] FileLogAppender::log() format error" << std::endl;
            }
        }
    }

    bool FileLogAppender::reOpen()
    {
        MutexType::Lock lock(m_mutex);
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename, std::ios::app);
        return !!m_filestream;
    }

    Logger::Logger(const std::string &name)
            : m_name(name), m_level(LogLevel::INFO), m_create_time(GetElapsed())
    {
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.push_back(appender);
    }

    void Logger::removeAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        for (auto it = m_appenders.begin(); it != m_appenders.end(); it++)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppenders()
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    /**
     * 调用Logger的所有appenders将日志写一遍，
     * Logger至少要有一个appender，否则没有输出
     */
    void Logger::log(LogEvent::ptr event)
    {
        if (event->getLevel() <= m_level)
        {
            for (auto &i : m_appenders)
            {
                i->log(event);
            }
        }
    }

    LogEventWrap::LogEventWrap(Logger::ptr logger, LogEvent::ptr event)
            : m_logger(logger), m_event(event)
    {
    }

    /**
     * @note LogEventWrap在析构时写日志
     */
    LogEventWrap::~LogEventWrap()
    {
        m_logger->log(m_event);
    }

    LoggerManager::LoggerManager()
    {
        m_default_logger.reset(new Logger("root"));
        m_default_logger->addAppender(LogAppender::ptr(new StdoutLogAppender));
        m_default_logger->addAppender(LogAppender::ptr(new FileLogAppender("root.log")));
        m_loggers[m_default_logger->getName()] = m_default_logger;
        init();
    }

    /**
     * 如果指定名称的日志器未找到，那会就新创建一个，但是新创建的Logger是不带Appender的，
     * 需要手动添加Appender
     */
    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
            return it->second;
        }

        Logger::ptr logger(new Logger(name));
        logger->addAppender(std::make_shared<StdoutLogAppender>());
        m_loggers[name] = logger;
        return logger;
    }

    /**
     * @todo 实现从配置文件加载日志配置
     */
    void LoggerManager::init()
    {
    }
} // namespace sylar

