#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "gamelib.h"
#include <unistd.h>

static int demotorzone_sconfitto = 0;
static struct Giocatore *vincitore = NULL;

// VINCITORI
static char ultimi_vincitori[3][50];
static int num_vincitori = 0;

// PROTORIPI GIOCO
static void avanza(struct Giocatore *g);
static int combatti(struct Giocatore *g, enum Tipo_nemico nemico);
static void indietreggia(struct Giocatore *g);
static int cambia_mondo(struct Giocatore *g, int *ha_avanzato);
static void stampa_giocatore(struct Giocatore *g);
static void stampa_zona(struct Giocatore *g);
static void raccogli_oggetto(struct Giocatore *g);
static void utilizza_oggetto(struct Giocatore *g, int *ha_avanzato);
static void passa(int *fine_turno, struct Giocatore *g);

static void salva_vincitore(struct Giocatore *g);

// STATO GIOCO
int mappa_chiusa = 1;

static struct Giocatore* giocatori[4] = {NULL, NULL, NULL, NULL};
static struct Zona_mondoreale* prima_zona_mondoreale = NULL;
static struct Zona_soprasotto* prima_zona_soprasotto = NULL;

// FUNZIONE DI STAMPA
static void stampa_tipo_zona(enum Tipo_zona tipo);
static void stampa_tipo_nemico(enum Tipo_nemico tipo);
static void stampa_tipo_oggetto(enum Tipo_oggetto tipo);

// MAPPA
static void genera_mappa();
static void inserisci_zona();

static void inserisci_nella_lista_mondoreale(struct Zona_mondoreale* nuova, int posizione);
static void inserisci_nella_lista_soprasotto(struct Zona_soprasotto* nuova, int posizione);

static void cancella_zona();
static void cancella_dalla_lista_mondoreale(int posizione);
static void cancella_dalla_lista_soprasotto(int posizione);

static void stampa_mappa();
static void stampa_zone();

static void chiudi_mappa();





void imposta_gioco() {

    int num_giocatori;      // Numero di giocatori scelto dall'utente
    int scelta;             // Scelta del menu
    int undici_usato = 0;      // Impedisce di avere più di un UndiciVirgolaCinque

    /* Libera giocatori precedenti */
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL)
        {
            free(giocatori[i]);     // Libera memoria del giocatore
            giocatori[i] = NULL;
        }
    }

    /* Imposta numero di giocatori */
    do {

        printf("Inserisci il numero di giocatori (1-4): ");

        if(scanf("%d", &num_giocatori) != 1) {
            printf("\nInput non valido. Devi inserire un numero.\n");
            while(getchar() != '\n');
            continue;
        }

        if(num_giocatori < 1 || num_giocatori > 4) {
            printf("\nIl numero di giocatori deve essere compreso tra 1 e 4.\n");
            while(getchar() != '\n');
            continue;
        }

        while(getchar() != '\n');
        break;

    }while(1);

    printf("\n");

    /* Crea giocatori */
    for (int i = 0; i < num_giocatori; i++)
    {
        // Allocazione memoria per il giocatore
        giocatori[i] = (struct Giocatore *)malloc(sizeof(struct Giocatore));
        if (giocatori[i] == NULL) {
            printf("Errore di allocazione memoria\n");
            exit(1);
        }


        // Inserimento nome (non solo numeri)
        int solo_numeri;

        do {
            solo_numeri = 1;

            printf("Inserisci il nome del giocatore %d: ", i + 1);
            scanf("%49s", giocatori[i]->nome);
            while(getchar() != '\n');

            for (int k = 0; giocatori[i]->nome[k] != '\0'; k++) {
                if (giocatori[i]->nome[k] < '0' || giocatori[i]->nome[k] > '9') {
                    solo_numeri = 0;
                    break;
                }
            }

            if (solo_numeri) {
                printf("\nErrore: il nome non può essere composto solo da numeri. Riprova.\n");
            }
        } while(solo_numeri);

        /* Lancio dado (1-20) */
        giocatori[i]->attacco_psichico = (rand() % 20) + 1;
        giocatori[i]->difesa_psichica = (rand() % 20) + 1;
        giocatori[i]->fortuna = (rand() % 20) + 1;

        printf("\nStatistiche iniziali:\n\n");
        printf("Attacco: %d | Difesa: %d | Fortuna: %d\n",
               giocatori[i]->attacco_psichico,
               giocatori[i]->difesa_psichica,
               giocatori[i]->fortuna);

        printf("1) +3 Attacco, -3 Difesa\n");
        printf("2) +3 Difesa, -3 Attacco\n");

        if (!undici_usato)
        {
            printf("3) Diventa UndiciVirgola\n");
        }

        do {
            printf("Scelta: ");

            if (scanf("%d", &scelta) != 1) {
            printf("Errore: inserisci un numero valido.\n");
            while (getchar() != '\n');
            continue;
            }

            if (scelta < 1 || scelta > 3 || (scelta == 3 && undici_usato)) {
            printf("Scelta non valida.\n");
            while (getchar() != '\n');
            continue;
            }


            while (getchar() != '\n');
            break;
        } while (1);


        // Applicazione della scelta
        if (scelta == 1)
        {
            giocatori[i]->attacco_psichico += 3;
            giocatori[i]->difesa_psichica -= 3;
        }
        else if (scelta == 2)
        {
            giocatori[i]->difesa_psichica += 3;
            giocatori[i]->attacco_psichico -= 3;
        }
        else if (scelta == 3)
        {
            giocatori[i]->attacco_psichico += 4;
            giocatori[i]->difesa_psichica += 4;
            giocatori[i]->fortuna -= 7;
            strcpy(giocatori[i]->nome, "UndiciVirgolaCinque");
            undici_usato = 1;
        }

        /* Zaino vuoto */
        for (int j = 0; j < 3; j++)
        {
            giocatori[i]->zaino[j] = nessun_oggetto;
        }

        /* Posizione iniziale del giocatore */
        giocatori[i]->pos_mondo_reale = NULL;
        giocatori[i]->pos_mondo_soprasotto = NULL;
        giocatori[i]->mondo = 0;        // Parte nel mondo reale
    
    }

    /* Inizializzo mappa */
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;

    // Menu creazione mappa
    do {
        printf("\n--- MENU MAPPA ---\n");
        printf("1) Genera mappa casuale\n");
        printf("2) Inserisci zona\n");
        printf("3) Cancella zona\n");
        printf("4) Stampa mappa\n");
        printf("5) Stampa zone specifiche\n");
        printf("6) Chiudi mappa\n");
        
        printf("Scelta: ");

        // Controllo input numerico
        if(scanf("%d", &scelta) != 1) {
            printf("Errore: devi inserire un numero.\n");
            while(getchar() != '\n');
            continue;
        }

        if(scelta < 1 || scelta > 6) {
            printf("Errore: scelta non valida. Inserisci un numero tra 1 e 6.\n");
            while(getchar() != '\n');
            continue;
        }

        switch (scelta) {
            case 1:
                genera_mappa();
                break;
            case 2:
                inserisci_zona();
                break;
            case 3:
                cancella_zona();
                break;
            case 4:
                stampa_mappa();
                break;
            case 5:
                stampa_zone();
                break;
            case 6:
                chiudi_mappa();     // Controlla vincoli (15 zone, 1 demotorzone)
                if(mappa_chiusa) {
                    return;
                }
                break;
            default:
                printf("Scelta non valida. Riprova.\n");
        }
    }while(1);
}

