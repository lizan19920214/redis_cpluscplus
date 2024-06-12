#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "win32_hiredis.h"
#include "hiredis.h"
#include <WinSock2.h>
#include <sstream>
using namespace std;

redisContext* redisConn = nullptr;
redisReply* m_Reply = nullptr;

char* ReadFromRedis(int index)
{
    if (index >= m_Reply->elements)
    {
        cout << "read index invalid!" << endl;
        return nullptr;
    }
    auto data = m_Reply->element[index];
    if (nullptr == data)
    {
        cout << "read index not found!" << endl;
        return nullptr;
    }
    if (data->len <= 0 || data->str == nullptr)
    {
        cout << "read data len error!" << endl;
        return nullptr;
    }
    return data->str;
}

int RedisCommand(const char* cmd)
{
    m_Reply = (redisReply*)redisCommand(redisConn, cmd);
    if (nullptr == m_Reply)
    {
        cout << "redis command error!" << endl;
        return -1;
    }
    if (m_Reply->type == REDIS_REPLY_ERROR) return -2;
    if (m_Reply->type == REDIS_REPLY_NIL)   return -3;

    cout << "redis command successful!" << endl;

    return 0;
}

void Clear()
{
    if (nullptr != m_Reply)
    {
        freeReplyObject(m_Reply);
    }
    if (nullptr != redisConn)
    {
        redisFree(redisConn);
    }
}

int Reconn()
{
    if (redisConn && redisConn ->err == 0)
    {
        return 0;
    }
    std::string ip = "127.0.0.1";
    std::string pwd = "You3@daba0";
    int port = 6379;
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    redisConn = redisConnectWithTimeout(ip.c_str(), port, tv);
    if (nullptr == redisConn)
    {
        cout << " connect error !" << endl;
        return -1;
    }
    cout << " connect suc !" << endl;
    if (redisConn->err != 0)
    {
        cout << " redis connect error:" << redisConn->err << endl;
        return -2;
    }

    m_Reply = (redisReply*)redisCommand(redisConn, "AUTH %s", pwd.c_str());
    if (m_Reply->type == REDIS_REPLY_ERROR)
    {
        printf("redis auth pass failed!error:%s\n", m_Reply->str);
        return -3;
    }
    printf("redis auth pass successfully! \n");

    return 0;
}

int main()
{
    Reconn();

    int id = 1234567;
    std::string name = "jack";
    std::stringstream redisCmd;
    redisCmd << "HMSET test_redis_id:" << id;
    redisCmd << " id " << id;
    redisCmd << " name " << name;

    int ret = RedisCommand(redisCmd.str().c_str());
    if (ret != 0)
    {
        return ret;
    }

    //设置redis字段过期时间
    std::stringstream strExpire;
    strExpire << "expire test_redis_id:" << id << " 1200";
    ret = RedisCommand(strExpire.str().c_str());
    if (ret != 0)
    {
        return ret;
    }

    std::stringstream strRead;
    strRead << "HMGET test_redis_id:" << id;
    strRead << " id ";
    strRead << " name ";
    ret = RedisCommand(strRead.str().c_str());
    if (ret != 0)
    {
        return ret;
    }
    int redisID = std::stoi(ReadFromRedis(0));
    std::string redisName = ReadFromRedis(1);
    
    Clear();
    
    system("pause");
    return 0;
}

