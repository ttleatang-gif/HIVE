# Implémentation du jeu Hive — Synthèse des outils

## I. Composants de la bibliothèque standard C++

### 1. Classes de conteneurs (STL Containers)
- **`std::vector`** - Structure de données centrale
  - Stockage de l’état du plateau `map` (vecteur tridimensionnel)
  - Stockage des coordonnées `{x, y, z}`
  - Stockage de la liste des positions successeures
  - Structure sous-jacente de la classe personnalisée `Queue`

- **`std::unordered_map`** - Recherche efficace des positions
  - Clé : valeur de hachage de la position (`int`)
  - Valeur : liste des pièces sur cette position (`vector<pair<int, int>>`)
  - Usage : retrouver rapidement toutes les pièces situées sur une case donnée

- **`std::pair`** - Type de données combiné
  - Stockage de la paire `{numéro du joueur, numéro de la pièce}`
  - Utilisé dans la table de recherche `find`

### 2. Bibliothèque d’algorithmes (`Algorithm`)
- **`std::find`** - Recherche d’éléments
  - Vérifie dans la fonction `dfs` si un chemin contient déjà une certaine position
  - Recherche d’éventuelles positions en double dans la liste des successeurs

- **`std::remove_if`** - Suppression conditionnelle
  - Supprime certains enregistrements de pièces dans `getLegalOneSteps`

### 3. Traitement des chaînes de caractères
- **`std::string`** - Opérations sur les chaînes
  - Utilisé dans la fonction de hachage pour convertir les coordonnées en chaîne
  - `std::to_string` pour la conversion des valeurs numériques
  - `std::stringstream` pour le formatage de l’affichage

### 4. Bibliothèque mathématique
- **`<cmath>`** - Fonctions mathématiques
  - `sqrt()` calcule la distance euclidienne (pour détecter la position du clic de souris)

### 5. Gestion des exceptions
- **`<stdexcept>`** - Classes d’exception
  - `std::runtime_error` utilisé lorsqu’une file est vide

### 6. Objets fonctionnels
- **`std::hash<std::string>`** - Objet de fonction de hachage
  - Transforme une chaîne de coordonnées en valeur de hachage
  - Utilisé comme clé dans la table de recherche des positions

## II. Bibliothèque de rendu graphique

### SFML (Simple and Fast Multimedia Library)
- **`SFML/Graphics.hpp`** - Rendu graphique
  - `RenderWindow` - Fenêtre du jeu
  - `CircleShape` - Dessin des pièces (forme circulaire)
  - `Vertex` / `Lines` - Dessin de la grille
  - `Color` - Définition des couleurs
  - `Text` / `Font` - Affichage du texte
  - `Vector2f` / `Vector2i` - Vecteurs bidimensionnels

- **`SFML/Window.hpp`** - Gestion de la fenêtre et des événements
  - `Event` - Gestion des événements
  - `Mouse` - Entrée souris
  - `VideoMode` - Configuration du mode vidéo
  - `Clock` - Contrôle du temps (délai de l’IA)

## III. Algorithmes principaux

### 1. Algorithmes de recherche sur graphe
- **BFS（recherche en largeur）**
  - Emplacement de l’implémentation : `getAntSuccessor()` (lignes 346-377)
  - Usage : calcul de toutes les positions possibles pour la fourmi (`Ant`)
  - Caractéristique : permet de trouver toutes les positions atteignables, utile pour une pièce à déplacement illimité

- **DFS（recherche en profondeur）**
  - Emplacement de l’implémentation : `dfs()` (lignes 380-397) et `getSpiderSuccessor()` (lignes 401-405)
  - Usage : calcul du déplacement de l’araignée (`Spider`) (exactement 3 pas)
  - Caractéristique : profondeur limitée à 3, évite les chemins répétés

### 2. Algorithme de vérification de connectivité
- **Emplacement de l’implémentation** : `isConnectivity()` (lignes 491-543)
- **Type d’algorithme** : parcours BFS
- **Usage** : vérifier que toutes les pièces du plateau restent connectées (règle essentielle de Hive)
- **Points clés** :
  - Utilise un BFS à partir d’une position initiale pour parcourir toutes les pièces adjacentes
  - Utilise un tableau de marquage pour indiquer les pièces déjà visitées
  - Vérifie finalement si toutes les pièces ont bien été atteintes

### 3. Technique de hachage
- **Fonction de hachage des positions** : `hashPos()` (lignes 54-57)
  - Entrée : coordonnées tridimensionnelles `{x, y, z}`
  - Traitement : conversion en chaîne puis hachage
  - Sortie : valeur entière
  - Usage : sert de clé dans la table de recherche des positions