static void stampa_tipo_zona(enum Tipo_zona tipo) {     // Stampa il nome della zona in base al suo tipo, riceve in input un valore dell'enum Tipo_zona

    switch(tipo) {
        case bosco:
            printf("Bosco");
            break;
        case scuola:
            printf("Scuola");
            break;
        case laboratorio:
            printf("Laboratorio");
            break;
        case caverna:
            printf("Caverna");
            break;
        case strada:
            printf("Strada");
            break;
        case giardino:
            printf("Giardino");
            break;
        case supermercato:
            printf("Supermercato");
            break;
        case centrale_elettrica:
            printf("Centrale Elettrica");
            break;
        case deposito_abbandonato:
            printf("Deposito Abbandonato");
            break;
        case stazione_polizia:
            printf("Stazione Polizia");
            break;
        // Caso di sicurezza nel caso il tipo non sia valido
        default:
            printf("Tipo zona sconosciuto");
            break;
    }
}

static void stampa_tipo_nemico(enum Tipo_nemico tipo) {     // Stampa il tipo di nemico presente in una zona
    switch(tipo) {
        case nessun_nemico:
            printf("Nessun Nemico");
            break;
        case billi:
            printf("Billi");
            break;
        case democane:
            printf("Democane");
            break;
        case demotorzone:
            printf("Demotorzone");
            break;
        default:
            printf("Tipo nemico sconosciuto");
            break;
    }
}

static void stampa_tipo_oggetto(enum Tipo_oggetto tipo) {     // Stampa il tipo di oggetto

    // Seleziona il tipo di oggetto e stampa il nome corrispondente
    switch(tipo) {
        case nessun_oggetto:
            printf("Nessun Oggetto");
            break;
        case bicicletta:
            printf("Bicicletta");
            break;
        case maglietta_fuocoinferno:
            printf("Maglietta FuocoInferno");
            break;
        case bussola:
            printf("Bussola");
            break;
        case schitarrata_metallica:
            printf("Schitarrata Metallica");
            break;
        default:
            printf("Tipo oggetto sconosciuto");
            break;
    }
}

static void genera_mappa() {        // Genera una mappa casualmente composta da: 
                                    // 15 zone del mondo reale
                                    // 15 zone del mondo soprasotto
                                    // Le due mappe solo collegate zona per zona

                                  

    // Puntatori per scorrere e creare la lista del Mondo Reale
    struct Zona_mondoreale *corrente_mondoreale = NULL;
    struct Zona_mondoreale *precedente_mondoreale = NULL;

    // Puntatori per scorrere e creare la lista del Mondo soprasotto
    struct Zona_soprasotto *corrente_soprasotto = NULL;
    struct Zona_soprasotto *precedente_soprasotto = NULL;

    // Posizione casuale (0-14) in cui verrà inserito il Demotorzone
    int indice_demotorzone = rand() % 15; // Posizione del demotorzone
    
    /* Libera zone precedenti nel mondo reale */
    while (prima_zona_mondoreale != NULL)
    {
        corrente_mondoreale = prima_zona_mondoreale;
        prima_zona_mondoreale = prima_zona_mondoreale->zona_avanti;
        free(corrente_mondoreale);
    }


    /* Libera zone precedenti nel mondo soprasotto */
    while (prima_zona_soprasotto != NULL)
    {
        corrente_soprasotto = prima_zona_soprasotto;
        prima_zona_soprasotto = prima_zona_soprasotto->zona_avanti;
        free(corrente_soprasotto);
    }

    // Reset dei puntatori iniziali
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;

    /* Creazione delle 15 zone */
    for (int i = 0; i < 15; i++) {

        /* Allocazione dinamica delle zone */
        corrente_mondoreale = malloc(sizeof(struct Zona_mondoreale));
        corrente_soprasotto = malloc(sizeof(struct Zona_soprasotto));

        if (!corrente_mondoreale || !corrente_soprasotto)
        {
            printf("Errore allocazione memoria mappa.\n");
            exit(1);
        }

        /* TIPO ZONA (uguale nei due mondi) */
        enum Tipo_zona tipo = rand() % 10;
        corrente_mondoreale->tipo_zona = tipo;
        corrente_soprasotto->tipo_zona = tipo;

        /* NEMICI */
        /* MONDO REALE */
        int nmr = rand() % 3;
        if (nmr == 0) {
            corrente_mondoreale->tipo_nemico = nessun_nemico;
        }
        else if (nmr == 1) {
            corrente_mondoreale->tipo_nemico = democane;
        }
        else {
            corrente_mondoreale->tipo_nemico = billi;
        }


        /* MONDO SOPRASOTTO */
        // Inserisci il demotorzone in una sola zona casuale
        if (i == indice_demotorzone) {
            corrente_soprasotto->tipo_nemico = demotorzone;
        }
        else {
            corrente_soprasotto->tipo_nemico = rand() % 2;
        }

        /* OGGETTI (solo mondo reale) */
        if(rand() % 2 == 0) {
            corrente_mondoreale->tipo_oggetto = nessun_oggetto;
        } else {
            corrente_mondoreale->tipo_oggetto = (rand() % 4) + 1; // Da 1 a 4
        }

        /* Puntatori */
        // Collegamenti mondo reale
        corrente_mondoreale->zona_avanti = NULL;
        corrente_mondoreale->zona_indietro = precedente_mondoreale;
        corrente_mondoreale->link_soprasotto = corrente_soprasotto;

        // Collegamenti mondo soprasotto
        corrente_soprasotto->zona_avanti = NULL;
        corrente_soprasotto->zona_indietro = precedente_soprasotto;
        corrente_soprasotto->link_mondoreale = corrente_mondoreale;

        // Aggiorna il puntatore avanti della zona precedente
        if(precedente_mondoreale != NULL) {
            precedente_mondoreale->zona_avanti = corrente_mondoreale;
        } else {
            prima_zona_mondoreale = corrente_mondoreale;
        }

        if(precedente_soprasotto != NULL) {
            precedente_soprasotto->zona_avanti = corrente_soprasotto;
        } else {
            prima_zona_soprasotto = corrente_soprasotto;
        }

        // Aggiorna i puntatori alla zona corrente
        precedente_mondoreale = corrente_mondoreale;
        precedente_soprasotto = corrente_soprasotto;
    }


    printf("Mappa generata con successo!\n");
}

