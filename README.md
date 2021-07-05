# WordCount_PCPC

Progetto del corso d’esame **Programmazione Concorrente e Parallela su Cloud**, Anno Accademico **2020/2021** curriculum **Cloud Computing**.

**Studente**: Liguorino Vincenzo

**Matricola**: 0522500840

**Dettagli per la compilazione e l’esecuzione**

Dato che il programma non è composta da molteplici file la compilazione si esegue con un semplice comando
```
mpicc wordCount.c -o esameLiguorino
```

L'esecuzione avviene tramite mpirun, specificando l'hostfile precedentemente creato

```
mpirun -np [N_PROC] –hostfile hfile  ./esameLiguorino
```

In fase di run del programma verrà chiesto all’utente il numero di file da inserire nella lista e successivamente il nome del file, nel mio caso all’interno del repository ci sono anche i tre file che sono stati utilizzati come test.

Report file: README_LIGUORINO.MD
