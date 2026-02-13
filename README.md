# PROGETTO Esame C

## Studente 
- Nome: Marco
- Cognome: Sposato
- Matricola: 331703

## Descrizione
Il progetto implementa un gioco svliluppato tramite in linguaggio C, in cui è basato su una mappa composta da due mondi:
- Mondo Reale
- Mondo Soprasotto

Il gioco è strutturato in turni e si può scegliere di far giocare da 1 a 4 giocatori.
Ogni giocatore può muoversi tra le zone presenti, combattare contro i nemici, raccogliere e utlizzare oggetti e 
può cambiare mondo secondo le regole riportate dalla traccia.

Il gioco termina quando:
- Un giocatore sconfigge il Demotorzone
- oppure quando tutti i giocatori sono morti.

## Compilazione
Il progetto è stato testato tramite Ubuntu 24.04 LTS con Gcc.

Per compilare il programma:
'''bash
gcc main.c gamelib.c -o esame -std=c11 -Wall

Per avviare il gioco bisogna compilare:
./esame
