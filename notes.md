- i file .h sono inclusi tra i file da passare a gcc nei makefile
- mettere o togliere i nomi dei parametri da tutte le firme dei metodi
- eseguire free_ipc al SIGTERM
- non tutti i messaggi di errore sono in rosso (quelli che non usano `fail` o `errno_fail` sono in bianco)
- se un figlio muore per qualsiasi errore bisogna segnalarlo al master e terminare tutti i processi
- wait per aspettare tutti i figlioli dei vari processi
- dobbiamo controllare il fail di malloc/calloc?

LFT     17/01   06/02
SO      19/01   14/02
EPS     29/01   16/02

-<18>-> SO-Lab -<10>-> SO(1) -<17>-> LFT(2) -<10>-> EPS(2)
                                            ..............

-<27>-> LFT(1) -<12>-> EPS(1) -<6>-> SO-Lab -<10>-> SO(2)
               ..........................................


## Inibitore

### Limitazione del numero di scissioni

### Assorbimento di parte dell'energia prodotta dalla scissione

### Trasformazione di un atomo in scoria dopo la scissione
- tremite kill con SIGTERM dell'atomo parent (ossia dell'unico di cui puo' conoscere a priori il pid)



[ ] atomo che si forka
[x] master che aspetta che tutti abbiano fatto le loro init
[ ] masking segnali a giro
[ ] setuppare bene SIGTERM su tutti
[ ] alarm sul master





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











0 [==============================       ] 1000  MAX_EXPLODE_THRESHOLD
└ 600                       ENERGY_DEMAND