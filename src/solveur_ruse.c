#include "solveur_ruse.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Clause* clause_new(size_t size, lit lits[static size]){
	Clause* c = malloc(sizeof(Clause)+sizeof(lit)*size);
	c->size = size;
	c->watcher1 = 0;
	c->watcher2 = (size == 1) ? 0 : 1;
	for (size_t i=0; i<size; i++){
		c->lits[i] = lits[i];
	}
	return c;
}

Formula* formula_new(size_t nvars){
	Formula* f = malloc(sizeof(Formula));
	f->nvars = nvars;
	f->nclauses = 0;
	f->capacity = 10;
	f->clauses = malloc(sizeof(Clause*)*(f->capacity));
	f->vars = malloc(sizeof(Variable)*nvars);
	for (size_t i=0; i<nvars; i++){
		f->vars[i].assign = -1;
		f->vars[i].watchers_pos = NULL;
		f->vars[i].watchers_neg = NULL;
	}
	f->assigned = malloc(sizeof(var)*nvars);
	f->nassigned = 0;
	return f;
}

void formula_free(Formula* f){
	Clause** pntr_clauses = f->clauses;
	size_t nclauses = f->nclauses;
	size_t nvars = f->nvars;
	WatchNode* tmp;
	for (size_t i=0; i<nclauses; i++){
		free(pntr_clauses[i]);
	}
	for (size_t i=0; i<nvars ; i++){
		while (f->vars[i].watchers_pos != NULL){
			tmp = f->vars[i].watchers_pos;
			f->vars[i].watchers_pos = f->vars[i].watchers_pos->next;
			free(tmp);
		}
		while (f->vars[i].watchers_neg != NULL){
			tmp = f->vars[i].watchers_neg;
			f->vars[i].watchers_neg = f->vars[i].watchers_neg->next;
			free(tmp);
		}
	}
	free(f->clauses);
	free(f->vars);
	free(f->assigned);
	free(f);
}

void watchnode_create_and_insert(Formula* f,Clause* c,size_t watch_idx){
	WatchNode* node = malloc(sizeof(WatchNode));
	node->lit_idx = watch_idx;
	node->clause = c;
	lit l = c->lits[watch_idx];
	if (l>0){
		node->next = f->vars[l-1].watchers_pos;
		f->vars[l-1].watchers_pos = node;
	}else{
		node->next = f->vars[-l-1].watchers_neg;
		f->vars[-l-1].watchers_neg = node;
	}
}

void formula_add_clause(Formula* f, Clause* c){
	if (f->capacity == f->nclauses){
		f->capacity += 10; 
		f->clauses = realloc(f->clauses,sizeof(Clause*)*(f->capacity));
	}
	f->clauses[f->nclauses] = c;                           
	f->nclauses ++;
	watchnode_create_and_insert(f,c,c->watcher1);
	if (c->watcher1 != c->watcher2){
		watchnode_create_and_insert(f,c,c->watcher2);
	}
}

int8_t lit_eval(Formula* f, lit l){
	var v = (l>0) ? l-1 : -l-1;
	int8_t val = f->vars[v].assign;
	if (val==-1){
		return -1;
	}
	if (l<0){
		return val ? 0 : 1;
	}
	return val;
}

int8_t clause_eval(Formula* f, Clause* c){
	size_t len = c->size; 
	for (size_t i=0 ; i<len ; i++){	
		lit l = c->lits[i];
		int8_t eval = lit_eval(f,l);
		if (eval == 1){
			return 1;
		}
	}
	return 0;
}

bool formula_eval(Formula* f){
	size_t len = f->nclauses;
	for (size_t i=0 ; i<len ; i++){ 
		Clause* c = f->clauses[i];
		if (!clause_eval(f,c)){
			return false;
		}
	}
	return true;
}

bool assign(Formula* f, var v, bool value){
	if (value){
		if (f->vars[v-1].assign == 0){
			return false;
		}
		f->vars[v-1].assign = 1;
	}else{
		if (f->vars[v-1].assign == 1){
			return false;
		}
		f->vars[v-1].assign = 0;
	}
	f->assigned[f->nassigned] = v;
	f->nassigned += 1;
	return true;
}

