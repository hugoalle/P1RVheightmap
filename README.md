# P1RVheightmap
P1RV Hugo ALLEMAND et Léa Prémont

à compiler en x64

Le projet nécessite les librairies :
  - OpenGL (version 4.0 ou supérieure) , 
  - GLFW (version 3.3.2 utilisée), 
  - GLEW (version 2.1.0 utilisée), 
  - OpenCV (version 4 utilisée)
  et GLM (OpenGL Mathematics, version 0.9.9 utilisée)
  
Un fichiers de heightmap est présent comme exemple.

Une fois l'éxcutable lancé : 

Le programme demande de choisir entre le mode visualisation de terrain 3D et édition de heightmap
Entrer 0 pour visualisation de terrain 3D généré à partir d'une heightmap
Entrer 1 pour étditer une heightmap

Le programme demande ensuite le chemin du fichier de la heightmap

Si le mode visualisation est choisi, il est demandé le pas voulu entre les pixels.
Par exemple, un pas de 3 permet de ne considérer qu'un pixel sur 3. Choisir 1 si vous voulez considerer chaque pixel.

Les commandes pour se déplacer dans le terrain 3D sont :
fléches directionnelles pour avant/arriere, gauche/droite, o pour monter, i pour descendre.
La caméra se dirige en maintenant le clic gauche appuyé et en bougeant la souris
On peut aussi choisir le mode de remplissage des polygones avec L pour le mode LINE et F pour le mode FILL

Pour l'édition de heightmap :
On peut dessiner ou flouter en cliquant sur la heightmap ou en restant appuyé et en bougeant la souris.
Le rayon du crayon et la couleur se paramètre avec les trackbars


