#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "gamelib.h"
#include <unistd.h>


int main() {

   srand(time(NULL));

    int scelta;
    int m_impost = 0;
    int menu = 0;
    int m_i;
    int rigioca_scelta;

    do{
        
        printf("\n  =========================================  ");
        printf("\n =                                         = ");
        printf("\n=============================================");
        printf("\n ||             ╭╮╭┳━━┳━╮╭╮╭╮╭╮           ||\n");
        printf(" ||             ┃╰╯┃┃━┫╭╮┫┃┃┃             ||\n");
        printf(" ||             ┃┃┃┃┃━┫┃┃┃╰╯┃             ||\n");
        printf(" ||             ╰┻┻┻━━┻╯╰┻━━╯             ||\n");
        printf("=============================================\n");

        printf(" ||1-->         IMPOSTA GIOCO          <--||\n");
        printf(" ||2-->             GIOCA              <--||\n");
        printf(" ||3-->         TERMINA GIOCO          <--||\n");
        printf(" ||4-->       VISUALIZZA CREDITI       <--||\n");
        printf("=============================================\n");
        scanf("%d", &scelta);                                               // legge la scelta del giocatore da tastiera
        while(getchar()!='\n');

        if (scelta > 4 || scelta < 1) {
            printf("la scelta che hai inserito è sbagliata\n");
            menu = 1;
        }

        switch(scelta) {

            case 1: {
                if(m_impost == 0) {
                    imposta_gioco();
                    m_impost = 1;
                }else{
                    printf("Gioco impostato... Vorresti modificarlo?\n");
                    do{
                        printf("Digita 1 per modificarlo, digita 2 per uscire\n");
                        scanf("%d", &m_i);
                        while(getchar()!='\n');
                    }while(m_i > 2 || m_i < 1);
                    if(m_i == 1){
                        imposta_gioco();
                    }else if(m_i == 2){
                        break;
                    }
                }
                menu = 1;
                break;
            }

            case 2:
            if (m_impost == 1){
                printf("\n.....Caricamento Gioco.....\n");
                sleep(1);
                system("clear");
                gioca();
                menu = 0;

                

                printf("\nPartita Terminata! Vuoi rigiocare??\n");
                printf("1- Riavvia partita!\n");
                printf("2- Termina gioco\n");
                scanf("%d", &rigioca_scelta);
                while(getchar()!='\n');

                if(rigioca_scelta == 1) {
                    menu = 1;
                } else {
                    menu = 0;
                }
            }else{
                printf("Devi impostare il gioco\n");
                menu = 1;
            }
            break;

            case 3:
                termina_gioco();
                menu = 0;
                break;

            case 4:
                crediti();
                menu = 1;
                break;
        }

    }while(menu == 1);
    return 0;
}