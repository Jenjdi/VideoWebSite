#include"Util.hpp"
#include"Data.hpp"
void testfileutil()
{
    aod::FileUtil("./www").CreateDirectory();
    aod::FileUtil("./www/index.html").SetContent("<html></html>");
    std::string body;
    aod::FileUtil("./www/index.html").GetContent(&body);
    aod::FileUtil("./www/index.html").Size();
}
void testjsonutil()
{
    Json::Value val;
    val["姓名"]="name";
    val["年龄"]=25;
    val["成绩"].append(123);
    val["成绩"].append(12.5);
    val["成绩"].append(168.5);
    aod::JsonUtil ju;
    std::string body;
    ju.Serialize(val,&body);
    std::cout<<body<<std::endl;
    Json::Value stu;
    ju.Deserialize(&stu,body);
    std::cout<<stu["姓名"].asString()<<std::endl;
    std::cout<<stu["年龄"].asInt()<<std::endl;
    size_t sz=val["成绩"].size();
    for(auto& e:val["成绩"])
    {
        std::cout<<e<<std::endl;
    }

}
void testdatabase()
{
    aod::TableVideo tv;
    Json::Value value;
    value["name"]="name1";
    value["info"]="this ";
    value["video"]="./video/abc.mp4";
    value["image"]="./image/abc.jpg";
    tv.Update(2,value);
    //tv.Insert(value);
    //tv.Delete(4);
    Json::Value val;
    tv.SelectAll(&val);
    aod::JsonUtil ju;
    std::string body;
    ju.Serialize(val,&body);
    std::cout<<body<<std::endl;
    
}
int main()
{
    //testfileutil();
    //testjsonutil();
    testdatabase();
}