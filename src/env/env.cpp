//
// Created by nepture on 2024/3/18.
//
#include <csignal>
#include "../../include/env.h"
#include "../../include/log.h"
#include <string.h>
#include <iomanip>

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

bool sylar::Env::init(int argc, char **argv) {
    char link[1024] = {0};
    char path[1024] = {0};
    sprintf(link, "/proc/%d/exe", getpid());
    // link to real path
    readlink(link, path, sizeof(path));
    m_exe = path;

    auto pos = m_exe.find_last_of('/');
    // work dir
    m_cwd = m_exe.substr(0, pos) + "/";

    m_program = argv[0];
    // -config /path/to/config -file xxxx -d
    const char *now_key = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) > 1) {
                if (now_key) {
                    add(now_key, "");
                }
                now_key = argv[i] + 1;
            } else {
                SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i
                                          << " val=" << argv[i];
                return false;
            }
        } else {
            if (now_key) {
                add(now_key, argv[i]);
                now_key = nullptr;
            } else {
                SYLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i
                                          << " val=" << argv[i];
                return false;
            }
        }
    }
    if (now_key) {
        add(now_key, "");
    }
    return true;
}

void sylar::Env::add(const std::string &key, const std::string &val) {
    RWMutexType::WriteLock writeLock = RWMutexType::WriteLock(m_mutex);
    m_args[key] = val;
}

bool sylar::Env::has(const std::string &key) {
    RWMutexType::ReadLock readLock(m_mutex);
    auto it = m_args.find(key);
    return it!=m_args.end();
}

void sylar::Env::del(const std::string &key) {
    RWMutexType::WriteLock writeLock(m_mutex);
    m_args.erase(key);
}

std::string sylar::Env::get(const std::string &key, const std::string &default_value) {
    RWMutexType::ReadLock readLock(m_mutex);
    auto it = m_args.find(key);
    if (it==m_args.end()){
        return default_value;
    }else{
        return it->second;
    }
}

void sylar::Env::removeHelp(const std::string &key) {
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_helps.begin();
    while (it!=m_helps.end()){
        if (it->first==key){
            m_helps.erase(it);
            break;
        }
        it++;
    }
}

void sylar::Env::addHelp(const std::string &key, const std::string &desc) {
    removeHelp(key);
    RWMutexType::WriteLock lock(m_mutex);
    m_helps.emplace_back(key, desc);
}



void sylar::Env::printHelp() {
    RWMutexType::ReadLock lock(m_mutex);
    std::cout << "Usage: " << m_program << " [options]" << std::endl;
    for (auto &i : m_helps) {
        std::cout << std::setw(5) << "-" << i.first << " : " << i.second << std::endl;
    }
}

bool sylar::Env::setEnv(const std::string &key, const std::string &val) {
    return !setenv(key.c_str(), val.c_str(), 1);
}

std::string sylar::Env::getEnv(const std::string &key, const std::string &default_value) {
    const char *v = getenv(key.c_str());
    if (v == nullptr) {
        return default_value;
    }
    return v;
}

std::string sylar::Env::getAbsolutePath(const std::string &path) const {
    if (path.empty()) {
        return "/";
    }
    if (path[0] == '/') {
        return path;
    }
    return m_cwd + path;
}

std::string sylar::Env::getConfigPath() {
    std::string k = get("c", "conf");
    return getAbsolutePath(k);
}

