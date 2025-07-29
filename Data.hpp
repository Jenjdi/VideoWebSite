#ifndef __MY_DATA__
#define __MY_DATA__
#include "Util.hpp"
#include <cstdlib>
#include <mutex>
#include <mysql/mysql.h>

namespace aod
{
#define HOST "localhost"
#define USER "root"
#define PASSWORD ""
#define DATABASE "aod_system"

    // #define SELECTALL_VIDEOS "select * from tb_video;"
    // #define SELECTONE_VIDEO "select * from tb_video where id=%d;"
    // #define INSERT_VIDEO "insert into tb_video values(null,'%s','%s','%s','%s');"
    // #define DELETE_VIDEO "delete from tb_video where id=%d;"
    // #define UPDATE_VIDEO "update tb_video set name='%s',info='%s' where id=%d;"
    // #define SELECTLIKE_VIDEO "select * from tb_video where name like '%%%s%%';"

    // static MYSQL *MysqlInit() {
    // 	MYSQL *mysql = mysql_init(NULL);
    // 	if (mysql == NULL) {
    // 		std::cout << "init mysql instance failed!\n";
    // 		return NULL;
    // 	}
    // 	if (mysql_real_connect(mysql, HOST, USER, PASSWORD, DATABASE, 0, NULL, 0) == NULL) {
    // 		std::cout << "connect mysql server failed!\n";
    // 		mysql_close(mysql);
    // 		return NULL;
    // 	}
    // 	mysql_set_character_set(mysql, "utf8");
    // 	return mysql;
    // }
    // static void MysqlDestroy(MYSQL *mysql)
    // {
    //     if (mysql != NULL)
    //     {
    //         mysql_close(mysql);
    //     }
    //     return;
    // }
    // static bool MysqlQuery(MYSQL *mysql, const std::string &sql) {
    // 	int ret = mysql_query(mysql, sql.c_str());
    // 	if (ret != 0) {
    // 		std::cout << sql << std::endl;
    // 		std::cout << mysql_error(mysql) << std::endl;
    // 		return false;
    // 	}
    // 	return true;
    // }
    static MYSQL *MysqlInit()
    {
        MYSQL *mysql = mysql_init(NULL);
        if (mysql == NULL)
        {
            std::cout << "init mysql instance failed!\n";
            return NULL;
        }
        if (mysql_real_connect(mysql, HOST, USER, PASSWORD, DATABASE, 0, NULL, 0) == NULL)
        {
            std::cout << "connect mysql server failed!\n";
            mysql_close(mysql);
            return NULL;
        }
        mysql_set_character_set(mysql, "utf8");
        return mysql;
    }
    static void MysqlDestroy(MYSQL *mysql)
    {
        if (mysql != NULL)
        {
            mysql_close(mysql);
        }
        return;
    }
    static bool MysqlQuery(MYSQL *mysql, const std::string &sql)
    {
        int ret = mysql_query(mysql, sql.c_str());
        if (ret != 0)
        {
            std::cout << sql << std::endl;
            std::cout << mysql_error(mysql) << std::endl;
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
        // TableVideo()
        // {
        //     _mysql = MysqlInit();
        //     if (_mysql == NULL)
        //     {
        //         exit(-1);
        //     }
        // }
        // ~TableVideo()
        // {
        //     MysqlDestroy(_mysql);
        // }
        TableVideo()
        {
            _mysql = MysqlInit();
            if (_mysql == NULL)
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
        //     sql.resize(4096 + video["info"].asString().size());
        //     if (video["name"].asString().size() == 0) {
        // 			return false;
        // 		}
        //     sprintf(&sql[0], INSERT_VIDEO,
        //             video["name"].asCString(),
        //             video["info"].asCString(),
        //             video["video"].asCString(),
        //             video["image"].asCString());
        //     return MysqlQuery(_mysql, sql);
        // }
        bool Insert(const Json::Value &video)
        {
            // id name info video image
            std::string sql;
            sql.resize(4096 + video["info"].asString().size()); // 防止简介过长
#define INSERT_VIDEO "insert tb_video values(null, '%s', '%s', '%s', '%s');"
            if (video["name"].asString().size() == 0)
            {
                return false;
            }
            // 要完成的细致的话需要对各个数据进行校验，因为不校验直接用就有可能出问题
            sprintf(&sql[0], INSERT_VIDEO, video["name"].asCString(),
                    video["info"].asCString(),
                    video["video"].asCString(),
                    video["image"].asCString());
            return MysqlQuery(_mysql, sql);
        }
        // bool Delete(int video_id)
        // {
        //     char sql[1024] = {0};
        //     sprintf(sql, DELETE_VIDEO, video_id);
        //     return MysqlQuery(_mysql, sql);
        // }
        bool Delete(int video_id)
        {
#define DELETE_VIDEO "delete from tb_video where id=%d;"
            char sql[1024] = {0};
            sprintf(sql, DELETE_VIDEO, video_id);
            return MysqlQuery(_mysql, sql);
        }
        // bool Update(int video_id, const Json::Value &video)
        // {
        //     std::string sql;
        //     sql.resize(4096 + video["info"].asString().size()); // 防止简介过长
        //     sprintf(&sql[0], UPDATE_VIDEO,
        //             video["name"].asString().c_str(),
        //             video["info"].asString().c_str(), video_id);
        //     return MysqlQuery(_mysql, sql);
        // }
        bool Update(int video_id, const Json::Value &video)
        {
            std::string sql;
            sql.resize(4096 + video["info"].asString().size()); // 防止简介过长
#define UPDATE_VIDEO "update tb_video set name='%s', info='%s' where id=%d;"
            sprintf(&sql[0], UPDATE_VIDEO, video["name"].asCString(),
                    video["info"].asCString(), video_id);
            return MysqlQuery(_mysql, sql);
        }
        bool SelectLike(const std::string &key, Json::Value *videos)
        {
#define SELECTLIKE_VIDEO "select * from tb_video where name like '%%%s%%';"
            char sql[1024] = {0};
            sprintf(sql, SELECTLIKE_VIDEO, key.c_str());
            _mutex.lock(); //-----lock start 保护查询与保存结果到本地的过程
            bool ret = MysqlQuery(_mysql, sql);
            if (ret == false)
            {
                _mutex.unlock();
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                std::cout << "mysql store result failed!\n";
                _mutex.unlock();
                return false;
            }
            _mutex.unlock(); //------lock end
            int num_rows = mysql_num_rows(res);
            for (int i = 0; i < num_rows; i++)
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
            return true;
        }
        bool SelectAll(Json::Value *videos)
        {
#define SELECTALL_VIDEO "select * from tb_video;"
            _mutex.lock(); //-----lock start 保护查询与保存结果到本地的过程
            bool ret = MysqlQuery(_mysql, SELECTALL_VIDEO);
            if (ret == false)
            {
                _mutex.unlock();
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                std::cout << "mysql store result failed!\n";
                _mutex.unlock();
                return false;
            }
            _mutex.unlock(); //------lock end
            int num_rows = mysql_num_rows(res);
            for (int i = 0; i < num_rows; i++)
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
            return true;
        }
        bool SelectOne(int video_id, Json::Value *video)
        {
#define SELECTONE_VIDEO "select * from tb_video where id=%d;"
            char sql[1024] = {0};
            sprintf(sql, SELECTONE_VIDEO, video_id);
            _mutex.lock(); //-----lock start 保护查询与保存结果到本地的过程
            bool ret = MysqlQuery(_mysql, sql);
            if (ret == false)
            {
                _mutex.unlock();
                return false;
            }
            MYSQL_RES *res = mysql_store_result(_mysql);
            if (res == NULL)
            {
                std::cout << "mysql store result failed!\n";
                _mutex.unlock();
                return false;
            }
            _mutex.unlock(); //------lock end
            int num_rows = mysql_num_rows(res);
            if (num_rows != 1)
            {
                std::cout << "have no data!\n";
                mysql_free_result(res);
                return false;
            }
            MYSQL_ROW row = mysql_fetch_row(res);
            (*video)["id"] = video_id;
            (*video)["name"] = row[1];
            (*video)["info"] = row[2];
            (*video)["video"] = row[3];
            (*video)["image"] = row[4];
            mysql_free_result(res);
            return true;
        }
    };
}

#endif