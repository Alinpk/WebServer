#ifndef SQL_CONN_POOL_H
#define SQL_CONN_POOL_H

#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "thread_safe_queue.h"
#include <string_view>
#include <memory>

using DeleterT = void(*)(MYSQL*);

class SqlConnPool {
public:
    static SqlConnPool *Instance();

    std::unique_ptr<MYSQL, DeleterT> GetConn();
    void FreeConn(MYSQL* conn);
    int GetFreeConnCount();

    void Init(std::string_view host, int port, std::string_view user, std::string_view pwd, std::string_view dbName, int connSize = 10);
    void ClosePool();
private:
    SqlConnPool();
    ~SqlConnPool();

    int m_maxConn;
    int m_useCount;
    int m_freeCount;

    ThreadSafeQueue<MYSQL*> m_connQue;
    std::mutex m_mtx;
};

#endif