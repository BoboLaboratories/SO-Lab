# TODO

## Master
- [x] stampa dello stato
- [x] (mai stati) memory leak frargs
- [ ] se va in meltdown prima ancora di eseguire il primo secondo non stampa nulla, esce e basta

## soctl
- [x] terminarla

## Misc
- [x] i file .h sono inclusi tra i file da passare a gcc nei makefile
- [x] mettere o togliere i nomi dei parametri da tutte le firme dei metodi
- [x] tutti devono gestire il SIGTERM
- [x] (no) se un figlio muore per qualsiasi errore bisogna segnalarlo al master_pid e terminare tutti i processi
- [x] wait per aspettare tutti i figli dei vari processi
- [x] (no) dobbiamo controllare il fail di malloc/calloc?
- [x] (no) eventualmente, fare in modo che lifo_create(<lifo>, <segment_size>, <elem_size>, -1, <ignored>) si crei in autonomia il semaforo
- [x] lifo è molto debole nella gestione degli errori
- [x] libreria sem da rivedere
  - [x] (se la deve vedere il chiamante) signal interruptions
  - [x] error handling
  - [x] (we are powerless) valori sopra SHRT_MAX danno ERANGE
- [x] le system call che ritornano -1 spesso e volentieri hanno piu info in errno e potenzialmente è good practice checkarlo
- [x] sostituzione nanosleep con alarm/timer
- [x] signal masking
- [x] atomo che si forka senza execve
- [x] (dismissed) logica running() meaningful signal

## Next big steps
- [x] gestione TIMEOUT
- [x] stampa statistiche
- [x] (no) pipe e ditchare fifo?
- [ ] prepare opportuni config in env per i vari scenari
- [ ] commento e controllo dei processi principali (incluso inhibitor_ctl)
- [ ] relazione
- [x] (con dolore) log delle operazioni svolte dall'inibitore

## Relazione
- [ ] versione di make e versione di gcc
- [ ] ogni processo può essere terminato in qualsiasi momento perché facciamo uso di tecniche tipo atexit
- [ ] lifo in shared memory con resize
- [ ] sigsuspend
- [ ] MIRA
- [ ] screen di una schermata di statistiche con guida su come leggerla

## Controlli pre esame
- [ ] rivedere commenti (e tutti i TODO)
- [ ] error handling ovunque
- [x] eliminare sandobox
- [x] risorse IPC leak
- [ ] memory leak
- [ ] non consegnare .git repo
- [ ] eliminare questo file

## Da sapere all'esame
- [ ] come funziona il nostro progetto (source: relazione + commenti nei sorgenti)
- [ ] comandi di linux, permessi e altre cose insegnate a lezione (source: slide)
- [ ] ripasso generale delle slide (source: slide)
- [ ] saper usare man (source: slide)


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
Energy  4345     70
Waste   342      11

        Total    Remaining
Time    87s      50s

        Total    LastSec    Used    Free
Energy  1234     1204       130     30








0 [====================|=========       ] 1000  MAX_EXPLODE_THRESHOLD
└ 600                       ENERGY_DEMAND







soctl run --meltdown --inhib
    source env/meltdown.sh
    ./master_pid --inhib

soctl stop
    kill -SIGTERM $(preg master_pid)

soctl inhibitor <start/stop/toggle>
    start  = ./inhibitor_ctl 1
    stop   = ./inhibitor_ctl 0
    toggle = ./inhibitor_ctl
