#include "solveur_naif.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Clause* clause_new(size_t size, lit lits[static size]){
	Clause* c = malloc(sizeof(Clause)+sizeof(lit)*size);
	c->size = size;
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
	}
	f->assigned = malloc(sizeof(var)*nvars);
	f->nassigned = 0;
	return f;
}

void formula_free(Formula* f){
	Clause** pntr_clauses = f->clauses;
	size_t nclauses = f->nclauses;
	for (size_t i=0; i<nclauses; i++){
		free(pntr_clauses[i]);
	}
	free(f->clauses);
	free(f->vars);
	free(f->assigned);
	free(f);
}

void formula_add_clause(Formula* f, Clause* c){
	if (f->capacity == f->nclauses){
		f->capacity += 10; 
		f->clauses = realloc(f->clauses,sizeof(Clause*)*(f->capacity));
	}
	f->clauses[f->nclauses] = c;
	f->nclauses ++;
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
			f->vars[i].assign = lsb;
		}
		if (formula_eval(f)){
			return true;
		}	
		aff++ ;
	}while(aff < (1ULL<<nvars));
	return false;
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
	if (!formula_naive_solve(f)){
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

