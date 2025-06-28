
#ifndef __MY_DATA__
#define __MY_DATA__
#include "Util.hpp"
#include <mysql/mysql.h>
#include <mutex>
namespace aod
{
#define HOST "localhost"
#define USER "root"
#define PASSWORD ""
#define DATABASE "aod_system"

#define SELECTALL_VIDEOS "select * from tb_video"
#define SELECTONE_VIDEO "select * from tb_video where id=%d"
#define INSERT_VIDEO "insert into tb_video values(null,'%s','%s','%s','%s')"
#define DELETE_VIDEO "delete from tb_video where id=%d"
#define UPDATE_VIDEO "update tb_video set name='%s',info='%s' where id=%d"
#define SELECTLIKE_VIDEO "select * from tb_video where name like '%%%s%%'"

    static MYSQL *MysqlInit()
    {
        MYSQL *mysql = mysql_init(nullptr);
        if (mysql == nullptr)
        {
            std::cout << "mysql init failed" << std::endl;
            return nullptr;
        }
        if (mysql_real_connect(mysql, HOST, USER, PASSWORD, DATABASE, 0, nullptr, 0) == nullptr)
        {
            std::cout << "connect failed" << std::endl;
            return nullptr;
        }
        mysql_set_character_set(mysql, "utf8");
        return mysql;
    }
    static void MysqlDestroy(MYSQL *mysql)
    {
        if (mysql != nullptr)
        {
            mysql_close(mysql);
        }
    }
    static bool MysqlQuery(MYSQL *mysql, const std::string &sql)
    {
        int ret = mysql_query(mysql, sql.c_str());
        if (ret != 0)
        {
            std::cout << "query failed" << std::endl;
            return false;
        }
        return true;
    }

    class TableVideo
    {
    private:
        MYSQL *_mysql;
        std::mutex _mutex;

    public:
        TableVideo()
        {
            _mysql = MysqlInit();
            if (_mysql == nullptr)
            {
                exit(-1);
            }
        }
        ~TableVideo()
        {
            MysqlDestroy(_mysql);
        }
        // bool Insert(const Json::Value &video)
        // {
        //     std::string sql;
        //     sql.resize(4096 + video["info"].asString().size()); // 防止简介过长
        //     sprintf(&sql[0], INSERT_VIDEO,
        //             video["name"].asString().c_str(),
        //             video["info"].asString().c_str(),
        //             video["video"].asString().c_str(),
        //             video["image"].asString().c_str());
        //     return MysqlQuery(_mysql, sql);
        // }
        bool Insert(const Json::Value &video)
        {
            std::string sql;
            sql.resize(4096 + video["info"].asString().size());
            sprintf(&sql[0], INSERT_VIDEO,
                    video["name"].asString().c_str(),
                    video["info"].asString().c_str(),
                    video["video"].asString().c_str(),
                    video["image"].asString().c_str());
            return MysqlQuery(_mysql, sql);
        }

        bool Delete(int video_id)
        {
            char sql[1024] = {0};
            sprintf(sql, DELETE_VIDEO, video_id);
            return MysqlQuery(_mysql, sql);
        }
        bool Update(int video_id, const Json::Value &video)
        {
            std::string sql;
            sql.resize(4096 + video["info"].asString().size()); // 防止简介过长
            sprintf(&sql[0], UPDATE_VIDEO, video["name"].asString().c_str(),
                    video["info"].asString().c_str(), video_id);
            return MysqlQuery(_mysql, sql);
        }
        bool SelectLike(const std::string &key, Json::Value *videos)
        {
            _mutex.lock();
            char sql[1024] = {0};
            sprintf(sql, SELECTLIKE_VIDEO, key.c_str());
            if (!MysqlQuery(_mysql, sql))
            {
                std::cout << "select like failed" << std::endl;
                _mutex.unlock();
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            if (res == nullptr)
            {
                std::cout << "get res failed" << std::endl;
                _mutex.unlock();
                return false;
            }
            int row_num = mysql_num_rows(res);

            for (int i = 0; i < row_num; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                Json::Value video;
                video["id"] = atoi(row[0]);
                video["name"] = row[1];
                video["info"] = row[2];
                video["video"] = row[3];
                video["image"] = row[4];
                videos->append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }
        bool SelectAll(Json::Value *videos)
        {
            _mutex.lock(); // 因为需要将结果保存到本地，因此需要加锁保护
            
            if (!MysqlQuery(_mysql, SELECTALL_VIDEOS))
            {
                std::cout << "select all failed" << std::endl;
                _mutex.unlock();
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            if (res == nullptr)
            {
                std::cout << "get res failed" << std::endl;
                _mutex.unlock();
                return false;
            }
            videos->clear();
            //*videos = Json::Value(Json::arrayValue);
            int row_num = mysql_num_rows(res);
            for (int i = 0; i < row_num; i++)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                Json::Value video;
                video["id"] = atoi(row[0]);
                video["name"] = row[1];
                video["info"] = row[2];
                video["video"] = row[3];
                video["image"] = row[4];
                videos->append(video);
            }
            mysql_free_result(res);
            _mutex.unlock();
            return true;
        }
        bool SelectOne(int video_id, Json::Value *video)
        {
            _mutex.lock();
            char sql[1024] = {0};
            sprintf(sql, SELECTONE_VIDEO, video_id);
            if (!MysqlQuery(_mysql, sql))
            {
                std::cout << "select one failed" << std::endl;
                _mutex.unlock();
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            if (res == nullptr)
            {
                std::cout << "get res failed" << std::endl;
                _mutex.unlock();
                return false;
            }
            int row_num = mysql_num_rows(res);
            if (row_num != 1)
            {
                std::cout << "get row failed" << std::endl;
                mysql_free_result(res);
                _mutex.unlock();
                return false;
            }
            MYSQL_ROW row = mysql_fetch_row(res);
            (*video)["id"] = atoi(row[0]);
            (*video)["name"] = row[1];
            (*video)["info"] = row[2];
            (*video)["image"] = row[3];
            _mutex.unlock();
            return true;
        }
        
    };
}

#endif