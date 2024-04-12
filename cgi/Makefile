.PHONY:all
all: mysqlEnrollCgi mysqlLogInCgi testCgi calculateCgi

mysqlEnrollCgi: mysqlEnrollCgi.cc
	g++ -o $@ $^ -std=c++11 -Llib -lmysqlclient -lpthread -ldl -static

mysqlLogInCgi: mysqlLogInCgi.cc
	g++ -o $@ $^ -std=c++11 -Llib -lmysqlclient -lpthread -ldl -static

testCgi: testCgi.cc
	g++ -o $@ $^ -std=c++11

calculateCgi: calculateCgi.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -rf mysqlEnrollCgi mysqlLogInCgi testCgi calculateCgi
