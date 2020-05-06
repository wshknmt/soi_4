#include <iostream>
#include "monitor.h"
#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include <queue>

using namespace std;

#define MAX_BUFOR   5
#define MAX_SLEEP_TIME 4

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



    void insertQueue( Wiadomosc nowa );
	void popQueue( int );

public:
	Czytelnia( );
	void dodaj( Wiadomosc nowa );
	void usun( int nrListy );
};

Czytelnia::Czytelnia( )
{

    int i;
	liczba_elementow[0] = liczba_elementow[1] = 0;

	for ( i = 0; i < 2 * MAX_BUFOR; i++ ) {
		bufor[i].id_wiadomosci = -1;
	}
}

void Czytelnia::dodaj( Wiadomosc nowa ) {
    enter();

    if( liczba_elementow[ nowa.id_kolejki ] == MAX_BUFOR ) {
        wait( pelna[ nowa.id_kolejki ] );
    }
   // cout<<"przed"<<endl;
    insertQueue( nowa );
   // cout<<"po"<<endl;
    liczba_elementow[ nowa.id_kolejki ]++;
    cout << "Dodano do kolejki " << nowa.id_kolejki + 1 << ": " << nowa.id_wiadomosci << endl;


    if( liczba_elementow[ nowa.id_kolejki ] == 1 ) {
        signal(pusta[ nowa.id_kolejki ]);
    }
    leave();
    /*enter();

    if( liczba_elementow[ nowa.id_kolejki ] == MAX_BUFOR ) {
        wait( pelna[ nowa.id_kolejki ] );
    }

    if(nowa.id_kolejki == 0) bufor1.push(nowa);
    else bufor2.push(nowa);


    liczba_elementow[ nowa.id_kolejki ]++;
    cout << "Dodano do kolejki " << nowa.id_kolejki + 1 << ": " << nowa.id_wiadomosci << endl;


    if( liczba_elementow[ nowa.id_kolejki ] == 1 ) {
        signal(pusta[ nowa.id_kolejki ]);
    }
    leave();*/


}

void Czytelnia::usun( int nrListy ) {

    enter();
   // cout<<"kk"<<endl;
    if( liczba_elementow[ nrListy ] == 0 ) {
        wait( pusta[nrListy ] );
    }
    cout << "Usunieto z kolejki " << nrListy + 1 << ": " << bufor[nrListy * MAX_BUFOR].id_wiadomosci << endl;

    popQueue( nrListy );
    liczba_elementow[ nrListy ]--;

    if( liczba_elementow[ nrListy ] == MAX_BUFOR-1 ) {
        signal(pelna[ nrListy ]);
    }

    leave();

    /*enter();
    if( liczba_elementow[ nrListy ] == 0 ) {
        wait( pusta[nrListy ] );
    }
    cout << "Usunieto " << endl;
    if(nrListy == 0) bufor1.pop();
    else bufor2.pop();

    liczba_elementow[ nrListy ]--;

    if( liczba_elementow[ nrListy ] == MAX_BUFOR-1 ) {
        signal(pelna[ nrListy ]);
    }

    leave();*/
}


void Czytelnia::insertQueue( Wiadomosc nowa ) {
	int i = 0;
	while ( bufor[nowa.id_kolejki * MAX_BUFOR + i].id_wiadomosci != -1 ) {
		i++;
	}

	bufor[nowa.id_kolejki * MAX_BUFOR + i] = nowa;
}

void Czytelnia::popQueue( int nrListy )
{
	int i = 0;
	while ( bufor[nrListy * MAX_BUFOR + i + 1].id_wiadomosci != -1 ) {
		bufor[nrListy * MAX_BUFOR + i] = bufor[nrListy * MAX_BUFOR + i + 1];
		i++;
	}

	bufor[nrListy * MAX_BUFOR + i].id_wiadomosci = -1;
}

void* pisarz_producent( void *ptr )
{
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

void* pisarz_konsument( void *ptr )
{
    Czytelnia *czyt = ( Czytelnia* )ptr;
    int nrListy;
	while( 1 ) {

		for ( nrListy = 0; nrListy < 2; nrListy++ ) {
            czyt->usun(nrListy);
		}
		sleep( rand( ) % MAX_SLEEP_TIME );

	}
}

int main( void) {
    srand( time( NULL ) );
    Czytelnia *czytPtr = new Czytelnia();
    pthread_t prod_t, kons_t;


    if( pthread_create( &kons_t, NULL, pisarz_konsument, ( void * )czytPtr ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku konsumenta\n" );
	}

    if( pthread_create( &prod_t, NULL, pisarz_producent, ( void * )czytPtr ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku producenta\n" );
	}

    while( 1 ) {

    }




	return 0;
}
