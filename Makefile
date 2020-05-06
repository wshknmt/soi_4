CFLAGS:= -Wall -pthread

pro: czytelnia.cpp monitor.h
	g++ $(CFLAGS) czytelnia.cpp -o pro

clean:
	rm -f *.o pro
