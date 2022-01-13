#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LINE_SIZE 300
#define MAX_COMMAND_SIZE 6

struct Dir;
struct File;

typedef struct Dir{
	char *name;
	struct Dir* parent;
	struct File* head_children_files;
	struct Dir* head_children_dirs;
	struct Dir* next;
} Dir;

typedef struct File {
	char *name;
	struct Dir* parent;
	struct File* next;
} File;

int index_pwd;

void free_files(Dir* parent) {
	if (parent == NULL) {
		return;
	}
	if (parent->head_children_files == NULL) { //daca nu are fisiere
		return;
	}

	File* tmp = parent->head_children_files;
	File* prev;

	while (tmp) { //stergem lista fisierelor
		prev = tmp;
		tmp = tmp->next;
		free(prev->name);
		free(prev);
	}
}

void free_dirs(Dir* parent) {
	if (parent == NULL) {
		return;
	}
	free_files(parent); //eliberam fisierele din directorul curent
	
	//daca nu sunt directoare de sters
	if (parent->head_children_dirs == NULL) {
		return;
	}

	Dir* tmp = parent->head_children_dirs;
	Dir* prev;

	//stergem recursiv directoarele si fisierele
	while (tmp) {
		prev = tmp;
		tmp = tmp->next;
		free_dirs(prev);
		free(prev->name);
		free(prev);
	}
}