static void inserisci_zona() {      // Inserire manualmente una nuova zona in una pos specifica della mappa
                                    // La zona viene creata sia nel Mondo Reale che nel Mondo Soprasotto e collegata specularmente

    int posizione;

    // Richiesta della posizione di inserimento
    printf("Inserisci la posizione dove inserire la nuova zona: ");
    scanf("%d", &posizione);
    while(getchar() != '\n');

    // Allocazione nuove zone
    struct Zona_mondoreale* nuova_zona_mondoreale = malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nuova_zona_soprasotto = malloc(sizeof(struct Zona_soprasotto));

    if(!nuova_zona_mondoreale || !nuova_zona_soprasotto) {
        printf("Errore allocazione memoria nuova zona\n");
        return;
    }


    // Assegnazione tipo zona casuale
    nuova_zona_mondoreale->tipo_zona = rand() % 10;
    nuova_zona_soprasotto->tipo_zona = nuova_zona_mondoreale->tipo_zona;


    int scelta_nemico_mr;
    // ==== NEMICO MONDO REALE ====
    do {
        printf("Nemico Mondo Reale:\n");
        printf("0) Nessun_nemico\n1) Billi\n2) Democane\n");
        scanf("%d", &scelta_nemico_mr);
        while(getchar() != '\n');
    } while (scelta_nemico_mr < 0 || scelta_nemico_mr > 2);
    nuova_zona_mondoreale->tipo_nemico = (enum Tipo_nemico)scelta_nemico_mr;


    int scelta_oggetto;
    // ==== OGGETTO MONDO REALE ====
    do {
        printf("Oggetto Mondo Reale: \n");
        printf("0) Nessun_oggetto\n1) Bicicletta\n2) Maglietta_fuocoinferno\n");
        printf("3) Bussola\n4) Schitarrata_metallica\n");
        scanf("%d", &scelta_oggetto);
        while(getchar() != '\n');
    }while(scelta_oggetto < 0 || scelta_oggetto > 4);
    nuova_zona_mondoreale->tipo_oggetto = (enum Tipo_oggetto)scelta_oggetto;

    int scelta_nemico_ss;
    // ==== NEMICO MONDO SOPRASOTTO ====
    do {
        printf("Nemico Mondo Soprasotto:\n");
        printf("0) Nessun_nemico\n1) Democane\n2) Demotorzone\n");
        scanf("%d", &scelta_nemico_ss);
        while(getchar() != '\n');
    }while(scelta_nemico_ss < 0 || scelta_nemico_ss > 3);
    nuova_zona_soprasotto->tipo_nemico = (enum Tipo_nemico)scelta_nemico_ss;


    // Inizializzazione puntatori
    nuova_zona_mondoreale->zona_avanti = nuova_zona_mondoreale->zona_indietro = NULL;
    nuova_zona_soprasotto->zona_avanti = nuova_zona_soprasotto->zona_indietro = NULL;


    // collegamento speculare
    nuova_zona_mondoreale->link_soprasotto = nuova_zona_soprasotto;
    nuova_zona_soprasotto->link_mondoreale = nuova_zona_mondoreale;


    // ==== INSERIMENTO NELLE LISTE ====
    inserisci_nella_lista_mondoreale(nuova_zona_mondoreale, posizione);
    inserisci_nella_lista_soprasotto(nuova_zona_soprasotto, posizione);

    printf("Zona inserita con successo!\n");
}

static void inserisci_nella_lista_mondoreale(struct Zona_mondoreale* nuova, int posizione) {    // Inserisci una nuova zona nella lista del Mondo Reale in una pos specificata
                                                                                                // La lista è una lista doppiamente collegata

    // Puntatore per scorrere la lista
    struct Zona_mondoreale* corrente = prima_zona_mondoreale;
    int i = 0;

    // Lista vuota o inserimento in testa
    if(!corrente || posizione == 0) {

        // La nuova zona punta all'attuale prima zona
        nuova->zona_avanti = prima_zona_mondoreale;

        // Se esiste una prima zona, aggiorna il puntatore indietro
        if(prima_zona_mondoreale) {
            prima_zona_mondoreale->zona_indietro = nuova;
        }

        // La nuova zona diventa la prima della lista
        prima_zona_mondoreale = nuova;
        return;
    }

    // Ricerca della posizione di inserimento
    // Ci sii ferma alla zona precedente rispetto alla posizione richiesta
    while (corrente->zona_avanti && i < posizione - 1) {
        corrente = corrente->zona_avanti;
        i++;
    }

    // Inserimento della nuova zona nella lista
    nuova->zona_avanti = corrente->zona_avanti;     // collegamento in avanti
    nuova->zona_indietro = corrente;                // collegamento indietro


    // Se esiste una zona successiva, aggiorna il suo puntatore indietro
    if(corrente->zona_avanti) {
        corrente->zona_avanti->zona_indietro = nuova;
    }

    // Collega la zona corrente alla nuova zona
    corrente->zona_avanti = nuova;
}

static void inserisci_nella_lista_soprasotto(struct Zona_soprasotto* nuova, int posizione) {    // Inserisci una nuova zona nella lista del Mondo Soprasotto in una posizione specificata.
                                                                                                // Lista doppiamente collegata


    // Puntatore per scorrere la lista
    struct Zona_soprasotto* corrente = prima_zona_soprasotto;
    int i = 0;

    // Lista vuota o inserimento in testa
    if(!corrente || posizione == 0) {

        // La nuova zona punta alla prima zona attuale
        nuova->zona_avanti = prima_zona_soprasotto;

        // Se esiste già una prima zona, aggiorna il puntatore indietro
        if(prima_zona_soprasotto) {
            prima_zona_soprasotto->zona_indietro = nuova;
        }

        // La nuova zona diventa la prima della lista
        prima_zona_soprasotto = nuova;
        return;
    }


    // Ricerca della posizione di inserimento
    // Scorriamo la lista fino alla posizione precedente rispetto alla posizione desiderata
    while(corrente->zona_avanti && i < posizione - 1) {
        corrente = corrente->zona_avanti;
        i++;
    }


    // Inserimento della nuova zona nella lista
    nuova->zona_avanti = corrente->zona_avanti;
    nuova->zona_indietro = corrente;


    // Se esiste una zona successiva, aggiorna il suo puntatore indietro
    if(corrente->zona_avanti) {
        corrente->zona_avanti->zona_indietro = nuova;
    }

    // Collega la zona correente alla nuova zona
    corrente->zona_avanti = nuova;
}

static void cancella_zona() {       // Questa funzione cancella una zona della mappa sia nel Mondo Reale sia nel Mondo Soprasotto, mantendo la simmetria delle due liste
    
    int posizione;

    // Controllo esistenza mappa
    // Se una delle due mappe non esiste, non è possibile cancellare alcuna zona
    if(!prima_zona_mondoreale || !prima_zona_soprasotto) {
        printf("Mappa non presente\n");
        return;
    }

    // Lettura posizione da cancellare
    printf("Inserisci la posizione della zona da cancellare: ");
    scanf("%d", &posizione);
    while (getchar() != '\n');

    // Controllo validità della posizione
    if(posizione < 0) {
        printf("Posizione non valida\n");
        return;
    }


    // Cancellazione della zona
    // La zona viene cancellata sia dal Mondo Reale sia dal Mondo Soprasotto, nella stessa posizione
    cancella_dalla_lista_mondoreale(posizione);
    cancella_dalla_lista_soprasotto(posizione);

    printf("Zona cancellata con successo!\n");
}

