/*
Nazwa pliku: czytelnia.cpp
Autor: Pawel Martyniuk
Opis: Program pozwala na:
przechowywanie wiadomosci utworzonych przez producentow w dwoch kolejkach
usuwanie wiadomosci z tych kolejek przez producentow
oraz czytanie tych wiadomosci przez czytelnikow
*/

#include <iostream>
#include "monitor.h"
#include <thread>
#include <unistd.h>
#include <stdlib.h>
#include <queue>

using namespace std;

#define MAX_BUFOR   60
#define MAX_SLEEP_TIME 8

int id_wiadomosci_licznik = 0; //zapewnia unikatowe id wiadomosci

//struktura do przechowywania pojedynczej wiadomosci utworzonej przez producenta
struct Wiadomosc {
	int id_kolejki;
	int id_producenta;
	int id_wiadomosci;

};

class Czytelnia : Monitor {
    //Zmienne warunkowe:
    Condition pusta[2]; //informuje czy dana kolejka jest pusta
    Condition pelna[2]; //informuje czy dana kolejka jest pelna
    Condition czy_ktos_czyta[2]; //informuje czy w danej kolejce sa jacys czytelnicy
    Condition nowa_wiad[2]; //informuje czy w danej kolejce pojawila sie nowsza wiadomosc

    Wiadomosc bufor[MAX_BUFOR];////tu sa 2 kolejki FIFO do przechowywania wiadomosci
    queue <Wiadomosc> bufor1;
    queue <Wiadomosc> bufor2;
    int liczba_elementow[2]; //liczba istniejacych elementow w kazdej z kolejek
    int ile_czytelnikow[2]; //liczba aktualnie czytajacych czytelnikow
  	int ostatnioPrzeczyt[2]; //przechowuje ostatnio czytane id wiadomosci

    void insertQueue( Wiadomosc nowa  ); //wstawianie wiadomosci do kolejki
	void popQueue( int ); //usuwanie wiadomosci z kolejki
	int id_ostatniejWiad(Wiadomosc bufor[], int nrListy); //zwraca id wiadomosci znajdujacej sie na koncu danej kolejki

public:
	Czytelnia( );
	void dodaj( Wiadomosc nowa ); //realizuje operacje producenta
	void usun( int nrListy ); //realizuje operacje konsumenta
	void sprawdz( int nrListy ); //realizuje operacje czytelnika
};

//struktura ktora jest przekazywana jako argument w funkcji tworzacej nowy watek
struct Czytelnik {
    Czytelnia *czytPtr;
    int nrListy;
};


Czytelnia::Czytelnia( )
{
	liczba_elementow[0] = liczba_elementow[1] = 0;
	ile_czytelnikow[0] = ile_czytelnikow[1] = 0;
	ostatnioPrzeczyt[0] = ostatnioPrzeczyt[1] = -1;

     //wyczyszczenie list
    int i;
	for ( i = 0; i < 2 * MAX_BUFOR; i++ ) {
		bufor[i].id_wiadomosci = -1;
	}
}

void Czytelnia::dodaj( Wiadomosc nowa ) {
    enter();
    //jezeli kolejka jest pelna czeka az sie zrobi miejsce
    if( liczba_elementow[ nowa.id_kolejki ] == MAX_BUFOR ) {
        wait( pelna[ nowa.id_kolejki ] );
    }
    //gdy ktos czyta nie mozna dodawac ani usuwac
    if( ile_czytelnikow[nowa.id_kolejki] > 0 ) {
        wait( czy_ktos_czyta[ nowa.id_kolejki ] );
    }
    insertQueue( nowa );
    liczba_elementow[ nowa.id_kolejki ]++; //zwiekszenie liczby elementow w kolejce
    cout << "Dodano do kolejki " << nowa.id_kolejki + 1 << ": " << nowa.id_wiadomosci << endl;

    //jezeli dodany element jest pierwszy w tej kolejce to wznawia procesy ktore moze czekaly na pojawienie sie tu wiadomosci
    if( liczba_elementow[ nowa.id_kolejki ] == 1 ) {
        signal(pusta[ nowa.id_kolejki ]);
    }
    //informuje czytelnikow ze jest nowa wiadomosc do przeczytania
    signal(nowa_wiad[ nowa.id_kolejki ]);
    leave();


}

