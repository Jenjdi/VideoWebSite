
#ifndef __MY_UTIL__
#define __MY_UTIL__
#include<iostream>
#include<fstream>
#include<string>
#include<memory>
#include<jsoncpp/json/json.h>
#include<unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
namespace aod
{
    class FileUtil
    {
        private:
        std::string _filename;
        public:
        FileUtil(const std::string& name)
        :_filename(name){}
        bool Exist()//文件是否存在
        {
            int ret=access(_filename.c_str(),F_OK);
            if(ret!=0)
            {
                std::cout<<"file not exist"<<std::endl;
                return false;
            }
            return true;
        }
        size_t Size()//获取文件大小
        {
            if(!this->Exist())
            {
                return 0;
            }
            struct stat st;
            int ret=stat(_filename.c_str(),&st);
            if(ret!=0)
            {
                std::cout<<"get file stat failed"<<std::endl;
            }
            return st.st_size;
        }
        bool GetContent(std::string* body)//读取文件到body中
        {
            std::ifstream ifs;
            ifs.open(_filename,std::ios::binary);
            if(!ifs.is_open())
            {
                std::cout<<"file open failed"<<std::endl;
                return false;
            }
            size_t flen=this->Size();
            body->resize(flen);
            ifs.read(&(*body)[0],flen);//从输入流中读取字符写入到body中
            if(!ifs.good())
            {
                std::cout<<"read file failed"<<std::endl;
                return false;
            }
            ifs.close();
            return true;
        }
        bool SetContent(const std::string& body)//从body中读取数据
        {
            std::ofstream ofs;
            ofs.open(_filename,std::ios::binary);
            if(!ofs.is_open())
            {
                std::cout<<"file open failed"<<std::endl;
                return false;
            }
            ofs.write(body.c_str(),body.size());
            if(!ofs.good())
            {
                std::cout<<"write file failed"<<std::endl;
                return false;
            }
            ofs.close();
            return true;
        }
        bool CreateDirectory()//针对目录操作创建目录
        {
            if(this->Exist())
            {
                return true;
            }
            int ret=mkdir(_filename.c_str(),0777);
            if(ret!=0)
            {
                std::cout<<"create directory failed"<<std::endl;
                return false;
            }
            return true;
        }
    };
    class JsonUtil
    {
        public:
        static bool Serialize(const Json::Value& value,std::string* body)
        {
            Json::StreamWriterBuilder swb;
            swb["emitUTF8"] = true;
            std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
            
            std::stringstream ss;
            int ret=sw->write(value,&ss);
            if(ret!=0)
            {
                std::cout<<"Serialize failed"<<std::endl;
                return false;
            }
            *body=ss.str();
            return true;
        }
        static bool Deserialize(Json::Value* value,const std::string& body)
        {
            Json::CharReaderBuilder crb;
            std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
            std::string err;
            bool ret=cr->parse(body.c_str(),body.c_str()+body.size(),value,&err);
            if(!ret)
            {
                std::cout<<"Deserialize failed"<<std::endl;
                return false;
            }
            return true;
        }
    };
}

#endif