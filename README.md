# Private chat
Progetto per il corso tenuto dal professor Grisetti di Sistemi Operativi dell' Università La Sapienza di Roma.
# Team 
Giordano Cicchetti (GC) & Federico Carmignani (FC)
# What
Il progetto “Private Chat” prevede la realizzazione di un programma in linguaggio C con lo scopo di poter inviare messaggi di testo fra utenti in chat private.
Per poter utilizzare il servizio è necessario registrarsi seguendo le direttive stampate (in lingua inglese) da terminale e inserendo solo un nickname (username) e la password: dati che verranno memorizzati in un file binario utilizzato dal Server come database denominato “registered_users”.
Il progetto è composto da due moduli principali:
•	Il Server riceve e memorizza ogni messaggio in una sorta di database temporaneo, esso consiste in una lista di messaggi inserita all’interno di una “struct” chat riferita alla conversazione fra due utenti della piattaforma. Esso mantiene quindi una lista di chat attualmente attive oltre ad una lista di utenti online per gestire in modo più efficiente l’inoltro dei messaggi. 
•	Il Client ha la possibilità di effettuare il login o registrarsi alla piattaforma, scegliere quindi un utente fra quelli registrati con cui parlare ed entrare così nella chat fra essi. Una volta entrato nella chat i messaggi inviati verranno sia memorizzati dal server che inviati eventualmente al destinatario se quest’ultimo è online in quel momento. Se un utente entra in una chat già esistente tutti i messaggi salvati nel server saranno visibili e stampati a schermo prima di continuare la conversazione.

# How
Il progetto è stato realizzato in linguaggio C ed è strutturato in due cartelle (/Client e /Server) in cui sono presenti nel complesso i seguenti files:

Files | Descrizione
----------------------- | -------------------------------------------------------------------------------------
common.h                | per gestione degli errori e costanti definite come gli headers dei messaggi
structures.c/.h         | per le strutture dati e le primitive necessarie a mantenere i dati nel server
registered_users        | file binario per mantenere in modo persistente i dati relativi agli utenti nel server
gitignore               | per gestire la repository su GitHub
makefile                | per agevolare la compilazione dei file sorgenti
binaryfilesearch.c/.h   | per le primitive di ricerca nel file binario degli utenti, di lettura, scrittura e stampa a schermo
ansi-color-codes.h      | per i codici ANSI per stampare a colori su terminale
client.c                | il programma con cui gli utenti possono connettersi al server con protocollo UDP             |
server.c                | il programma da far girare lato server per permettere il forwarding dei messaggi

Sia nel client che nel server sono gestite tramite ‘signal handlers’ uscite improvvise dal programma tramite CTRL-C o CTRL-Z.
È presente un sistema Whatsapp-like basato su due spunte per indicare rispettivamente la ricezione dei messaggi da parte del server e la ricezione/lettura del client destinatario del messaggio; inoltre, nel caso in cui un’ utente entri in una chat già esistente, l’altro utente già in chat sarà avvisato del suo ingresso.
Abbiamo testato i due eseguibili client e server usando il software di debugging “Valgrind” per analizzare ‘memory leaks’ per allocazioni dinamiche da noi utilizzate.

# How-to-run
Eseguire il programma è molto semplice e per farlo girare distinguiamo due casi:

1)	In locale, basta scaricare la cartella di GitHub all’indirizzo https://github.com/Giordano-Cicchetti/Prog_SO, aprire una finestra di terminale (shell) e spostarsi all’interno della cartella scaricata relativa al client o al server. Quindi digitare il comando “make” per far compilare i file sorgenti e in seguito “./server” per eseguire il server. A questo punto basta eseguire in due terminali distinti “./client” per avere due client che possono interagire fra loro seguendo le richieste del programma stesso. Per uscire dal programma usare il comando “QUIT”, tuttavia anche CTRL-C e CTRL-Z sono gestiti adeguatamente.

2)	In remoto, basta scaricare la cartella di GitHub all’indirizzo https://github.com/Giordano-Cicchetti/Prog_SO. Quindi dove si vuole far girare il server attivare il “port forwarding” nel proprio router a cui è connesso il computer a disposizione, sulla porta 2015, verso l’IP del server, per permettere ad esso di ricevere dall’esterno ed in seguito far girare il server con “./server” dalla cartella “/server”. Per uscire dal programma usare il comando “QUIT”, tuttavia anche CTRL-C e CTRL-Z sono gestiti adeguatamente. A questo punto i client modificano il file “common.h” a riga 11 con l’indirizzo IP del computer del server e poi possono eseguire “./client” all’interno della cartella relativa seguendo la stessa procedura sopra descritta per poter avviare il programma, connettersi al server e chattare con un altro utente.
