CERINTA

Se cere implementarea unui program MPI scalabil, capabil sa aplice mai multe filtre pe imagini de tipul PNM sau PGM. 


IMPLEMENTARE 

===================
Fisierul homework.c
===================
Am creat doua functii auxiliare(allocMatrix si applyFilter).
Parsarea fisierelor de input a fost facuta cu fgets, pentru headere si cu fread pentru a creea matricea de pixeli.
Structura de tip imagine contine 4 campuri de tip int ce reprezinta culoarea, latimea, inaltimea si valoarea maxima a unui pixel al unei poze.
Cel de-al 5-lea camp este o matrice de tip unsigned char ce va retine valorile propriu-zise ale pixelilor din imagine.
Scrierea noii imagini este realizata cu fprintf pentru headere si fwrite pentru matricea de pixeli.
le
Cu procesul cu rank-ul 0 (root) citesc imaginea initiala si o scriu in cea de output dupa ce am aplicat toate filtrele necesare astfel:
	- dupa ce a fost citita imaginea de input, o trimit pe aceasta intreaga catre toate celelalte procese folosind MPI_Bcast, acestea
avand nevoie de ea
	- pentru a trimite matricea de pixeli, am transformat-o intr-un vector, iar fiecare proces dupa ce primeste acest vector il transforma
inapoi in matrice pentru a aplica prelucrarile necesare aplicarii filtrului
	- acum ca fiecare proces cunoastea imaginea de input, isi creeaza o imagine de output intermediara, aplicand primul filtru pe imaginea
primita, doar pe partea care ii corespunde lui ( am paralelizat parcurgerea matricii de pixeli pe inaltime ( height ) )
	- fiecare proces, dupa ce a realizat prelucrarile necesare (functia applyFilter), trimite catre root doar partea pe care a aplicat
filtrul( de la start la end ). Apoi (root) calculeaza un start si un end pentru fiecare proces existent, pentru a stii unde in matricea
de pixeli sa puna ceea ce primeste de la celelalte procese, recompunand astfel imaginea initiala, dupa aplicarea primului filtru
	- imaginea de output intermediara este apoi trimisa de catre root catre celelalte procese pentru a servi ca noua matrice de input
pentru aplicarea urmatorului filtru
	- se repeta pasii anteriori pana au fost aplicate toate filtrele
	- dupa ce s-au epuizat toate filtrele, root scrie in fisierul de output ultima imagine intermediara de output recompusa