static void cancella_dalla_lista_mondoreale(int posizione) {    // Cancella una zona della lista del Mondo Reale in una posizione specificata, 
                                                                // liberando la memoria e mantenendo i collegamenti tra le zone.
            
    // Puntatore alla zona corrente (si parte dalla testa)                                                            
    struct Zona_mondoreale* corrente = prima_zona_mondoreale;
    int i = 0;

    // Controllo lista vuota
    // Se la lista è vuota non c'è nulla da cancellare
    if(!corrente) {
        return;
    }

    // Cancellazione della prima zona
    // Eliminazione della testa della lista
    if(posizione == 0) {
        prima_zona_mondoreale = corrente->zona_avanti;

        // Aggiorna il puntatore indietro della nuova testa
        if(prima_zona_mondoreale) {
            prima_zona_mondoreale->zona_indietro = NULL;
        }

        // Libera la memoria della zona eliminata
        free(corrente);
        return;
    }

    // Scorrimento fino alla posizione desiderata
    while(corrente && i < posizione) {
        corrente = corrente->zona_avanti;
        i++;
    }

    // Se la posizione non esiste, termina la funzione
    if(!corrente) {
        return;
    }

    // Aggiornamento collegamenti
    // Collega la zona precedente a quella successiva
    if(corrente->zona_indietro) {
        corrente->zona_indietro->zona_avanti = corrente->zona_avanti;
    }

    // Collega la zona successiva a quella precedente
    if(corrente->zona_avanti) {
        corrente->zona_avanti->zona_indietro = corrente->zona_indietro;
    }

    // Deallocazione memoria
    // libera la memoria della zona rimossa
    free(corrente);
}

static void cancella_dalla_lista_soprasotto(int posizione) {    // Cancella una zona dalla lista del Mondo Soprasotto nella posizione indicata,
                                                                // aggiornando i collegamenti e liberando la memoria occupata.

    // Puntatore alla zona corrente (inizio della lista)
    struct Zona_soprasotto* corrente = prima_zona_soprasotto;
    int i = 0;

    // Controllo lista vuota
    // Se la lista è vuota non c'è nulla da cancellare
    if(!corrente) {
        return;
    }

    // Cancellazione della prima zona
    // Eliminazione della testa della lista
    if(posizione == 0) {
        prima_zona_soprasotto = corrente->zona_avanti;

        // Aggiorna il puntatore indietro della nuova testa
        if(prima_zona_soprasotto) {
            prima_zona_soprasotto->zona_indietro = NULL;
        }

        // Libera la memoria della zona eliminata
        free(corrente);
        return;
    }

    // Scorrimento fino alla posizione richiesta
    while(corrente && i < posizione) {
        corrente = corrente->zona_avanti;
        i++;
    }

    // Se la posizione non esiste, termina la funzione
    if(!corrente) {
        return;
    }

    // Aggiornamento collegamenti
    // Collega la zona precedente a quella successiva
    if(corrente->zona_indietro) {
        corrente->zona_indietro->zona_avanti = corrente->zona_avanti;
    }

    // Collega la zona successiva a quella precedente
    if(corrente->zona_avanti) {
        corrente->zona_avanti->zona_indietro = corrente->zona_indietro;
    }

    // Deallocazione memoria
    // Libera la memoria della zona eliminata
    free(corrente);
}

static void stampa_mappa() {    // La funzione stampa_mappa visualizza le zone presenti in entrambe le mappe (scegliendo tra mondo reale e mondo soprasotto).
    
    int scelta;     // Scelta dell'utente sul mondo da stampare
    int i = 0;      // Contatore delle zone

    // Controllo esistenza mappa
    // Se una delle due mappe non esiste, la mappa non è valida
    if (!prima_zona_mondoreale || !prima_zona_soprasotto) {
        printf("Mappa non presente\n");
        return;
    }

    // Menu di scelta
    printf("\n--- MENU STAMPA MAPPA ---\n");
    printf("1) Stampa Mondo Reale\n");
    printf("2) Stampa Mondo Soprasotto\n");
    scanf("%d", &scelta);
    while (getchar() != '\n');

    // Stampa mondo reale
    if (scelta == 1) {

        // Puntatore per scorrere la lista del mondo reale
        struct Zona_mondoreale *corrente = prima_zona_mondoreale;

        printf("\n--- MAPPA MONDO REALE ---\n");

        // Scorrimento di tutte le zone
        while (corrente)
        {

            printf("Zona %d\n", i);

            // Stampa tipo della zona
            printf("Tipo zona: ");
            stampa_tipo_zona(corrente->tipo_zona);

            printf("\n");

            // Stampa tipo di nemico presente
            printf("Tipo nemico: ");
            stampa_tipo_nemico(corrente->tipo_nemico);

            printf("\n");

            // Stampa oggetto presente nella zona
            printf("Tipo oggetto: ");
            stampa_tipo_oggetto(corrente->tipo_oggetto);

            printf("\n");
            printf("----------------------\n");

            // Passa alla zona successiva
            corrente = corrente->zona_avanti;
            i++;
        }

        // Stampa mondo soprasotto
    } else if (scelta == 2){

        // Puntatotore per scorrere la lista del mondo soprasotto
        struct Zona_soprasotto *corrente = prima_zona_soprasotto;

        printf("\n--- MAPPA MONDO SOPRASOTTO ---\n");

        // Scorrimento di tutte le zone
        while (corrente) {

            printf("Zona %d\n", i);

            // Stampa tipo della zona
            printf("Tipo zona: ");
            stampa_tipo_zona(corrente->tipo_zona);

            printf("\n");

            // Stampa tipo di nemico presente
            printf("Tipo nemico: ");
            stampa_tipo_nemico(corrente->tipo_nemico);

            printf("\n");
            printf("----------------------\n");

            // Passa alla zona successiva
            corrente = corrente->zona_avanti;
            i++;
        }
    } else {
        printf("Scelta non valida\n");
    }
}

static void stampa_zone() {     // Stampa una zona specifica in entrambe le mappe, data la sua posizione.
    
    int posizione;
    int i = 0;      // Contatore per scorrere la lista

    struct Zona_mondoreale* zona_mr = prima_zona_mondoreale;
    struct Zona_soprasotto* zona_ss = NULL;

    // Controllo esistenza mappa
    if(!prima_zona_mondoreale || !prima_zona_soprasotto) {
        printf("Mappa non presente\n");
        return;
    }

    // Input posizione zona
    printf("Inserisci la posizione della zona da stampare:");
    scanf("%d", &posizione);
    while(getchar() != '\n');

    // Controllo validità della posizione
    if(posizione < 0) {
        printf("Posizione non valida\n");
        return;
    }

    /* Scorre fino alla posizione i */
    while (zona_mr && i < posizione) {
        zona_mr = zona_mr->zona_avanti;
        i++;
    }

    if(!zona_mr) {
        printf("Zona non esistente\n");
        return;
    }

    // Recupero zona soprasotto collegata
    zona_ss = zona_mr->link_soprasotto;

    /* STAMPA MONDO REALE */
    printf("\nZona mondo reale (posizione %d) \n", posizione);

    printf(" Tipo zona: ");
    stampa_tipo_zona(zona_mr->tipo_zona);
    printf("\n");

    printf(" Tipo nemico: ");
    stampa_tipo_nemico(zona_mr->tipo_nemico);
    printf("\n");

    printf(" Tipo oggetto: ");
    stampa_tipo_oggetto(zona_mr->tipo_oggetto);
    printf("\n");

    /* STAMPA MONDO SOPRASOTTO */
    printf("\nZona mondo soprasotto \n");

    printf(" Tipo zona: ");
    stampa_tipo_zona(zona_ss->tipo_zona);
    printf("\n");   

    printf(" Tipo nemico: ");
    stampa_tipo_nemico(zona_ss->tipo_nemico);
    printf("\n");
}

