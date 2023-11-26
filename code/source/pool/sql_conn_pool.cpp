#include "sql_conn_pool.h"
#include <cassert>
#include <log.h>

void SqlConnDeleter(MYSQL* ptr)
{
    if (ptr != nullptr)
        SqlConnPool::Instance()->FreeConn(ptr);
}

SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool sqlConnPool;
    return &sqlConnPool;
}

std::unique_ptr<MYSQL, DeleterT> SqlConnPool::SqlConnPool::GetConn()
{
    auto sqlopt = m_connQue.Pop(0);
    if (!sqlopt)
    {
        LOG_WARN("sql connect pool busy");
        return std::unique_ptr<MYSQL, DeleterT>(nullptr, &SqlConnDeleter);
    }
    return std::unique_ptr<MYSQL, DeleterT>(sqlopt.value(), &SqlConnDeleter);
}

void SqlConnPool::FreeConn(MYSQL* conn)
{
    assert(conn != nullptr);
    m_connQue.Push(conn);
}

int SqlConnPool::GetFreeConnCount()
{
    return m_connQue.Size();
}

void SqlConnPool::Init(std::string_view host, int port, std::string_view user, std::string_view pwd, std::string_view dbName, int connSize)
{
    assert(connSize > 0);
    for (int i = 0; i < connSize; ++i)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            LOG_ERROR("mysql init error");
            assert(sql);
        }
        sql = mysql_real_connect(sql,
                                 host.data(),
                                 user.data(),
                                 pwd.data(),
                                 dbName.data(),
                                 port,
                                 nullptr,
                                 0);
        if (!sql)
        {
            LOG_ERROR("mysql connect error");
        }

        m_connQue.Push(sql);
    }
    m_maxConn = connSize;
}

void SqlConnPool::ClosePool()
{
    while (m_maxConn--)
    {
        auto sqconn = m_connQue.Pop(-1);
        assert(sqconn);
        mysql_close(sqconn.value());
    }
    mysql_library_end();
}

SqlConnPool::SqlConnPool()
{}

SqlConnPool::~SqlConnPool()
{
    ClosePool();
}
