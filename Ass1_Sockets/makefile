default:
	mkdir build
	$(MAKE) --no-print-directory compile

compile:
	gcc server.c -lpthread -o build/server.out
	gcc client.c -lpthread -o build/client.out

clean:
	rm -r build

rebuild:
	$(MAKE) --no-print-directory clean
	$(MAKE) --no-print-directory default