static void chiudi_mappa() {    // Verifica che la mappa sia chiusa correttamente secondo le regole specificate.
                                // Controllare se ci siano almeno 15 zone
                                // che nel Mondo soprasotto ci sia esattamente un demtorzone

    int numero_zone = 0;            // Contatore totale delle zone
    int numero_demotorzone = 0;     // Contatore dei demotorzone nel soprasotto

    // Puntatori per scorrere le due mappe
    struct Zona_mondoreale* mr = prima_zona_mondoreale;
    struct Zona_soprasotto* ss = prima_zona_soprasotto;

    /* Conteggio zone Mondo Reale */
    // Scorre tutta la lista del mondo reale contando le zone
    while(mr) {
        numero_zone++;
        mr = mr->zona_avanti;
    }


    /* Conteggio zone Mondo Soprasotto */
    // Scorre la lista del mondo soprasotto e conta i demotorzone
    while(ss) {
        if(ss->tipo_nemico == demotorzone) {
            numero_demotorzone++;
        }
        ss = ss->zona_avanti;
    }

    /* Controlli */
    // Controllo numero minimo di zone
    if(numero_zone < 15) {
        printf("Errore: la mappa deve contenere almeno 15 zone.\n");
        return;
    }

    // Controllo presenza esatta di un solo demotorzone
    if(numero_demotorzone != 1) {
        printf("Errore: deve esserci esattamente un demotorzone nel mondo soprasotto.\n");
        return;
    }

    /* Mappa chiusa correttamente */
    printf("Mappa chiusa correttamente. Gioco pronto!\n");
    // Pausa di un 1 sec
    sleep(1);
    // Pulizia dello schermo
    system("clear");
}

static void ordine_giocatori(int ordine[], int n) {     // Mescola casualmente l'ordine dei giocatori
                                                        // Implememta l'algoritmo di Fisher-Yates shuffle
                                                        // Serve per garantire un ordine di un turno casuale ad ogni round di gioco.
    
        // Scorre l'array partendo dall'ultimo elemento
        for (int i = n - 1; i > 0; i--) {
        // Estrae un indice casuale tra 0 e i
        int j = rand() % (i + 1);

        // Scambio degli elementi ordine[i] e ordine[j]
        int temp = ordine[i];
        ordine[i] = ordine[j];
        ordine[j] = temp;
    }
}

void gioca() {      // Funzione che gestisce l'intera fase di gioco.
                    // Controlla che il gioco sia impostato correttamente,
                    // inizializza i giocatori e gestisce il ciclo dei turni.

    // Controllo che la mappa sia stata chiusa correttamente
    if (!mappa_chiusa) {
        printf("Devi prima completare la creazione della mappa.\n");
        return;
    }

    // Controllo presenza mappa
    if (prima_zona_mondoreale == NULL){
        printf("Mappa non presente. Devi prima crearla.\n");
        return;
    }

    // Controllo presenza giocatori
    int giocatori_vivi = 0;
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            giocatori_vivi++;
        }
    }


    if (giocatori_vivi == 0) {
        printf("Nessun giocatore disponibile.");
        return;
    }

    // Inizializza la posizione iniziale dei giocatori
    for (int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            giocatori[i]->mondo = 0;    // Mondo Reale
            giocatori[i]->pos_mondo_reale = prima_zona_mondoreale;
            giocatori[i]->pos_mondo_soprasotto = NULL;
        }
    }

    // Flag che mantiene attivo il gioco
    int gioco_attivo = 1;

    // Ciclo principale del gioco
    while(gioco_attivo) {

        int ordine[4];  // Array per l'ordine casuale dei turni
        int n = 0;      // Numeri di giocatori vivi

        // Costruzione dell'array con gli indici dei giocatori vivi
        for (int i = 0; i < 4; i++) {
            if (giocatori[i] != NULL) {
                ordine[n++] = i;
            }
        }

        // Tutti i giocatori sono morti
        if(n== 0) {
            printf("Tutti i giocatori sono stati eliminati. Il gioco termina.\n");
            return;
        }

        // Mescola casualmente l'ordine dei turni
        ordine_giocatori(ordine, n);

        // Ciclo dei turni
        for (int k = 0; k < n; k++) {
            int idx = ordine[k];        // Indice del giocatore corrente
            struct Giocatore *g = giocatori[idx];
        
            if(g == NULL) {
                continue;
            }

            int ha_avanzato = 0;    // Impedisce più avanzamenti nello stesso turno
            int fine_turno = 0;     // Controlla la fine del turno


            // Turno del singolo giocatore
            while(!fine_turno) {
                // Menu del turno
                printf("\n--- TURNO GIOCATORE %s ---\n", g->nome);
                printf("1) Avanza\n");
                printf("2) Indietreggia\n");
                printf("3) Cambia mondo\n");
                printf("4) Stampa giocatore\n");
                printf("5) Stampa zona\n");
                printf("6) Raccogli oggetto\n");
                printf("7) Usa oggetto\n");
                printf("8) Passa turno\n");
                printf("Scelta: ");

                int scelta_turno;

                // Controllo input
                if(scanf("%d", &scelta_turno) != 1) {
                    printf("Input non valido. Devi inserire un numero.\n");
                    while(getchar() != '\n');
                    continue;
                }
                while(getchar() != '\n');

                
                // Gestione delle scelte del giocatore
                switch (scelta_turno) {
                    case 1:     // Avanza
                        if(ha_avanzato) {
                            printf("Hai già avanzato in questo turno.\n");
                        } else {
                            avanza(g);
                            ha_avanzato = 1;
                        }
                        break;
                    case 2:     // Indietreggia
                        if(!ha_avanzato) {
                            indietreggia(g);
                            ha_avanzato = 1;
                        } else {
                            printf("Hai già avanzato in questo turno, non puoi indietreggiare.\n");
                        }
                        break;
                    case 3:     // Cambia mondo
                        cambia_mondo(g, &ha_avanzato);
                        break;
                    case 4:     // Stampa il giocatore
                        stampa_giocatore(g);
                        break;
                    case 5:     // Stampa la zona
                        stampa_zona(g);
                        break;
                    case 6:     // Raccoglie l'oggetto
                        raccogli_oggetto(g);
                        break;
                    case 7:     // Utilizza un oggetto
                        utilizza_oggetto(g, &ha_avanzato);
                        break;
                    case 8:     // Passa il turno
                        passa(&fine_turno, g);
                        break;
                    default:
                        printf("Scelta non valida. Riprova.\n");
                }

                // Controllo morte del giocatore
                if(g->pos_mondo_reale == NULL && g->pos_mondo_soprasotto == NULL) {
                    printf("Il giocatore %s è morto.\n", g->nome);
                    free(g);
                    giocatori[idx] = NULL;
                    fine_turno = 1;
                }
            }
        }

        // Controllo vittoria per sconfitta del demotorzone
        if(demotorzone_sconfitto) {
            printf("\n %s ha sconfitto il DEMOTORZONE!\n", vincitore->nome);
            salva_vincitore(vincitore);
            termina_gioco();
            return;
        }
    }
}