void Czytelnia::sprawdz( int nrListy ) {
    enter();
    //jezeli kolejka jest pusta czeka az cos zostanie dodane
    if( liczba_elementow[ nrListy ] == 0 ) {
        wait( pusta[nrListy ] );
    }
    //sprawdza czy cos nowego przybylo w kolejce
    if( ostatnioPrzeczyt[nrListy] == id_ostatniejWiad(bufor, nrListy) ) {
        wait( nowa_wiad[nrListy] );
    }
    //zwieksza liczbe czytelnikow operujacych na tej kolejce
    ile_czytelnikow[ nrListy ]++;
    cout << "Przeczytano: " << id_ostatniejWiad(bufor, nrListy) << "  z kolejki numer: " << nrListy + 1 << endl;
    //uaktualnienie ostatniej wiadomosci
    ostatnioPrzeczyt[nrListy] = id_ostatniejWiad(bufor, nrListy);
    //zmniejsza liczbe czytelnikow operujacych na tej kolejce
    ile_czytelnikow[ nrListy ]--;
    //jezeli ktos czeka az wszyscy czytelnicy przestana czytac to zostanie wznowiony
    if( ile_czytelnikow[ nrListy ] == 0 ) {
        signal(czy_ktos_czyta[ nrListy ]);
    }
    leave();
}

void Czytelnia::usun( int nrListy ) {

    enter();
    //jezeli kolejka jest pusta czeka az cos zostanie dodane
    if( liczba_elementow[ nrListy ] == 0 ) {
        wait( pusta[ nrListy ] );
    }
    //gdy ktos czyta nie mozna dodawac ani usuwac
    if( ile_czytelnikow[ nrListy ] > 0 ) {
        wait( czy_ktos_czyta[ nrListy ] );
    }

    cout << "Usunieto z kolejki " << nrListy + 1 << " : " << bufor[nrListy * MAX_BUFOR].id_wiadomosci << endl;

    popQueue( nrListy );
    liczba_elementow[ nrListy ]--; //zmniejszenie liczby elementow w kolejce
    //jezeli jakis producent czeka na wolne miejce to zostaje on wznowiony
    if( liczba_elementow[ nrListy ] == (MAX_BUFOR-1) ) {
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

int Czytelnia::id_ostatniejWiad(Wiadomosc bufor[], int nrListy) {
    int i = 0;
    while ( bufor[nrListy * MAX_BUFOR + i].id_wiadomosci != -1 ) {
		i++;
	}
	return bufor[nrListy * MAX_BUFOR + i -1 ].id_wiadomosci;
}

//watek producenta
void* pisarz_producent( void *ptr ) {
    Czytelnia *czyt = ( Czytelnia* )ptr;
    while( 1 ) {
    Wiadomosc nowa;
    nowa.id_kolejki = rand( ) % 2;
    nowa.id_producenta = getpid( );
    nowa.id_wiadomosci = id_wiadomosci_licznik++;
    if(nowa.id_wiadomosci == 0) nowa.id_kolejki = 1;
    czyt->dodaj(nowa);
    sleep( rand( ) % MAX_SLEEP_TIME );
    }
}

//watek konsumenta
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

//watek czytelnika
void* czytelnik( void *p ) {
     Czytelnik *cz = (Czytelnik*)p;
     cz->nrListy--;
     Czytelnia *czyt = ( Czytelnia* )cz->czytPtr;
	while( 1 ) {
        czyt->sprawdz(cz->nrListy);
	    sleep( rand( ) % ( MAX_SLEEP_TIME ) );
	}
}

int main( void) {
    srand( time( NULL ) );
    Czytelnik cz;
    cz.czytPtr = new Czytelnia();
    cz.nrListy = 1;
    pthread_t prod1_t,prod2_t, kons_t, czyt_t;//identyfikatory watkow

     //wywolujemy producenta
    if( pthread_create( &prod1_t, NULL, pisarz_producent, ( void * )cz.czytPtr ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku producenta\n" );
	}
	if( pthread_create( &prod2_t, NULL, pisarz_producent, ( void * )cz.czytPtr ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku producenta\n" );
	}

	//wywolujemy konsumenta
    if( pthread_create( &kons_t, NULL, pisarz_konsument, ( void * )cz.czytPtr ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku konsumenta\n" );
	}

	//wywolujemy czytelnika
	if( pthread_create( &czyt_t, NULL, czytelnik, ( void * )&cz ) != 0 ) {
		printf( "Nie udalo sie stworzyc watku czytenika\n" );
	}

     while( 1 ) {

     }

	return 0;
}