bool formula_naive_solve(Formula* f){
	uint64_t aff = 0;
	size_t nvars = f->nvars;
	do {
		for (size_t i=0; i<nvars; i++){
			int lsb = (aff>>i)&1;
			assign(f,i+1,lsb);
		}
		if (formula_eval(f)){
			return true;
		}	
		for (size_t i=0; i<nvars; i++){
			f->vars[i].assign = -1;
			f->nassigned -= 1;
		}
		aff++ ;
	}while(aff < (1ULL<<nvars));
	return false;
}

void remove_watch_node(Formula* f, Clause* c,lit l){
	size_t var_idx = (l>0) ? l-1 : -l-1;
	WatchNode **head = (l>0) ? &f->vars[var_idx].watchers_pos : &f->vars[var_idx].watchers_neg;
	WatchNode *prev = NULL;
	WatchNode *cur = *head;
	while (cur != NULL){
		Clause* tmp = cur->clause;
		if (tmp == c){
			if (prev != NULL){ 
				prev->next = cur->next;
			}else{ 
				*head = cur->next;
			}
			free(cur);
			return;
		}
		prev = cur;
		cur = cur->next;
	}
	/* si pas trouvé : rien à faire */
}

bool update_watched(Formula* f, var v){
	int8_t v_ass = f->vars[v-1].assign;			/* assignation de la variable Xv */
	if (v_ass == -1)
	{
		/* si la variable n'est pas assigné rien à mettre à jour */
		return true;
	}
	WatchNode* node = (v_ass==0) ? f->vars[v-1].watchers_pos : f->vars[v-1].watchers_neg;
	/* si la variable Xv est assigné à false alors on vérifie les clauses surveillant le littéral Xv
	 * si la variable Xv est assigné à true alors on vérifie les clauses surveillant le littéral !Xv */
	while (node != NULL){
		WatchNode* next_node = node->next;
		Clause* c = node->clause;
		size_t idx = node->lit_idx;		/* l'indice du littéral Xv dans la clause qui le surveille */ 
		size_t other_idx = (c->watcher1 == idx) ? c->watcher2 : c->watcher1; /* l'indice de l'autre watched-literal */
		if (lit_eval(f,c->lits[other_idx]) == 1 || lit_eval(f,c->lits[idx]) == 1){
			/* si l'un des deux littéraux surveillés est évalué à vrai on a gagné */
			node = next_node;
			continue;
		}
		bool flag = false;
		for (size_t i=0; i<(c->size) ; i++){
			if (i!=c->watcher1 && i!=c->watcher2){
			/* on parcourt tous les littéraux de la clause excepté les two-watched-literals */ 
				lit l = c->lits[i];
				size_t idx_l = (l>0) ? l-1 : -l-1;
				/* indice du litéral l */
				if (f->vars[idx_l].assign == -1 || lit_eval(f,l) == true){
					/* si le littéral est non assigné ou evalué à vrai on change le watched-literal 
					 * et on mets à jour les listes chainées ainsi que l'indice du watched-literal */
					watchnode_create_and_insert(f,c,i);
					remove_watch_node(f,c,(v_ass==0) ? v : -v);
					if(idx == c->watcher1){
						c->watcher1 = i;
					}else{
						c->watcher2 = i;
					}
					flag = true;
					break;
				}
			}
		}
		if (flag){
			node = next_node;
			continue;
		}
		lit l_bis = c->lits[other_idx];
		/* valeur de l'autre watched-literal */
		if (f->vars[(l_bis>0) ? l_bis-1 : -l_bis-1].assign == -1){
			/* si il est non assigné alors on l'assigne pour qu'il soit évalué à true */ 
			if (l_bis>0){
				assign(f,(l_bis>0) ? l_bis : -l_bis,1);	
			}else{
				assign(f,(l_bis>0) ? l_bis : -l_bis,0);	
			}
			node = next_node;
			continue;
		}else if (!lit_eval(f,l_bis)){
			return false;
		}
		node = next_node;
	}
	return true;
}

bool propagate(Formula* f, size_t from){
    	for (size_t i=from; i<(f->nassigned); i++){
        	var v = f->assigned[i];
        	bool res = update_watched(f,v);
        	if (!res){
            		return false;
        	}
    	}
    	return true;
}

