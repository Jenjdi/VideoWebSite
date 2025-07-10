#include "Data.hpp"
#include "httplib.h"
namespace aod
{
    const std::string WWWROOT = "./www";
    const std::string VIDEOROOT = "/video/";
    const std::string IMAGEROOT = "/image/";
    TableVideo *tb_video = nullptr;
    class Server
    {
    private:
        uint16_t _port;
        httplib::Server _srv;
        static void Insert(const httplib::Request &req, httplib::Response &res)
        {
            if (req.has_file("name") == false ||
                req.has_file("info") == false ||
                req.has_file("video") == false ||
                req.has_file("image") == false)
            {
                res.status = 400;
                res.body = R"({"result":"false","reason":"upload video failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            httplib::MultipartFormData name = req.get_file_value("name");
            httplib::MultipartFormData info = req.get_file_value("info");
            httplib::MultipartFormData video = req.get_file_value("video");
            httplib::MultipartFormData image = req.get_file_value("image");
            std::string video_name = name.content;
            std::string video_info = info.content;
            std::string video_path = WWWROOT + VIDEOROOT + video_name + video.filename;
            std::string image_path = WWWROOT + IMAGEROOT + video_name + image.filename;
            if (FileUtil(video_path).SetContent(video.content) == false)
            {
                res.status = 500;
                res.body = R"({"result":"false","reason":"video insert failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            if (FileUtil(image_path).SetContent(image.content) == false)
            {
                res.status = 500;
                res.body = R"({"result":"false","reason":"image insert failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            Json::Value video_json;
            video_json["name"] = video_name;
            video_json["info"] = video_info;
            video_json["video"] = WWWROOT + VIDEOROOT + video_name + video.filename;
            video_json["image"] = WWWROOT + IMAGEROOT + video_name + image.filename;
            if (tb_video->Insert(video_json) == false)
            {
                res.status = 500;
                res.body = R"({"result":"false","reason":"insert video failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
        }
        static void Update(const httplib::Request &req, httplib::Response &res)
        {
            // 获取视频id
            int video_id = atoi(req.matches[1].str().c_str());
            Json::Value video;
            if (JsonUtil::Deserialize(&video, req.body) == false)
            {
                res.status = 400;
                res.body = R"({"result":"false","reason":"Get Video Failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            // 修改视频信息
            if (tb_video->Update(video_id, video) == false)
            {
                res.status = 500;
                res.body = R"({"result":"false","reason":"Update Video Failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
        }
        static void SelectOne(const httplib::Request &req, httplib::Response &res)
        {
            int video_id = atoi(req.matches[1].str().c_str());
            Json::Value video;
            if (tb_video->SelectOne(video_id, &video) == false)
            {
                res.status = 400;
                res.body = R"({"result":"false","reason":"Get Video Failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            JsonUtil::Serialize(video, &res.body) == false;
            res.set_header("Content-Type", "application/json");
            return;
        }
        static void SelectAll(const httplib::Request &req, httplib::Response &res)
        {
            bool select_flag = true; // 默认所有查询
            // video?search="关键字"
            std::string search_key;
            if (req.has_param("search") == true)
            {
                select_flag = false; // 模糊匹配
                search_key = req.get_param_value("search");
            }
            Json::Value videos;
            if (select_flag == false)
            {
                if (tb_video->SelectLike(search_key, &videos) == false)
                {
                    res.status = 500;
                    res.body = R"({"result":"false","reason":"Select Video Failed"})";
                    res.set_header("Content-Type", "application/json");
                    return;
                }
            }
            else
            {
                if (tb_video->SelectAll(&videos) == false)
                {
                    res.status = 500;
                    res.body = R"({"result":"false","reason":"Select All Video Failed"})";
                    res.set_header("Content-Type", "application/json");
                    return;
                }
            }
            JsonUtil::Serialize(videos, &res.body) == false;
            res.set_header("Content-Type", "application/json");
            return;
        }
        static void Delete(const httplib::Request &req, httplib::Response &res)
        {
            // 1.获取要删除的视频的ID
            int video_id = atoi(req.matches[1].str().c_str());
            // 2.删除视频文件和视频封面
            Json::Value video;
            if (tb_video->SelectOne(video_id, &video) == false)
            {
                res.status = 500;
                res.body = R"({"result":"false","reason":"Video Not Exist"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
            std::string video_path = WWWROOT + video["video"].asString();
            std::string image_path = WWWROOT + video["image"].asString();
            remove(video_path.c_str());
            remove(image_path.c_str());
            // 3.删除数据库信息
            if (tb_video->Delete(video_id) == false)
            {
                res.status = 500;
                res.body = R"({"result":"false","reason":"Delete Database Video Failed"})";
                res.set_header("Content-Type", "application/json");
                return;
            }
        }

    public:
        Server(uint16_t port)
            : _port(port) {}
        bool RunModule()
        {
            // 1.初始化，创建指定目录
            tb_video = new TableVideo();
            FileUtil(WWWROOT).CreateDirectory();
            std::string video_path = WWWROOT + VIDEOROOT;
            FileUtil(video_path).CreateDirectory();
            std::string image_path = WWWROOT + IMAGEROOT;
            FileUtil(image_path).CreateDirectory();
            // 2.搭建http服务器，开始运行
            _srv.set_mount_point("/", WWWROOT);
            _srv.Post("/video", Insert);
            _srv.Delete("/video/(\\d+)", Delete);
            _srv.Put("/video/(\\d+)", Update);
            _srv.Get("/video/(\\d+)", SelectOne);
            _srv.Get("/video", SelectAll);
            _srv.listen("0.0.0.0", _port);
            return true;
        }
    };
};