static int combatti(struct Giocatore *g, enum Tipo_nemico nemico) {     // Gestisce il combattimento tra un giocatore e un nemico
                                                                        // Utlizza attacco psichico, difesa psichica e fortuna.
                                                                        // Ritorna:
                                                                        // 0 -> giocatore sconfitto (muore)
                                                                        // 1-> nemico sconfitto e scompare dalla zona
                                                                        // 2-> nemico sconfitto ma rimane nella zona                                                                    

    // Calcolo della forza del giocatore:
    // Attacco psichico + tiro di dado (1-20)                                                                    
    int forza_g = g->attacco_psichico + (rand() % 20) + 1;
    int forza_n = 0;

    // Difesa del giocatore riduce la forza del nemico
    int difesa_g = g->difesa_psichica / 2;

    // Impostazione della forza base del nemico in base al tipo
    switch(nemico) {
        case billi:
            forza_n = 15;
            break;
        case democane:
            forza_n = 18;
            break;
        case demotorzone:
            forza_n = 25;
            break;
        default:
            printf("Errore: nemico sconosciuto.\n");
            return 1; //Nessun nemico
    }


    // Fortuna: possibilità di aumentare la forza del giocatore
    int dado = rand() % 20 + 1;
    if(dado < g->fortuna) {
        printf("La fortuna è dalla tua parte! La tua forza aumenta di 5 punti.\n");
        forza_g += 5;
    }

    // Riduzione della forza del nemico grazie alla difesa del giocatore
    forza_n -= difesa_g;
    if (forza_n < 1) forza_n = 1;   // La forza del nemico non può scendere sotto 1

    // Stampa delle forze per rendere il combattimento leggibile
    printf("Combattimento in corso...\n");
    printf("Forza giocatore: %d\n", forza_g);
    printf("Forza nemico: %d\n", forza_n);

    // Esito del combattimento
    if(forza_g >= forza_n) {
        printf("Hai sconfitto il nemico!\n");

        // 50% probabilità che il nemico sparisca
        if(rand() % 2 == 0) {
            printf("Il nemico è scomparso dalla zona.\n");
            return 1; // Vittoria + scompare
        } else {
            printf("Il nemico rimane nella zona.\n");
            return 2; // Vittoria + rimane
        }
    } else {
        printf("Sei stato sconfitto dal nemico...\n");
        return 0; // Sconfitta
    }
}

static void avanza(struct Giocatore *g) {       // Permette al giocatore di avanzare nella zona successiva della mappa
                                                // in cui si trova (Mondo Reale o Soprasotto)
                                                // Prima di avanzare:
                                                // - se è presente un nemico, viene avviato il combattimento
                                                // - se il giocatore perde, viene eliminato dal gioco
    
    if(g->mondo == 0) {
        // MONDO REALE

        // Zona corrente nel mondo reale
        struct Zona_mondoreale *z = g->pos_mondo_reale;


        // 1) COMBATTIMENTO
        if(z->tipo_nemico != nessun_nemico) {

            // Avvia il combattimento
            int vivo = combatti(g, z->tipo_nemico);

            // Se il giocatore perde il combattimento, muore
            if(!vivo) {
                printf("Il giocatore %s è morto.\n", g->nome);

                // Azzeramento posizioni per segnalare la morte
                g->pos_mondo_reale = NULL;
                g->pos_mondo_soprasotto = NULL;
                return;
            }

            // Controllo demotorzone
            if(z->tipo_nemico == demotorzone) {
                printf("\n %s ha sconfitto il DEMOTORZONE!\n", g->nome);
                demotorzone_sconfitto = 1;
                vincitore = g;
            }

            // Nemico sconfitto, rimuoverlo dalla zona
            if (rand() % 2 == 0) {
                z->tipo_nemico = nessun_nemico;
                printf("Il nemico scompare dalla zona.\n");
            } else {
                printf("Il nemico resta nella zona.\n");
            }
        }

        // 2) AVANZAMENTO (zona successiva)
        if(z->zona_avanti != NULL) {
            g->pos_mondo_reale = z->zona_avanti;
            printf("Avanzi alla zona successiva nel mondo reale.\n");
        } else {
            printf("Sei alla fine del mondo reale. Non puoi avanzare ulteriormente.\n");
        }
        
    } else {
        // MONDO SOPRASOTTO
        // Zona corrente nel mondo soprasotto
        struct Zona_soprasotto *z = g->pos_mondo_soprasotto;

        if(z->tipo_nemico != nessun_nemico) {
            
            // Avvia il combattimento
            int vivo = combatti(g, z->tipo_nemico);

            // Se il giocatore perde, viene eliminato
            if(!vivo) {
                printf("Il giocatore %s è stato eliminato dal gioco.\n", g->nome);

                g->pos_mondo_reale = NULL;
                g->pos_mondo_soprasotto = NULL;
                return;
            }

            // Controllo vittoria contro il demotorzone
            if(z->tipo_nemico == demotorzone) {
                demotorzone_sconfitto = 1;
                vincitore = g;
            }

            // 50% di prob che il nemico sparica. Nemico sconfitto, rimuoverlo dalla zona
            if (rand() % 2 == 0) {
                z->tipo_nemico = nessun_nemico;
                printf("Il nemico scompare dalla zona.\n");
            } else {
                printf("Il nemico resta nella zona.\n");
            }
        }

        // Avanzamento alla zona successiva
        if(z->zona_avanti != NULL) {
            g->pos_mondo_soprasotto = z->zona_avanti;
            printf("Avanzi alla zona successiva nel mondo soprasotto.\n");
        } else {
            printf("Sei alla fine del mondo soprasotto. Non puoi avanzare ulteriormente.\n");
        }
    }
}

static void indietreggia(struct Giocatore *g) {     // Permette al giocatore di tornare nella zona precedente della mappa in cui si trova (Mondo Reale o Soprasotto)
                                                    // Prima di indietreggiare:
                                                    // - se è presente un nemico, viene avviato il combattimento
                                                    // - se il giocatore perde, viene eliminato dal gioco


    if(g->mondo == 0) {

        // zona corrente MONDO REALE
        struct Zona_mondoreale *z = g->pos_mondo_reale;

        // COMBATTIMENTO
        if(z->tipo_nemico != nessun_nemico) {

            // Avvia il combattimento
            int vivo = combatti(g, z->tipo_nemico);

            // Se il giocatore perde il combattimento, muore
            if(!vivo) {
                printf("Il giocatore %s è stato eliminato dal gioco.\n", g->nome);


                //Azzeramento delle posizioni per segnalare la morte
                g->pos_mondo_reale = NULL;
                g->pos_mondo_soprasotto = NULL;
                return;
            }

            // NEMICO SCONFITTO, nemico può scomparire (50%)
            if (rand() % 2 == 0) {
                z->tipo_nemico = nessun_nemico;
                printf("Il nemico scompare dalla zona.\n");
            } else {
                printf("Il nemico resta nella zona.\n");
            }
        }

        // INDIETREGGIAMENTO
        if(z->zona_indietro != NULL) {
            g->pos_mondo_reale = z->zona_indietro;
            printf("Indietreggi alla zona precedente nel mondo reale.\n");
        } else {
            printf("Sei all'inizio del mondo reale. Non puoi indietreggiare ulteriormente.\n");
        }
        
    } else {

        // zona corrente MONDO SOPRASOTTO
        struct Zona_soprasotto *z = g->pos_mondo_soprasotto;

        if(z->tipo_nemico != nessun_nemico) {

            // Avvia il combattimento
            int vivo = combatti(g, z->tipo_nemico);

            // Se il giocatore perde, viene eliminato
            if(!vivo) {
                printf("Il giocatore %s è stato eliminato dal gioco.\n", g->nome);

                g->pos_mondo_reale = NULL;
                g->pos_mondo_soprasotto = NULL;
                return;
            }

            // Dopo la vittoria, il nemico può scomparire (50%)
            if (rand() % 2 == 0) {
                z->tipo_nemico = nessun_nemico;
                printf("Il nemico scompare dalla zona.\n");
            } else {
                printf("Il nemico resta nella zona.\n");
            }
        }

        // 2) Indietreggiamento alla zona precedente
        if(z->zona_indietro != NULL) {
            g->pos_mondo_soprasotto = z->zona_indietro;
            printf("Indietreggi alla zona precedente nel mondo soprasotto.\n");
        } else {
            printf("Sei all'inizio del mondo soprasotto. Non puoi indietreggiare ulteriormente.\n");
        }
    }
}    