void touch (Dir* parent, char* name) {
	if (parent == NULL) {
		return;
	}

	//cautam sa vedem daca mai exista un fisier cu acelasi nume
	if (parent->head_children_files != NULL) {
		File* iteration = parent->head_children_files;
		while (iteration != NULL) {
			if (strcmp(iteration->name, name) == 0) {
				printf("File already exists\n");
				free(name); //eliberam numele nefolosit
				return;
			}
			iteration = iteration->next;
		}
	}

	//daca nu exista un fisier cu acelasi nume, facem unul
	File* new_file = (File*)malloc(sizeof(File));
	new_file->name = name;
	new_file->next = NULL;
	new_file->parent = parent;

	if (parent->head_children_files == NULL) { //daca lista e goala
		parent->head_children_files = new_file;
	} else {
		File* tmp = parent->head_children_files;
		while(tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = new_file;
	}
}

void mkdir (Dir* parent, char* name) { 
	if (parent == NULL) {
		return;
	}

	//verificam daca mai exista un director cu acelasi nume
	if (parent->head_children_dirs != NULL) {
		Dir* iteration = parent->head_children_dirs;
		while (iteration != NULL) {
			if (strcmp(iteration->name, name) == 0) {
				printf("Directory already exists\n");
				free(name); //eliberam numele nefolosit
				return;
			}
			iteration = iteration->next;
		}
	}

	//daca nu s-a gasit un director cu acelasi nume, facem unul nou
	Dir* new_dir = (Dir*)malloc(sizeof(Dir));
	new_dir->name = name;
	new_dir->head_children_dirs = NULL;
	new_dir->head_children_files = NULL;
	new_dir->next = NULL;
	new_dir->parent = parent;

	if (parent->head_children_dirs == NULL) { //daca lista e goala
		parent->head_children_dirs = new_dir;
	} else {
		Dir* tmp = parent->head_children_dirs;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = new_dir;
	}
}

void ls (Dir* parent) { 

	//parcurgem fiecare lista in parte si printam ce gasim
	Dir* afis_dir = parent->head_children_dirs;
	while (afis_dir != NULL) {
		printf("%s\n", afis_dir->name);
		afis_dir = afis_dir->next;
	}
	File* afis_file = parent->head_children_files;
	while (afis_file != NULL) {
		printf("%s\n", afis_file->name);
		afis_file = afis_file->next;
	}
}

void rm (Dir* parent, char* name) { 
	if (parent == NULL) {
		return;
	}
	if (parent->head_children_files == NULL) { //daca lista e goala
		printf("Could not find the file\n");
		return;
	}
	File* tmp = parent->head_children_files;
	File* prev;

	if (strcmp(tmp->name, name) == 0) { //daca este primul element
		parent->head_children_files = tmp->next;
		free(tmp->name);
		free(tmp);
		return;
	}

	//parcurgem lista ca sa gasim elementul
	while(tmp != NULL && strcmp(tmp->name, name) != 0) {
		prev = tmp;
		tmp = tmp->next;
	}
	if (tmp) { //daca am gasit elementul, il stergem
		prev->next = tmp->next;
		free(tmp->name);
		free(tmp);
		return;
	}

	printf("Could not find the file\n");
}

void rmdir (Dir* parent, char* name) { 
	if (parent == NULL) {
		return;
	}
	if (parent->head_children_dirs == NULL) { //daca lista este goala
		printf("Could not find the dir\n");
		return;
	}
	Dir* tmp = parent->head_children_dirs;
	Dir* prev;

	if (strcmp(tmp->name, name) == 0) { //daca este primul element
		parent->head_children_dirs = tmp->next;
		free(tmp->name);
		free_dirs(tmp);
		free(tmp);
		return;
	}

	//parcurgem lista ca sa gasim elementul
	while (tmp != NULL && strcmp(tmp->name, name) != 0) {
		prev = tmp;
		tmp = tmp->next;
	}
	if (tmp) { //daca am gasit elementul, il stergem
		prev->next = tmp->next;
		free(tmp->name);
		free_dirs(tmp);
		free_files(tmp);
		free(tmp);
		return;
	}
	printf("Could not find the dir\n");
}

void cd(Dir** target, char *name) { 

	if(target == NULL) {
		return;
	}
	if(strcmp(name, "..") == 0) { //daca trebuie sa mergem inapoi in ierarhie
		if ((*target)->parent != NULL) { //verificam daca exista parinte
			*(target) = (*target)->parent;
			return;
		} else {
			return;
		}
	} else { //in caz contrar, cautam directorul in care sa ne mutam
		Dir* tmp = (*target)->head_children_dirs;
		while (tmp) {
			if (strcmp(tmp->name, name) == 0) {
				(*target) = tmp;
				return;
			}
			tmp = tmp->next;
		}
	}
	printf("No directories found!\n");
}

void pwd_rec (Dir* target, char* output) {
	Dir* tmp = target;

	if (tmp->parent != NULL) {
		//daca nu am ajuns inca la home, apelam recursiv
		pwd_rec(tmp->parent, output);

		index_pwd += sprintf(&output[index_pwd], "/%s", tmp->name);
	} else { //am ajuns in capul ierarhiei
		index_pwd += sprintf(&output[index_pwd], "/%s", tmp->name);
	}
}

char *pwd (Dir* target) {

	Dir* tmp = target;
	char* output = (char*)malloc(sizeof(char) * MAX_INPUT_LINE_SIZE);

	//ne folosim de o variabila globala declarata la linia 25 pentru a putea
	//evita suprascrierea caracterelor in output
	index_pwd = 0;
	pwd_rec(target, output);

	return output;

}

void stop (Dir* target) {
	free_dirs(target); //eliberam directoarele si fisierele din home
	free(target->name);
	free(target);
}

void tree (Dir* target, int level) { 
	if (target == NULL) {
		return;
	}

	Dir* tmp_dir = target->head_children_dirs;
	File* tmp_file = target->head_children_files;

	while (tmp_dir) { //afisam mai intai recursiv directoarele
		for(int ind = 0; ind < level; ind++) {
			printf("    "); //la fiecare nivel se afiseaza mai multe spatii
		}
		printf("%s\n", tmp_dir->name); //afisam in final si numele
		if (tmp_dir->head_children_dirs || tmp_dir->head_children_files) {
			tree(tmp_dir, level + 1); //apelam recursiv pt toate directoarele
		}
		tmp_dir = tmp_dir->next;
	}

	while (tmp_file) { //afisam si fisierele din directorul respectiv
		for(int ind = 0; ind < level; ind++) {
			printf("    ");
		}
		printf("%s\n", tmp_file->name);
		tmp_file = tmp_file->next;
	}
}

void mv(Dir* parent, char *oldname, char *newname) { 

	Dir* tmp_dir = parent->head_children_dirs;
	Dir* verif_dir = parent->head_children_dirs;
	File* tmp_file = parent->head_children_files;
	File* verif_file = parent->head_children_files;
	int file = 0, dir = 0;

	if (tmp_dir == NULL && tmp_file == NULL) { //daca nu exista
		free(newname);
		printf("File/Director not found\n");
		return;
	} 

	while (tmp_dir) { //cautam directorul care trebuie reactualizat
		if(strcmp(tmp_dir->name, oldname) == 0) {
			dir = 1;
			break;
		}
		tmp_dir = tmp_dir->next;
	}

	if (!dir) { //daca nu am gasit un director cu numele respectiv
		while (tmp_file) { //cautam fisierul care trebuie reactualizat
			if(strcmp(tmp_file->name, oldname) == 0) {
				file = 1;
				break;
			}
			tmp_file = tmp_file->next;
		}
	}

	if (!dir && !file) { //daca nu s-a gasit niciuna dintre cele 2
		free(newname);
		printf("File/Director not found\n");
		return;
	}

	
	while (verif_dir) { //verifcam daca exista deja
		if (strcmp(verif_dir->name, newname) == 0) {
			free(newname);
			printf("File/Director already exists\n");
			return;
		}
		verif_dir = verif_dir->next;
	}

	while (verif_file) { //verifcam daca exista deja
		if (strcmp(verif_file->name, newname) == 0) {
			free(newname);
			printf("File/Director already exists\n");
			return;
		}
		verif_file = verif_file->next;
	}
	
	if (dir) { //daca s-a gasit un director de reactualizat

		//stocam pointerii la listele de fisiere si de directoare in niste
		//pointeri auxiliari, dupa care taiem legaturile directorului
		//curent pentru a nu le elibera accidental
		Dir* aux_dir = tmp_dir->head_children_dirs;
		File* aux_file = tmp_dir->head_children_files;
		tmp_dir->head_children_dirs = NULL; 
		tmp_dir->head_children_files = NULL;
		rmdir(parent, oldname); //stergem directorul
		mkdir(parent, newname); //facem altul cu noul nume

		//refacem legaturile cu listele de fisiere si de directoare
		Dir* iter_dir = parent->head_children_dirs;
		while (iter_dir->next != NULL) {
			iter_dir = iter_dir->next;
		}
		iter_dir->head_children_dirs = aux_dir;
		iter_dir->head_children_files = aux_file;

	} else if (file) { //daca s-a gasit un fisier de reactualizat

		rm(parent, oldname); //stergem fisierul
		touch(parent, newname); //facem altul cu noul nume
	}
	
}

int main () {

	//Alocam directorul principal, denumit "home", care momentan nu are
	//subdirectoare sau fisiere in el
	Dir* home = (Dir*)malloc(sizeof(Dir));
	Dir* current_dir = home;
	home->name = (char*)malloc(sizeof(char) * 5);
	strcpy(home->name, "home");
	home->parent = NULL;
	home->head_children_dirs = NULL;
	home->head_children_files = NULL;

	int buton = 1; //conditia de iesire din bucla do while

	do
	{
		//alocam memorie inputului si il citim urmand sa il impartim in tokeni 
		char* input = (char*)malloc(sizeof(char) * MAX_INPUT_LINE_SIZE);

		fgets(input, MAX_INPUT_LINE_SIZE, stdin);
		char* command = strtok(input, " \n");
		char* token1 = strtok(NULL, " \n");
		char* token2 = strtok(NULL, "\n");

		//in functie de ce a fost in input, daca comanda are marimea medie a
		//unei comenzi (6 caractere cu tot cu '\0'), intram in if si verificam
		//daca se potriveste cu vreo comanda stiuta, pe scurt daca e relevanta
		if (strlen(command) <= MAX_INPUT_LINE_SIZE) {
			//verificam daca in input este o comanda cu sau fara parametrii
			if (token1 != NULL) {
				char* name = (char*)malloc(sizeof(char) * MAX_INPUT_LINE_SIZE);
				strcpy(name, token1); //copiem numele
				
				if (strcmp(command, "touch") == 0) {
					touch(current_dir, name);
				} else if (strcmp(command, "mkdir") == 0) {
					mkdir(current_dir, name);
				} else if (strcmp(command, "rm") == 0) {
					rm(current_dir, name);
					free(name);
				} else if (strcmp(command, "rmdir") == 0) {
					rmdir(current_dir, name);
					free(name);
				} else if (strcmp(command, "cd") == 0) {
					cd(&current_dir, name);
					free(name);
				} else if (strcmp(command, "mv") == 0 && token2) {
					//in cazul acesta, trebuie verificat daca exista si token2
					//adica daca exista si un nume nou, urmand sa alocam 
					//pentru el pentru a-l putea prelucra
					char* new_name = (char*)malloc(sizeof(char) 
														* MAX_INPUT_LINE_SIZE);
					strcpy(new_name, token2); //copiem noul nume
					mv(current_dir, name, new_name);

					free(name); //eliberam numele vechi
				} 
				else {
					free(name); //in cazul in care nu s-a tastat vreo comanda
					//relevanta, eliberam memoria alocata pentru nume
				}
			} else { //pentru comenzile fara parametrii
				if (strcmp(command, "stop") == 0) {
					stop(home);
					buton = 0; //setam butonul astfel incat sa iesim din bucla
				} else if (strcmp(command, "ls") == 0) {
					ls(current_dir);
				} else if (strcmp(command, "tree") == 0) {
					tree(current_dir, 0);
				} else if (strcmp(command, "pwd") == 0) {
					char *path = pwd(current_dir); //alocam memorie stringului
					printf("%s", path);
					free(path); //eliberam stringul dupa ce nu il mai folosim
				}
			}
		}
		free(input); //stergem inputul 
	} while (buton);
	
	return 0;
}
