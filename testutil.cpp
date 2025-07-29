
#include"Server.hpp"

void testserver()
{
    aod::Server s(1234);
    s.RunModule();
}
int main()
{
    //testfileutil();
    //testjsonutil();
    //testdatabase();
    testserver();
}