#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 10
typedef int32_t lit;
typedef int16_t var;

typedef struct Variable {
	int8_t assign; // -1 unassigned, 0 false, 1 true
} Variable;

typedef struct Clause {
    	size_t size;
    	lit lits[];
} Clause;

typedef struct Formula {
    	size_t nvars;
    	size_t nclauses;
    	size_t capacity;

    	Clause** clauses;
    	Variable* vars;

    	var* assigned;
    	size_t nassigned;
} Formula;

/**
 * Fonction pour allouer et intialiser une Clause.
 * @param size le nombre de littÃ©raux dans la clause
 * @param lits le tableau des littÃ©raux qui seront copiÃ©s dans la clause
 * @return un pointeur var la clause crÃ©Ã©e
 */
Clause* clause_new(size_t size, lit lits[static size]);

/**
 * Fonction pour allouer et intialiser une Formule.
 * Une formule aura une capacitÃ© de 10 clause et si elle devient pleine alors la capacitÃ© sera augmentÃ©e par incrÃ©ment de 10
 * @param nvars le nombre de variables utilisÃ©es dans la formule
 * @return un pointeur var la formule crÃ©Ã©e
 */
Formula* formula_new(size_t nvars);

void formula_free(Formula* f);

void formula_add_clause(Formula* f, Clause* c);

int8_t lit_eval(Formula* f, lit l);

int8_t clause_eval(Formula* f, Clause* c);

bool formula_eval(Formula* f);

bool assign(Formula* f, var v, bool value);

bool formula_naive_solve(Formula* f);

Formula* read_cnf(const char* fname);