static int cambia_mondo(struct Giocatore *g, int *ha_avanzato) {    // Permette al giocatore di cambiare tra Mondo Reale e Mondo soprasotto

    //Regole:
    // - Dal Mondo Reale al Mondo Soprasotto
    // Vale come avanza(), non si può usare se si è già avanzato
    // Non deve esserci nessun nemico in quella zona

    // - Dal Mondo soprasotto al Mondo Reale
    // Può essere usata per scappare da un nemico però è necessario superare un tiro di fortuna

    // Ritorna:
    // 1 se il cambio mondo è riuscito
    // 0 se il cambio mondo fallisce

    // Controllo di sicurezza
    if(g == NULL) return 0;

    // Mondo Reale -> Mondo Soprasotto
    if(g->mondo == 0) {

        // >ona corrente nel mondo reale
        struct Zona_mondoreale *z = g->pos_mondo_reale;

        // Non si può cambiare mondo se si è già avanzato o indietreggiato
        if(*ha_avanzato) {
            printf("Non puoi cambiare mondo dopo aver avanzato o indietreggiato in questo turno.\n");
            return 0;
        }

        // non ci deve essere nessun nemico sennò deve prima affrontarlo
        if(z->tipo_nemico != nessun_nemico) {
            printf("Devi prima affrontare il nemico presente nella zona prima di cambiare mondo.\n");
            return 0;
        }

        // Controllo esistenza del collegamento al soprasotto
        if(z->link_soprasotto == NULL) {
            printf("Errore: zona soprasotto collegata non trovata.\n");
            return 0;
        }

        // Cambio mondo: Il giocatore viene catapultato nel Soprasotto
        g->pos_mondo_soprasotto = z->link_soprasotto;
        g->pos_mondo_reale = NULL;
        g->mondo = 1;

        // Il cambio mondo conta come una avanza()
        *ha_avanzato = 1;

        printf("%s è passato al mondo soprasotto.\n", g->nome);
        return 1;

    } else {

        // SOPRASOTTO -> MONDO REALE
        // Zona corrente nel mondo soprasotto
        struct Zona_soprasotto *z = g->pos_mondo_soprasotto;

        // Controllo dell'esistenza del collegamento verso il mondo reale
        if(z->link_mondoreale == NULL) {
            printf("Non esiste una zona collegata nel mondo reale.\n");
            return 0;
        }

        // Tiro di dado per verificare la fortuna
        int dado = (rand() % 20) + 1;

        printf("Tiro di fortuna per tornare al mondo reale: %d (Fortuna = %d)\n", dado, g->fortuna);
        // Se il tiro è inferiore alla fortuna del giocatore, il cambio mondo riesce 
        if(dado < g->fortuna) {
            g->pos_mondo_reale = z->link_mondoreale;
            g->pos_mondo_soprasotto = NULL;
            g->mondo = 0;

            printf("%s riesce a tornare nel Mondo Reale!\n", g->nome);
            return 1;
        }

        // In questo caso il tiro fallisce, ma il giocatore rimane nel mondo soprasotto
        printf("%s è tornato al mondo reale.\n", g->nome);
        return 1;
    }
}

static void stampa_giocatore(struct Giocatore *g) {     // Stampa tutte le informazioni del giocatore:
                                                        // Nome, mondo corrente, statistiche e contenuto dello zaino

    // Controllo di sicurezza: verifica che il giocatore esista                                                    
    if(g == NULL) {
        printf("Giocatore non esistente.\n");
        return;
    }

    // Intestazione della stampa
    printf("\n===== STATO GIOCATORE =====\n");
    printf("Nome: %s\n", g->nome);

    // Stampa del mondo in cui si trova il giocatore
    if(g->mondo == 0) {
        printf("Mondo: Mondo Reale\n");
    } else {
        printf("Mondo: Mondo Soprasotto\n");
    }

    // Stampa delle statistiche principali del giocatore
    printf("Attacco Psichico: %d\n", g->attacco_psichico);
    printf("Difesa Psichica: %d\n", g->difesa_psichica);
    printf("Fortuna: %d\n", g->fortuna);

    printf("===========================\n");
    // Stampa del contenuto dello zaino
    printf("Zaino:\n");
    for(int i = 0; i < 3; i++) {
        printf(" Slot %d: ", i + 1);
        stampa_tipo_oggetto(g->zaino[i]);
        printf("\n");
    }

    printf("===========================\n");

}

static void stampa_zona(struct Giocatore *g) {      // Stampa le informazioni della zona in cui si trova il giocatore 
                                                    // Mostra dati diversi a seconda che il giocatore si trovi 
                                                    // nel Mondo Reale o nel Mondo Soprasotto

    // Controllo di sicurezza: verifica che il giocatore esista                                                
    if(g == NULL) {
        printf("Giocatore non valido.\n");
        return;
    }

    printf("\n===== STATO ZONA =====\n");

    // Giocatore nel Mondo Reale
    if(g->mondo == 0) {

        // Zona corrente nel mondo reale
        struct Zona_mondoreale *z = g->pos_mondo_reale;

        // Controllo di sicurezza sulla zona
        if(z == NULL) {
            printf("Zona del mondo reale non valida\n");
            return;
        }

        // Stampa le info del mondo e i tipi
        printf("Mondo: Mondo Reale\n");

        printf("Tipo zona: ");
        stampa_tipo_zona(z->tipo_zona);
        printf("\n");

        printf("Tipo nemico: ");
        stampa_tipo_nemico(z->tipo_nemico);
        printf("\n");

        printf("Tipo oggetto: ");
        stampa_tipo_oggetto(z->tipo_oggetto);
        printf("\n");
    } else {

        // Giocatore nel mondo soprasotto
        struct Zona_soprasotto *z = g->pos_mondo_soprasotto;

        // Controllo di sicurezza sulla zona
        if(z == NULL) {
            printf("Zona del mondo soprasotto non valida.\n");
            return;
        }

        // Stampa info sul mondo e i tipi
        printf("Mondo: Mondo Soprasotto\n");

        printf("Tipo zona: ");
        stampa_tipo_zona(z->tipo_zona);
        printf("\n");

        printf("Tipo nemico: ");
        stampa_tipo_nemico(z->tipo_nemico);
        printf("\n");

        printf("Oggetto presente: Nessuno (Soprasotto)\n");
    }

    printf("=========================");

}

