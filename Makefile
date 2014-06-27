LIBDIR=/usr/lib/mysql/plugin
all:
	gcc -Wall -fPIC -I/usr/include/mysql -I. -shared lib_mysqludf_curl.c -lcurl -o ./lib_mysqludf_curl.so 

install:
	cp lib_mysqludf_curl.so $(LIBDIR)
