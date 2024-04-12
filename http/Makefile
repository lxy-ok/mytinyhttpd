bin = HttpServer
src = main.cc
outPutDir = out
cp = g++
link_flags = -std=c++11 -lpthread

.PHONY:all
all:$(bin) cgi

$(bin):$(src)
	$(cp) -o $@ $^ $(link_flags) 

.PHONY:cgi
cgi:
	cd cgi;make;cd ..

.PHONY:out
out:
	mkdir $(outPutDir)
	mv $(bin) $(outPutDir)
	cp -r wwwroot $(outPutDir)
	cp ./cgi/shellCgi.sh $(outPutDir)/wwwroot
	cp ./cgi/pythonCgi.py $(outPutDir)/wwwroot
	mv ./cgi/mysqlEnrollCgi $(outPutDir)/wwwroot
	mv ./cgi/mysqlLogInCgi $(outPutDir)/wwwroot
	mv ./cgi/testCgi $(outPutDir)/wwwroot
	mv ./cgi/calculateCgi $(outPutDir)/wwwroot

#.PHONY:out
#out:
#	mkdir $(outPutDir)
#	mv $(cgi) wwwroot
#	mv $(bin) $(outPutDir)
#	cp -r wwwroot $(outPutDir)
#	cp ./cgi/shellCgi.sh $(outPutDir)/wwwroot
#	cp ./cgi/python.py $(outPutDir)/wwwroot
#	mv ./cgi/mysqlCgi $(outPutDir)/wwwroot
	
.PHONY:clean
clean:
	cd cgi; make clean; cd ..; rm -rf $(outPutDir)