static void raccogli_oggetto(struct Giocatore *g) {     // L'ggetto può essere raccolto solo se:
                                                        // Il giocatore è vivo
                                                        // Non ci sono nemici nella zona
                                                        // è presente un oggetto
                                                        // c'è spazio nello zaino

    // Controllo di sicurezza: verifica che il giocatore esista                                                    
    if(g == NULL) {
        printf("Giocatore non valido.\n");
        return;
    }


    // Gli oggetti esistono solo nel mondo reale
    // Se il giocatore si trova nel mondo soprasotto, l'azione fallisce
    if(g->mondo == 0) {
        printf("Nel mondo soprasotto non ci sono oggetti da raccoglierre.\n");
        return;
    }

    // Zona corrente del mondo reale
    struct Zona_mondoreale *z = g->pos_mondo_reale;

    // Controllo nemico
    // Non è possibile raccogliere oggetti se il nemico non è stato sconfitto
    if(z->tipo_nemico != nessun_nemico) {
        printf("Devi prima sconfiggere il nemico prima di raccogliere l'oggetto.\n");
        return;
    }

    // Controllo oggetto presente
    if(z->tipo_oggetto == nessun_oggetto) {
        printf("Non c'è nessun oggetto in questa zona.\n");
        return;
    }


    // Cerca spazio nello zaino
    for(int i = 0; i < 3; i++) {
        if(g->zaino[i] == nessun_oggetto) {
            // Inserisci l'oggetto nello zaino
            g->zaino[i] = z->tipo_oggetto;
            // Rimuove l'oggetto dalla zona
            z->tipo_oggetto = nessun_oggetto;

            printf("Hai raccolto l'oggetto: ");
            stampa_tipo_oggetto(g->zaino[i]);
            printf("\n");

            return;
        }
    }

    // Zaino pieno
    printf("Lo zaino è pieno. Non puoi raccogliere l'oggetto.\n");
}

static void utilizza_oggetto(struct Giocatore *g, int *ha_avanzato) {   // Permette al giocatore di utlizzare un oggetto presente nel proprio zaino
                                                                        // Ogni oggetto ha un effetto diverso sul giocatore.
                                                                        // Alcuni oggetti possono influenzare il movimento nel turno corrente

    // Controllo di sicurezza: verifica che il giocatore esista                                                                    
    if(g == NULL) {
        printf("Giocatore non valido.\n");
        return;
    }

    // Stampa il contenuto dello zaino del giocatore
    printf("\n--- ZAINO ---\n");
    for(int i = 0; i < 3; i++) {
        printf("%d) ", i + 1);
        stampa_tipo_oggetto(g->zaino[i]);
        printf("\n");
    }

    // Opzione per annullare
    printf("0) Annulla\n");
    printf("Scegli un oggetto da usare: ");

    int scelta;

    // Lettura della scelta dell'utente
    if(scanf("%d", &scelta) != 1) {
        while(getchar() != '\n');
        printf("Input non valido.\n");
        return;
    }
    while(getchar() != '\n');

    // Caso annullamento
    if(scelta == 0) {
        printf("Uso oggetto annullato.\n");
        return;
    }

    // Controllo validità scelta:
    // indice valido
    // slot dello zaino non vuoto
    if(scelta < 1 || scelta > 3 || g->zaino[scelta - 1] == nessun_oggetto) {
        printf("Oggetto non valido.\n");
        return;
    }

    // Recupera l'oggetto scelto
    enum Tipo_oggetto ogg = g->zaino[scelta - 1];

    // Messaggio di utilizzo
    printf("Usi: ");
    stampa_tipo_oggetto(ogg);
    printf("\n");


    // Applica l'effetto dell'oggetto scelto
    switch(ogg) {
        case bicicletta:
            printf("Puoi muoverti di nuovo in questo turno!\n");
            *ha_avanzato = 0;
            break;
        case maglietta_fuocoinferno:
            g->attacco_psichico += 5;
            printf("Attacco aumentato di 5!\n");
            break;
        case bussola:
            g->fortuna += 5;
            printf("Fortuna aumentata di 5!\n");
            break;
        case schitarrata_metallica:
            g->attacco_psichico += 5;
            g->difesa_psichica += 5;
            printf("Attacco e Difesa aumentati di 5!\n");
            break;
        default:
            printf("Oggetto senza effetto.\n");
            return;
    }

    // Rimuove oggetto dallo zaino
    g->zaino[scelta - 1] = nessun_oggetto; 
}

static void passa(int *fine_turno, struct Giocatore *g) {       // Termina il turno del giocatore
                                                                // La funzione viene chiamata quando il giocatore decide di cedere
                                                                // il turno al giocatore successivo

    // Messaggio che indica la fine del truno del giocatore                                                            
    printf("Turno di %s terminato.\n", g->nome);
    // Imposta il flag che forza l'uscita del ciclo del turno in gioca()
    *fine_turno = 1;
}

void termina_gioco() {      // Termina gioco libera tutta la memoria allocata dinamicamente
                            // Saluta i giocatori ancora in vita, dealloca i giocatori, 
                            // dealloca entrambe le mappe (Mondo Reale e Mondo Soprasotto)
                            // e riporta il gioco allo stato iniziale per una nuova partita

    printf("\n=== GIOCO TERMINATO ===\n");

    // Saluta e libera tutti i giocatori ancora presenti
    for(int i = 0; i < 4; i++) {
        if(giocatori[i] != NULL) {
            printf("Arrivederci %s!\n", giocatori[i]->nome);
            free(giocatori[i]);     // Dealloca il giocatore
            giocatori[i] = NULL;    // Evita dangling pointer
            // Dopo aver liberato la memoria, azzero il puntatore per evitare riferimenti a memoria deallocata
        }
    }

    // Libera mappa mondo reale
    while(prima_zona_mondoreale != NULL) {
        struct Zona_mondoreale *tmp = prima_zona_mondoreale;
        prima_zona_mondoreale = prima_zona_mondoreale->zona_avanti;
        free(tmp);
    }

    // Libera mappa soprasotto
    while(prima_zona_soprasotto != NULL) {
        struct Zona_soprasotto *tmp = prima_zona_soprasotto;
        prima_zona_soprasotto = prima_zona_soprasotto->zona_avanti;
        free(tmp);
    }

    // La mappa non è più valida: bisogna impostarla di nuovo
    mappa_chiusa = 0;

    printf("Grazie per aver giocato! \n");
}

static void salva_vincitore(struct Giocatore *g) {

    // Controllo di sicurezza: se il puntatore al giocatore è NULL esce dalla funzione
    if(g == NULL) return;

    // Salva al massimom gli ultimi 3 vincitori
    if(num_vincitori < 3) {
        int i = 0;

        // Copia carattere per carattere il nome del giocatore 
        // nell'array degli ultimi vincitori
        while(g->nome[i] != '\0') {
            ultimi_vincitori[num_vincitori][i] = g->nome[i];
            i++;
        }
        // Aggiugne il terminatore di stringa
        ultimi_vincitori[num_vincitori][i] = '\0';
        // Incrementa il numero di vincitori salvati
        num_vincitori++;
    }
}

void crediti() {

    printf("\n ===== CREDITI =====\n");
    printf("Creatore del gioco: Marco\n");

    // Se non ci sono vincitori salvati
    if(num_vincitori == 0) {
        printf("Nessun vincitore registrato.\n");
        // Se esistono vincitori, li stampa in ordine
    } else {
        printf("Ultimi vincitori: \n");

        // Stampa i nomi dei vincitori salvati
        for(int i = 0; i < num_vincitori; i++) {
            printf("%d) %s\n", i + 1, ultimi_vincitori[i]);
        }
    }

    printf("===================\n");
}




