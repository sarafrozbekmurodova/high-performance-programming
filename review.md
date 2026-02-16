
# Analyse du Projet Galsim (Code et Instructions)

## 1. Méta-Analyse Optimisée (Information Condensée)

Ce document fournit un aperçu structurel et fonctionnel optimisé pour traitement algorithmique.

*   **Sujet** : Simulation N-Corps Gravitationnelle (2D).
*   **Contrainte Performance** : Code Série Optimisé (Threading interdit).
*   **Algorithme** : Force N-Corps Naïve $O(N^2)$, Intégration Euler Symplectique.
*   **Langage** : C (C99+), Bibliothèques X11, Math.
*   **Fichiers Clés** : `galsim.c` (Logique unique), `makefile`.

---

## 2. Analyse Fonctionnelle Détaillée

### A. Modèle Physique et Mathématique
Le code implémente textuellement la variante de force spécifiée, potentiellement distincte du modèle standard de Plummer.

1.  **Formule de Force ($F_{ij}$)** :
    *   Le code utilise : $\vec{F}_{ij} = -G m_i m_j \frac{ \vec{r}_{ij} }{ (\|\vec{r}_{ij}\| + \epsilon_0)^3 }$
    *   *Note Critique* : Implémenté via `pow(distance + epsilon, 3)`. Ceci diffère du standard $\frac{1}{(r^2 + \epsilon^2)^{3/2}}$ souvent utilisé pour éviter la racine carrée `sqrt`, mais correspond à l'instruction littérale `(rij + e0)^-3`.
    *   **Variables** :
        *   $G = \frac{100}{N}$ (Dynamique calculé à chaque tour).
        *   $\epsilon_0 = 10^{-3}$ (Constante `epsilon`).

2.  **Intégration Temporelle (Euler Symplectique)** :
    *   Mise à jour Vitesse : $v_{n+1} = v_n + \Delta t \cdot a_n(x_n)$
    *   Mise à jour Position : $x_{n+1} = x_n + \Delta t \cdot v_{n+1}$
    *   *Implémentation* : Correcte dans `step()`. Les vitesses sont accumulées dans `temp_particles` puis appliquées *avant* la mise à jour des positions.

### B. Flux d'Exécution (`main`)
1.  **Initialisation** : Parse arguments (N, fichier, nsteps, $\Delta t$, graphics).
2.  **Chargement (`read_file`)** :
    *   Lecture séquentielle binaire $N \times$ `struct Particle`.
    *   Complexité : $O(N)$.
3.  **Boucle de Simulation (`for 0..nsteps`)** :
    *   Appel `step()`.
    *   Gestion du temps : `clock_gettime(CLOCK_MONOTONIC)`.
4.  **Finalisation** :
    *   Sortie graphique (nettoyage).
    *   Écriture fichier (`write_file`) binaire identique à l'entrée.
    *   Mesure temps "Wall seconds" (excluant I/O, incluant calculs).

---

## 3. Analyse Structurelle du Code (`galsim.c`)

### A. Structures de Données
*   **Type** : Array of Structures (AoS).
*   **`struct Particle`** :
    *   `double x_pos, y_pos` (Position).
    *   `double mass` (Masse Constante).
    *   `double x_velocity, y_velocity` (Vitesse).
    *   `double brightness` (Donnée passive).
    *   *Taille* : $6 \times 8 = 48$ octets/particule. Alignement standard.
*   **`struct ParticleChange`** :
    *   Accumulateur temporaire pour les deltas de vitesse ($\Delta v$).
    *   *Usage* : `temp_particles` est `memset` à 0 à chaque `step()`.

### B. Algorithme `step()` (Cœur du Calcul)
1.  **Reset** : `memset` de `temp_particles`.
2.  **Boucle de Force $O(N^2)$** :
    *   Double boucle imbriquée : `i` de 0 à $N$, `j` de $i+1$ à $N$.
    *   **Optimisation Newton 3** : Exploite $F_{ji} = -F_{ij}$ pour réduire les calculs de moitié.
    *   **Opérations Coûteuses** :
        *   `sqrt` pour `distance` (inévitable avec la formule $(r+\epsilon)$).
        *   `pow(..., 3)` pour le dénominateur (très coûteux, à optimiser par `x*x*x`).
        *   Division : `force_multiplier = G / ...`.
    *   **Accumulation** : Les contributions sont ajoutées à `temp_particles` (Lecture/Écriture non-séquentielle potentielle pour `j`).
3.  **Mise à jour $O(N)$** :
    *   Boucle simple pour appliquer `temp_particles` et mettre à jour `x_pos`, `y_pos`.
4.  **Graphismes** : Appel conditionnel à `draw_galaxy`.

---

## 4. Conformité et Performance (Review)

| Critère | État | Analyse / Recommandation |
| :--- | :--- | :--- |
| **Complexité Algorithmique** | $O(N^2)$ | Conforme. Implémentation "pairs" efficace. |
| ** Modèle Mémoire** | AoS (Structs) | **Sous-optimal**. L'accès mémoire n'est pas contigu pour les calculs vectoriels (SIMD). Une structure SoA (Structure of Arrays) améliorerait la vectorisation et le cache. |
| **Opérations Mathématiques** | Lourd | `pow(x,3)` est inefficace comparé à `x*x*x`. La division est lente, préférer une multiplication par l'inverse si possible (ici difficile car terme variable). |
| **Vectorisation** | Partielle | Le compilateur (`-ftree-vectorize`) aura du mal avec l'AoS et la dépendance des données dans la boucle interne (`temp_particles[j]`). |
| **I/O Fichier** | Binaire | Conforme. Lecture/Écriture directe de blocs mémoire (`fwrite` du tableau entier). |
| **Graphisme** | X11 | Fonctionnel mais doit être désactivé pour les mesures de performance (`graphics=0`). |
| **Portabilité** | Valide | Utilise X11 standard et POSIX `clock_gettime`. |

### Résumé pour l'IA
Le projet est une implémentation fonctionnelle et directe des instructions.
*   **Point fort** : Respect strict de la logique physique demandée et de l'intégration symplectique.
*   **Point faible (Performance)** : L'utilisation de `pow` et la structure AoS limitent les performances "High Performance".
*   **Structure** : Monolithique, facile à analyser statiquement.