## IV. Structures de données personnalisées

### 1. Classe `Queue`
- **Emplacement de l’implémentation** : lignes 26-49
- **Fonction** : file personnalisée stockant des éléments de type `vector<int>`
- **Usage** :
  - Structure auxiliaire pour les algorithmes BFS
  - Utilisée dans la vérification de connectivité
- **Mode d’implémentation** : basée sur `vector` (utilise `erase` pour simuler le retrait en tête)

### 2. Système de coordonnées hexagonales
- **Décalages de coordonnées** : `NEAR` (ligne 113)
  - 6 directions de décalage : `{0,1}, {1,1}, {1,0}, {0,-1}, {-1,-1}, {-1,0}`
  - Utilisé pour calculer les positions adjacentes

### 3. Système de coordonnées tridimensionnelles
- **Format des coordonnées** : `{x, y, z}`
  - `x, y` : coordonnées du plateau bidimensionnel (grille hexagonale)
  - `z` : hauteur (pour gérer l’empilement du scarabée)

## V. Outils de logique du jeu

### 1. Gestion des états
- **Copie profonde** : `deepCopy()` (lignes 614-632)
  - Copie complète de l’état du jeu
  - Utilisée dans la recherche de l’IA et le retour arrière des états

### 2. Normalisation des états
- **Normalisation du plateau** : `normalize()` (lignes 636-736)
  - Translation : déplace la première pièce à l’origine
  - Rotation : essaie les 6 rotations de 60 degrés et choisit celle ayant le meilleur score
  - Symétrie axiale : compare avant et après réflexion
  - Usage : comparaison et élimination des états équivalents

### 3. Système de pondération
- **Poids angulaire** : `angleWeight` (lignes 80-98)
  - Matrice `60x60`, attribuant des poids selon la position angulaire

- **Poids axial** : `axleWeight` (lignes 101-107)
  - Tableau de 60 éléments, pondération linéaire

- **Poids de type** : `kindWeight` (ligne 129)
  - Poids attribués selon le type de pièce et le joueur
  - Utilisé pour l’évaluation lors de la normalisation

## VI. Outils liés à l’IA

### 1. Génération de nombres aléatoires
- **`<random>`** - Inclus mais non utilisé directement dans le code
- **Usage prévu** : prise de décision aléatoire pour l’IA

### 2. Contrôle du délai de l’IA
- **`Clock`** - Classe temporelle de SFML
- **Usage** : contrôle du temps de réflexion de l’IA pour une meilleure expérience de jeu

## VII. Entrées / sorties de fichiers

### 1. Chargement de police
- **`Font::loadFromFile()`** - Chargement du fichier de police
- **Usage** : affichage des textes et informations du jeu

## VIII. Fonctions utilitaires

### 1. Conversion de coordonnées
- **Rotation hexagonale** (lignes 665-667)
  - Formule de rotation de 60 degrés : `x' = -y`, `y' = x + y`

- **Symétrie axiale** (lignes 709-712)
  - Formule de réflexion : `x' = -a`, `y' = a + b`

### 2. Calcul de distance
- **Distance euclidienne** (ligne 841)
  - `sqrt(dx² + dy²)`
  - Usage : trouver la position la plus proche du clic de souris

### 3. Recherche de position
- **`getBlank()`** (lignes 240-257)
  - Récupère les positions vides autour d’une pièce

- **`isCovered()`** (lignes 330-337)
  - Vérifie si une pièce est recouverte par une autre

## IX. Résumé

### Dépendances principales
1. **Conteneurs STL** : `vector`, `unordered_map`, `pair` - stockage et recherche des données
2. **Bibliothèque SFML** : rendu graphique et interaction utilisateur
3. **Algorithmes de graphe** : BFS, DFS - calcul des déplacements des pièces
4. **Technique de hachage** : recherche rapide des positions
5. **Système de coordonnées** : grille hexagonale + coordonnées tridimensionnelles

### Modèles de conception clés
- **Patron d’état** : gestion des états du jeu
- **Patron de stratégie** : règles de déplacement selon le type de pièce
- **Patron de table de recherche** : accélération de la recherche grâce au hachage

### Optimisations de performance
- Utilisation de `unordered_map` pour une recherche de position en `O(1)`
- Mise en cache des informations de position pour éviter des calculs répétés
- Optimisation des déplacements des pièces via BFS / DFS