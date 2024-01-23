# TODO

## Master
- stampa dello stato

## Lifo
- documentazione
- gestione degli errori

## MIRA
- Ammirare il capolavoro
- Documentare !!!!
- come fa l'inibitore ad assolvere anche alla funzione di limitazione scissioni/riduzionei in scoria di atomi scissi?

## soctl
- progettarla e farla

## Misc
- [x] i file .h sono inclusi tra i file da passare a gcc nei makefile
- [x] mettere o togliere i nomi dei parametri da tutte le firme dei metodi
- [ ] tutti devono gestire il SIGTERM
- [ ] se un figlio muore per qualsiasi errore bisogna segnalarlo al master e terminare tutti i processi
- [x] wait per aspettare tutti i figli dei vari processi
- [x] dobbiamo controllare il fail di malloc/calloc? (No)
- [ ] eventualmente, fare in modo che mklifo(<lifo>, <segment_size>, <elem_size>, -1, <ignored>) si crei in autonomia il semaforo
- [ ] lifo e' molto debole nella gestione degli errori
- [ ] libreria sem da rivedere
  - [ ] signal interruptions
  - [ ] error handling
  - [ ] valori sopra SHRT_MAX danno ERANGE
- [ ] le system call che ritornano -1 spesso e volentieri hanno piu info in errno e potenzialmente è good practice checkarlo
- [ ] stostituzione nanosleep con alarm/timer


- [ ] SEM_ATT e SEM_ALI per controllare i componenti tramite inibitore
- [ ] stampa delle statistiche
- [x] nano sleep da sostituire con wait dei children e/o pause
- [ ] gestione degli errori tipo errno e -1 su varie sem_op e simili
- [ ] relazione
- [x] soctl 
- [ ] signal masking

- [ ] atomo che si forka senza execve
- [ ] logica running() meaningful signal
- [ ] pipe e ditchare fifo !!

LFT     17/01   06/02
SO      19/01   14/02
EPS     29/01   16/02

-<18>-> SO-Lab -<10>-> SO(1) -<17>-> LFT(2) -<10>-> EPS(2)
                                            ..............

-<27>-> LFT(1) -<12>-> EPS(1) -<6>-> SO-Lab -<10>-> SO(2)
               .......................................... 

|  Corso  | Appello | Data  | Delta-gg | Inizio prenotaz. | Fine prenotaz. |
|:-------:|:-------:|:------|:--------:|:----------------:|:--------------:|
|   LFT   |    1    | 17/01 |    -     |        -         |       -        |
|   EPS   |    1    | 29/01 |    12    |      09/01       |     22/01      |
| SO Lab  |    2    | 04/02 |    6     |        -         |     03/02      |
| LFT Lab |    2    | 13/02 |    ~8    |      24/01       |     06/02      |
|   SO    |    2    | 14/02 |   ~10    |      25/01       |     07/02      |


|  Corso  | Appello | Data  | Delta-gg | Inizio prenotaz. | Fine prenotaz. |
|:-------:|:-------:|:------|:--------:|:----------------:|:--------------:|
|   LFT   |    1    | 17/01 |    -     |        -         |       -        |
| SO Lab  |    2    | 04/02 |   ~17    |        -         |     03/02      |
| LFT Lab |    2    | 13/02 |   ~27    |      24/01       |     06/02      |
|   SO    |    2    | 14/02 |   ~28    |      25/01       |     07/02      |
|   EPS   |    2    | 16/02 |   ~30    |      27/01       |     09/02      |


## Inibitore

### Limitazione del numero di scissioni

### Assorbimento di parte dell'energia prodotta dalla scissione

### Trasformazione di un atomo in scoria dopo la scissione
- tremite kill con SIGTERM dell'atomo parent (ossia dell'unico di cui puo' conoscere a priori il pid)



[x] atomo che si forka
[x] master che aspetta che tutti abbiano fatto le loro init
[x] masking segnali a giro
[ ] setuppare bene SIGTERM su tutti
[ ] alarm sul master



## Controlli pre esame
- [ ] error handling ovunque
- [ ] memory leak
- [ ] risorse IPC leak



1)

            Total    Last sec   Free   Inhib
Atoms       13534    50         -      -
Waste       3425     12         -      -
Energy      1234     2143       50     345

Remaining time: 12s
Status: running






2)
        Global [running]
        Total    LastSec
Atoms   13534    50
Waste   3425     12
Fission 934      14
Att     230      20

        Inhibitor [on]
        Total    LastSec
Fission 234      4
Energy  4345     70
Waste   342      11

        Total    Remaining
Time    87s      50s

        Total    Used   LastSec   Free
Energy  1234     1204   130       30











0 [====================|=========       ] 1000  MAX_EXPLODE_THRESHOLD
└ 600                       ENERGY_DEMAND






// atomo
semop(A-1, M-1)
    e += de
if (semop(IP=0, IPC_NOWAIT) == -1)
    // non presente
    semop(M+1)
    semop(A+1)
else
    // presente
    semop(I+1)


// inibitore da rifare sulla base del nuovo semaforo IP (Inibitore Presente)
semop(M-1)
    fresh_start = 1
    semop(I-1)

printf("Sono collegato.\n");

while (!interrupted) {
    if (!fresh_start) {
        semop(I-1)
    }
    
    e = max(...)
    fresh_start = 0
    if(!interrupted){
        semop(M+1)
        semop(A+1)
    }
}

semop(I+1, M+1)
semop(A+1)

exit();


// master
semop(M-1)
    read()
semop(M+1)


A -> M -> I
A -> I -> M







semop(IP-1)

printf("Sono collegato.\n");

while (!interrupted) {
    semop(I-1)

    // mask start

    e = max(...)
    semop(M+1)
    // mask end

    if (interrupted) {
        semop(IP+1)
    }

    semop(A+1)
}

exit();


soctl run --meltdown --inhib
    source env/meltdown.sh
    ./master --inhib

soctl stop
    kill -SIGTERM $(preg master)

soctl inhibitor <start/stop/toggle>
    start  = ./inhibitor_ctl 1
    stop   = ./inhibitor_ctl 0
    toggle = ./inhibitor_ctl



        WITHOUT INHIBITOR
TIMEOUT EXPLODE BLACKOUT MELTDOWN


 WITH INHIBITOR
TIMEOUT BLACKOUT


A -> a