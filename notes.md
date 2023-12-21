- i file .h sono inclusi tra i file da passare a gcc nei makefile
- mettere o togliere i nomi dei parametri da tutte le firme dei metodi
- eseguire free_ipc al SIGTERM
- non tutti i messaggi di errore sono in rosso (quelli che non usano `fail` o `errno_fail` sono in bianco)
- se un figlio muore per qualsiasi errore bisogna segnalarlo al master e terminare tutti i processi
- wait per aspettare tutti i figlioli dei vari processi

LFT     17/01   06/02
SO      19/01   14/02
EPS     29/01   16/02

-<18>-> SO-Lab -<10>-> SO(1) -<17>-> LFT(2) -<10>-> EPS(2)
                                            ..............

-<27>-> LFT(1) -<12>-> EPS(1) -<6>-> SO-Lab -<10>-> SO(2)
               ..........................................