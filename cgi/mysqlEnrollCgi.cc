#include"CgiCommon.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"./include/mysql.h"

void SendQuery(const std::string &queryString)
{
  MYSQL* mysql = mysql_init(nullptr);
  if(mysql_real_connect(mysql, "127.0.0.1", "Cgi", "123456", "mysqlCgiTest", 3306, nullptr, 0) == nullptr)
  {
    fprintf(stderr, "Failed to connect to database: Error: %s\n",
          mysql_error(mysql));

    exit(1);
  }
  else
  {
    cerr << "mysql connect success" << endl;
  }


  if(mysql_query(mysql, queryString.c_str()) == 0)
  {
    cerr << "query success" <<endl;
  }
  else 
  {
    fprintf(stderr, "Failed to send query to mysql: Error: %s\n",
          mysql_error(mysql));

    exit(2);
  }

  mysql_close(mysql);
}

int main()
{
  std::string queryString;
  GetQueryString(GetMethod(), queryString);
  // 得到的queryString ==> name=qwe&passwd=rqwemysql
  
  //std::string queryString = "name=zhangsan&passwd=123456";

  std::string name;
  std::string passwd;
  CutString(queryString, "&", name, passwd);

  std::string lableInName;
  std::string valueInName;
  CutString(name, "=", lableInName, valueInName);

  std::string lableInPasswd;
  std::string valueInPasswd;
  CutString(passwd, "=", lableInPasswd, valueInPasswd);

  // insert into user (name, password) values ('valueInName', 'valueInPasswd');
  queryString = "insert into user (name, passwd) values (password('" + valueInName + "'), password('" + valueInPasswd + "'))";
  SendQuery(queryString);
  CGILOG(INFO, queryString);

  cout << "<html><head><meta charset=\"utf-8\"></head><body><p>注册成功，请重新登录</p></body></html>" << endl;

  return 0;
}
