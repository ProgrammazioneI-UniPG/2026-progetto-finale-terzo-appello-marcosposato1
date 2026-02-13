extern int mappa_chiusa;
/* Funzioni richieste dalla traccia */
void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();

/* enum */

enum Tipo_zona {bosco, scuola, laboratorio, caverna, strada, giardino, supermercato, centrale_elettrica, deposito_abbandonato, stazione_polizia};

enum Tipo_nemico{nessun_nemico, billi, democane, demotorzone};

enum Tipo_oggetto{nessun_oggetto, bicicletta, maglietta_fuocoinferno, bussola, schitarrata_metallica};


/* Giocatore */
struct Giocatore{
    char nome[50];
    int mondo; /* 1 per mondo reale, 0 per mondo sopra-sotto */
    struct Zona_mondoreale* pos_mondo_reale;
    struct Zona_soprasotto* pos_mondo_soprasotto;
    int attacco_psichico;
    int difesa_psichica;
    int fortuna;
    enum Tipo_oggetto zaino[3];
};


/* Zona denl mondo reale */
struct Zona_mondoreale{
    enum Tipo_zona tipo_zona;
    enum Tipo_nemico tipo_nemico;
    enum Tipo_oggetto tipo_oggetto;
    struct Zona_mondoreale* zona_avanti;
    struct Zona_mondoreale* zona_indietro;
    struct Zona_soprasotto* link_soprasotto;
};


/* Zona del mondo sopra-sotto*/
struct Zona_soprasotto{
    enum Tipo_zona tipo_zona;
    enum Tipo_nemico tipo_nemico;
    struct Zona_soprasotto* zona_avanti;
    struct Zona_soprasotto* zona_indietro;
    struct Zona_mondoreale* link_mondoreale;

};
