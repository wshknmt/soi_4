#include <iostream>
#include "monitor.h"
#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include <queue>

using namespace std;

#define MAX_BUFOR   50
#define MAX_SLEEP_TIME 2

int id_wiadomosci_licznik = 0;

struct Wiadomosc {
	int id_kolejki;
	int id_producenta;
	int id_wiadomosci;

};


class Czytelnia : Monitor {
    Condition pusta[2], pelna[2];
    Wiadomosc bufor[MAX_BUFOR];
    queue <Wiadomosc> bufor1;
    queue <Wiadomosc> bufor2;
    int liczba_elementow[2];
    int ile_czytelnikow[2];



    void insertQueue( Wiadomosc nowa  );
	void popQueue( int );

public:
	Czytelnia( );
	void dodaj( Wiadomosc nowa );
	void usun( int nrListy );
	void sprawdz( int nrListy );
};

struct Czytelnik {
    Czytelnia *czytPtr;
    int nrListy;
};


Czytelnia::Czytelnia( )
{

    int i;
	liczba_elementow[0] = liczba_elementow[1] = 0;
	ile_czytelnikow[0] = ile_czytelnikow[1] = 0;

	for ( i = 0; i < 2 * MAX_BUFOR; i++ ) {
		bufor[i].id_wiadomosci = -1;
	}
}

void Czytelnia::dodaj( Wiadomosc nowa ) {
    enter();

    if( liczba_elementow[ nowa.id_kolejki ] == MAX_BUFOR || ile_czytelnikow[nowa.id_kolejki] > 0 ) {
        wait( pelna[ nowa.id_kolejki ] );
    }
    insertQueue( nowa );
    liczba_elementow[ nowa.id_kolejki ]++;
    cout << "Dodano do kolejki " << nowa.id_kolejki + 1 << ": " << nowa.id_wiadomosci << endl;


    if( liczba_elementow[ nowa.id_kolejki ] == 1 ) {
        signal(pusta[ nowa.id_kolejki ]);
    }
    leave();


}

void Czytelnia::sprawdz( int nrListy ) {
    enter();

    if( liczba_elementow[ nrListy ] == 0 ) {
        wait( pusta[nrListy ] );
    }
    ile_czytelnikow[ nrListy ]++;
    cout << "Przeczytano: " << bufor[nrListy * MAX_BUFOR].id_wiadomosci << " / " << bufor[nrListy * MAX_BUFOR].id_producenta << endl;
    ile_czytelnikow[ nrListy ]--;
    if( ile_czytelnikow[ nrListy ] == MAX_BUFOR-1 ) {
        signal(pelna[ nrListy ]);
    }
    leave();
}

void Czytelnia::usun( int nrListy ) {

    enter();
    if( liczba_elementow[ nrListy ] == 0 || ile_czytelnikow[ nrListy ] > 0 ) {
        wait( pusta[nrListy ] );
    }

    cout << "Usunieto z kolejki " << nrListy + 1 << ": " << bufor[nrListy * MAX_BUFOR].id_wiadomosci << endl;

    popQueue( nrListy );
    liczba_elementow[ nrListy ]--;

    if( liczba_elementow[ nrListy ] == MAX_BUFOR-1 ) {
        signal(pelna[ nrListy ]);
    }

    leave();
}


void Czytelnia::insertQueue( Wiadomosc nowa ) {
	int i = 0;
	while ( bufor[nowa.id_kolejki * MAX_BUFOR + i].id_wiadomosci != -1 ) {
		i++;
	}

	bufor[nowa.id_kolejki * MAX_BUFOR + i] = nowa;
}

void Czytelnia::popQueue( int nrListy ) {
	int i = 0;
	while ( bufor[nrListy * MAX_BUFOR + i + 1].id_wiadomosci != -1 ) {
		bufor[nrListy * MAX_BUFOR + i] = bufor[nrListy * MAX_BUFOR + i + 1];
		i++;
	}

	bufor[nrListy * MAX_BUFOR + i].id_wiadomosci = -1;
}

void* pisarz_producent( void *ptr ) {
    Czytelnia *czyt = ( Czytelnia* )ptr;
    while( 1 ) {
        Wiadomosc nowa;
		nowa.id_kolejki = rand( ) % 2;
		nowa.id_producenta = getpid( );
		nowa.id_wiadomosci = id_wiadomosci_licznik++;
		czyt->dodaj(nowa);
		sleep( rand( ) % MAX_SLEEP_TIME );
    }
}

void* pisarz_konsument( void *ptr ) {
    Czytelnia *czyt = ( Czytelnia* )ptr;
    int nrListy;
	while( 1 ) {

		for ( nrListy = 0; nrListy < 2; nrListy++ ) {
            czyt->usun(nrListy);
		}
		sleep( rand( ) % MAX_SLEEP_TIME );

	}
}

void* czytelnik( Czytelnik cz ) {
     //int nrListy = 1;
    cz.nrListy--;
   // Czytelnia *czyt = ( Czytelnia* )ptr;
   Czytelnia *czyt = ( Czytelnia* )cz.czytPtr;
	while( 1 ) {
        czyt->sprawdz(cz.nrListy);
		sleep( rand( ) % MAX_SLEEP_TIME );
	}
}

int main( void) {
    srand( time( NULL ) );
    Czytelnik cz;
    cz.czytPtr = new Czytelnia();
    cz.nrListy = 1;
    Czytelnia *czytPtr = new Czytelnia();
    pthread_t prod_t, kons_t, czyt_t;


    if( pthread_create( &kons_t, NULL, pisarz_konsument, ( void * )czytPtr ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku konsumenta\n" );
	}

    if( pthread_create( &prod_t, NULL, pisarz_producent, ( void * )czytPtr ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku producenta\n" );
	}

	/*if( pthread_create( &czyt_t, NULL, czytelnik, ( void * )cz ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku producenta\n" );
	}*/

    while( 1 ) {

    }




	return 0;
}
