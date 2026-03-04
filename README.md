# Projet : Simplification d'Images PBM

Implémentation d’un **solveur SAT** pour tester la satisfiabilité de formules CNF. 
Le projet inclut: 
- un solveur naïf.
- un solveur optimisée utilisant le two-watched literals pour la propagation unitaire et le backtracking.

---

## Exécutables et Utilisation :

### 1. solveur_naif

**Usage :**

Usage: bin/solveur_naif <file.cnf>

**Entrées :**
- <file.cnf> : fichier au format cnf

**Sorties :**
- Satisfiabilite ou Insatisfiablité de la formule cnf.
\(Ps: si la formule est satisfiable , l'affectation qui la satisfait est renvoyé)

## 2. solveur_ruse

**Usage :**

Usage: bin/solveur_ruse <file.cnf>

**Entrées :**
- <file.cnf> : fichier au format cnf

**Sorties :**
- Satisfiabilite ou Insatisfiablité de la formule cnf.
\(Ps: si la formule est satisfiable , l'affectation qui la satisfait est renvoyé)
