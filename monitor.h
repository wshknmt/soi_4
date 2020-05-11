//Nazwa pliku: monitor.h
//Autor: Tomasz Jordan Kruk ze strony: http://www.ia.pw.edu.pl/~tkruk/
//Komentarze: Paweł Martyniuk
//Opis: Deklaracja klas Semaphore, Condition i Monitor
#ifndef __monitor_h
#define __monitor_h

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

//semafor
class Semaphore {
 public:
	Semaphore( int value ) { if( sem_init( & sem, 0, value ) != 0 )	throw "sem_init: failed"; } //utworzenie semafora
	~Semaphore() { sem_destroy( & sem ); } //usuniecie semafora
	void p() { if( sem_wait( & sem ) != 0 )	throw "sem_wait: failed"; } //zmniejsza wartosc semafora o 1 a jezeli jest rowny 0 to go blokuje
	void v() { if( sem_post( & sem ) != 0 )	throw "sem_post: failed"; } //zwieksza wartosc semafora o 1
 private:
	sem_t sem;
};

//zmienna warunkowa
class Condition {
	friend class Monitor;
 public:
	Condition() : w( 0 ) { waitingCount = 0; }
	void wait() { w.p(); }
	bool signal() { if( waitingCount ) { --waitingCount; w.v(); return true; } else return false; }
 private:
	Semaphore w;
	int waitingCount; //liczba oczekujacych watkow
};

//monitor
class Monitor {
 public:
	Monitor() : s( 1 ) {}
	void enter() { s.p(); } //zajecie sekcji krytycznej
	void leave() { s.v(); } //zwolnienie sekcji krytycznej
	void wait( Condition & cond ) { ++cond.waitingCount; leave(); cond.wait(); } //zablokowanie jakiejs operacji
	bool signal( Condition & cond ) { if( cond.signal() ) { enter(); return true; } else return false; } //wznawia jedna z operacji oczekujacych
 private:
	Semaphore s;
};

#endif