void backtrack(Formula* f,var v){
    	while (f->nassigned > 0){
        	var last = f->assigned[(f->nassigned)-1];
        	f->vars[last-1].assign = -1;
        	f->nassigned--;
        	if (last == v) {
            		break;
        	}
    	}
}

var choose_variable(Formula* f){
	for (size_t i=0; i<f->nvars; i++){
		if (f->vars[i].assign == -1)
			return i+1;
	}
    	return 0; // aucune variable libre
}

bool solve(Formula* f){
	var stack[f->nvars];
    	size_t choice_top = 0;
    	while (!formula_eval(f)){
        	var v = choose_variable(f);
		/* Toutes les variables assignées */
       		if (v == 0){
			break;
		}
        	
		/* Empilement et assignation à vrai */
        	stack[choice_top] = v;
		choice_top++;
        	assign(f,v,1);
        	/* Propagation unitaire */
       		while (!propagate(f,f->nassigned-1)){
			/* Pile vide donc insatisfiable */
            		if (choice_top == 0){
				return false; 
			}
            		/* Backtrack si propagation échoue */
			choice_top--;
            		var last_choice = stack[choice_top];
            		backtrack(f,last_choice);
            		assign(f,last_choice,0);
		}
    	}
    	return formula_eval(f); // true si satisfiable, false sinon
}

Formula* read_cnf(const char* fname){
	FILE* f = fopen(fname, "r");
    	if (!f) {
        	perror("fopen");
        	exit(1);
    	}

    	size_t nvars = 0, nclauses = 0;
    	char line[4096]; // choix arbitraire d'un buffer de 4ko

    	/* cerche la ligne qui commence par p et rÃ©cupÃ¨re le nb de variable et de clauses */
    	while (fgets(line, sizeof(line), f)) {
        	if (line[0] == 'c'){
            		continue;
		}if (line[0] == 'p'){
            		if (sscanf(line, "p cnf %zu %zu", &nvars, &nclauses) == 2){
                		break;
			}
        	}
    	}
    	if (nvars <= 0 || nclauses <= 0) {
        	fprintf(stderr, "Invalid DIMACS header\n");
        	exit(1);
    	}
    	Formula* res = formula_new(nvars);

    	/* read clauses */
    	size_t maxlen = nvars; // une clause a au plus nvar litteraux
    	int32_t lits[maxlen];
    	size_t cidx = 0;
    	while (fgets(line, sizeof(line), f)) { // cidx < nclauses &&
        	if (line[0] == 'c' || line[0] == 'p'){
            		continue;
		}
        	size_t lidx = 0;
        	uint32_t lit = 1;
        	char* pos = line;
        	while (lidx <= maxlen) {
            		int n;
            		if (sscanf(pos, " %d%n", &lit, &n) != 1){
                		break;
			}
            		pos += n;
            		if (lit == 0){
                		break; // end of the clause
			}
            		lits[lidx++] = lit;
        	}
        	if (lidx > maxlen) {
            		fprintf(stderr, "Clause too long %zu (/%zu)\n", lidx, maxlen);
            		exit(1);
        	} else if (lidx == 0){
            		continue; /* skip empty lines */
		}
		cidx++;
        	Clause* c = clause_new(lidx, lits);
        	formula_add_clause(res, c);
    	}
    	if (cidx != nclauses) {
        	fprintf(stderr, "Warning: read %zu clauses, header said %zu\n", cidx, nclauses);
    	}
    	fclose(f);
    	return res;
}

int main(int argc, char* argv[argc]){
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file.cnf>\n", argv[0]);
        	return 1;
    	}
  	Formula* f = read_cnf(argv[1]);
	size_t nvars = f->nvars;
	if (!solve(f)){
		printf("Insatisfiable\n");	
	}else{
		printf("Satisfiable !\n");
		printf("L'affectation qui satisfait la formule est la suivante :\n[");
		for (size_t i=0; i<nvars-1; i++){
			printf("%d,",f->vars[i].assign);
		}
		printf("%d]\n",f->vars[nvars-1].assign);
	}
	formula_free(f);
	return 0;
}
