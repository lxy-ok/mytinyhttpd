#pragma once 
//用于确保头文件只被编译一次，即使它被包含多次也不会导致重复定义的问题
#include"LOG.hpp"

#include<string>
#include<vector>
#include<unordered_map>
#include<memory>
#include<algorithm>
#include<sstream>
//提供了对字符串流的支持，可以方便地进行字符串的输入输出操作
#include<cstring>
#include<cassert>
//下面都是unix中的头文件
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/sendfile.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include<pthread.h>
