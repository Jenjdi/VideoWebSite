#include "Data.hpp"
#include "httplib.h"
namespace aod
{
#define WWWROOT "./www"
#define VIDEOROOT "/video/"
#define IMAGEROOT "/image/"
	TableVideo *tb_video = NULL;
    class Server
    {
    private:
        int _port;
        httplib::Server _srv;
        // static void Insert(const httplib::Request &req, httplib::Response &res)
        // {
        //     if (req.has_file("name") == false ||
        //         req.has_file("info") == false ||
        //         req.has_file("video") == false ||
        //         req.has_file("image") == false)
        //     {
        //         res.status = 400;
        //         res.body = R"({"result":"false","reason":"upload video failed"})";
        //         res.set_header("Content-Type", "application/json");
        //         return;
        //     }
        //     httplib::MultipartFormData video = req.get_file_value("video");
        //     httplib::MultipartFormData image = req.get_file_value("image");
        //     httplib::MultipartFormData name = req.get_file_value("name");
        //     httplib::MultipartFormData info = req.get_file_value("info");
        //     std::string video_name = name.content;
        //     std::string video_info = info.content;
        //     std::string root=WWWROOT;
        //     std::string video_path = root + VIDEOROOT + video_name + video.filename;
        //     std::string image_path = root + IMAGEROOT + video_name + image.filename;
        //     if (FileUtil(video_path).SetContent(video.content) == false)
        //     {
        //         res.status = 500;
        //         res.body = R"({"result":"false","reason":"video insert failed"})";
        //         res.set_header("Content-Type", "application/json");
        //         return;
        //     }
        //     if (FileUtil(image_path).SetContent(image.content) == false)
        //     {
        //         res.status = 500;
        //         res.body = R"({"result":"false","reason":"image insert failed"})";
        //         res.set_header("Content-Type", "application/json");
        //         return;
        //     }
        //     Json::Value video_json;
        //     video_json["name"] = video_name;
        //     video_json["info"] = video_info;
        //     video_json["video"] = VIDEOROOT + video_name + video.filename;
        //     video_json["image"] = IMAGEROOT + video_name + image.filename;
        //     if (tb_video->Insert(video_json) == false)
        //     {
        //         res.status = 500;
        //         res.body = R"({"result":"false","reason":"insert video failed"})";
        //         res.set_header("Content-Type", "application/json");
        //         return;
        //     }
        //     return;
        // }
        static void Insert(const httplib::Request &req, httplib::Response &rsp) {
				if (req.has_file("name") == false ||
					req.has_file("info") == false ||
					req.has_file("video") == false ||
					req.has_file("image") == false) {
						
					rsp.status = 400;
					rsp.body = R"({"result":false, "reason":"上传的数据信息错误"})";
					rsp.set_header("Content-Type", "application/json");
					return ;
				}
				httplib::MultipartFormData name = req.get_file_value("name");//视频名称
				httplib::MultipartFormData info = req.get_file_value("info");//视频简介
				httplib::MultipartFormData video = req.get_file_value("video");//视频文件
				httplib::MultipartFormData image = req.get_file_value("image");//图片文件
				//MultipartFormData {name, content_type, filename, content}
				std::string video_name = name.content;
				std::string video_info = info.content;
				// ./www/image/变形金刚a.jpg
				std::string root = WWWROOT;
				std::string video_path = root + VIDEOROOT + video_name + video.filename;
				std::string image_path = root + IMAGEROOT + video_name + image.filename;

				if (FileUtil(video_path).SetContent(video.content) == false) {
					rsp.status = 500;
					rsp.body = R"({"result":false, "reason":"视频文件存储失败"})";
					rsp.set_header("Content-Type", "application/json");
					return ;
				}
				if (FileUtil(image_path).SetContent(image.content) == false) {
					rsp.status = 500;
					rsp.body = R"({"result":false, "reason":"图片文件存储失败"})";
					rsp.set_header("Content-Type", "application/json");
					return ;
				}
				Json::Value video_json;
				video_json["name"] = video_name;
				video_json["info"] = video_info;
				video_json["video"] = VIDEOROOT + video_name + video.filename;// /video/变形金刚robot.mp4
				video_json["image"] = IMAGEROOT + video_name + image.filename;// /video/变形金刚robot.mp4
				if (tb_video->Insert(video_json) == false) {
					rsp.status = 500;
					rsp.body = R"({"result":false, "reason":"数据库新增数据失败"})";
					rsp.set_header("Content-Type", "application/json");
					return ;
				}
				rsp.set_redirect("/index.html", 303);
				return ;
			}
        static void Update(const httplib::Request &req, httplib::Response &res)
        {
            // 获取视频id
            int video_id = stoi(req.matches[1]);
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
        
        static void SelectOne(const httplib::Request &req, httplib::Response &rsp) {
				//1. 获取视频的ID
				int video_id = std::stoi(req.matches[1]);
				//2. 在数据库中查询指定视频信息
				Json::Value video;
				if (tb_video->SelectOne(video_id, &video) == false) {
					rsp.status = 500;
					rsp.body = R"({"result":false, "reason":"查询数据库指定视频信息失败"})";
					rsp.set_header("Content-Type", "application/json");
					return ;
				}
				//3. 组织响应正文--json格式的字符串
				JsonUtil::Serialize(video, &rsp.body);
				rsp.set_header("Content-Type", "application/json");
				return ;
			}
			static void SelectAll(const httplib::Request &req, httplib::Response &rsp) {
				// /video   &    /video?search="关键字"
				bool select_flag = true;//默认所有查询
				std::string search_key;
				if (req.has_param("search") == true) {
					select_flag = false;//模糊匹配
					search_key = req.get_param_value("search");
				}
				Json::Value videos;
				if (select_flag == true) {
					if (tb_video->SelectAll(&videos) == false){
						rsp.status = 500;
						rsp.body = R"({"result":false, "reason":"查询数据库所有视频信息失败"})";
						rsp.set_header("Content-Type", "application/json");
						return ;
					}
				}else {
					if (tb_video->SelectLike(search_key, &videos) == false) {
						rsp.status = 500;
						rsp.body = R"({"result":false, "reason":"查询数据库匹配视频信息失败"})";
						rsp.set_header("Content-Type", "application/json");
						return ;
					}
				}
				JsonUtil::Serialize(videos, &rsp.body);
				rsp.set_header("Content-Type", "application/json");
				return ;
			}
        static void Delete(const httplib::Request &req, httplib::Response &res)
        {
            // 1.获取要删除的视频的ID
            int video_id = stoi(req.matches[1]);
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
        // Server(uint16_t port)
        //     : _port(port) {}
        // bool RunModule()
        // {
        //     // 1.初始化，创建指定目录
        //     tb_video = new TableVideo();
        //     FileUtil(WWWROOT).CreateDirectory();
        //     std::string root=WWWROOT;
        //     std::string video_path = root + VIDEOROOT;
        //     FileUtil(video_path).CreateDirectory();
        //     std::string image_path = root + IMAGEROOT;
        //     FileUtil(image_path).CreateDirectory();
        //     // 2.搭建http服务器，开始运行
        //     _srv.set_mount_point("/", WWWROOT);
        //     _srv.Post("/video", Insert);
        //     _srv.Delete("/video/(\\d+)", Delete);
        //     _srv.Put("/video/(\\d+)", Update);
        //     _srv.Get("/video/(\\d+)", SelectOne);
        //     _srv.Get("/video", SelectAll);
        //     _srv.listen("0.0.0.0", _port);
        //     return true;
        // }
        Server(int port):_port(port){}
			bool RunModule() {
				//1. 初始化操作---初始化数据管理模块，创建指定的目录
				tb_video = new TableVideo();
				FileUtil(WWWROOT).CreateDirectory();
				std::string root = WWWROOT;
				std::string video_real_path = root + VIDEOROOT;// ./www/video/
				FileUtil(video_real_path).CreateDirectory();
				std::string image_real_path = root + IMAGEROOT;// ./www/image/
				FileUtil(image_real_path).CreateDirectory();
				//2. 搭建http服务器，开始运行
				//	1. 设置静态资源根目录
				_srv.set_mount_point("/", WWWROOT);
				//	2. 添加请求-处理函数映射关系
				_srv.Post("/video", Insert);
				_srv.Delete("/video/(\\d+)", Delete);
				_srv.Put("/video/(\\d+)", Update);
				_srv.Get("/video/(\\d+)", SelectOne);
				_srv.Get("/video", SelectAll);
				//	3. 启动服务器
				_srv.listen("0.0.0.0", _port);
				return true;
			}
    };
};