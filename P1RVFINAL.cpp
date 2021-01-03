// P1RV_heightmap_1.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//
//  main.cpp

//  P1RV_19102020

//

//  Created by Hugo ALLEMAND on 19/10/2020.

//  Copyright © 2020 Hugo ALLEMAND. All rights reserved.

//


#include <Windows.h>

//librarire de la stl
#include <string>
#include <iostream>

//OpenGL maths
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

//librarie pour OpenGL et fenetrage
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

//librairie OpenCV pour charger la heightmap
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/ml.hpp>

#include "blurBrush.h"
#include "circlebrush.h"


using namespace std;
using namespace cv;

int modeRendu = GL_FILL;

// ============================== VARIABLE POUR LE RENDU 3D DE LA HEIGHTMAP =====================================

//position de la souris et sensibilité pour le deplacement
double mouseX = -1;
double mouseY = -1;
float sensibilite = 0.004;

//angles de rotation (coordonees spheriques) avec leurs increments
float theta = 0;
float phi = 0;
float deltaTheta = 0.0;
float deltaPhi = 0.0;

//increments de deplacement
float deltaMove = 0;
float deltaStrafe = 0;
float deltaUp = 0;

//vecteur avant
float forwardViewX = 1;
float forwardViewY = 1;
float forwardViewZ = 1;

//vecteur "a droite"
float rightX = 1;
float rightY = 1;
float rightZ = 1;

//Dimension de la fenêtre d'affichage
int windowW = 640;
int windowH = 640;

//vecteur up world
float upX = 0;
float upY = 0;
float upZ = 1;

//vecteur position de la camera
float camPosX = 0;
float camPosY = 0;
float camPosZ = 255;

//vers quelle coordonée la caméra regarde
float camAtX = 500;
float camAtY = 500;
float camAtZ = 0;

//Pour stocker l'image 
Mat img;

//Matrice du modele : determine où est le modèle dans le monde (ici à l'origine car matrice identité)
glm::mat4 Model = glm::mat4(1.0f);

//Matrice de vue : determine où l'origine du monde est par rapport à la caméra
glm::mat4 View = glm::lookAt(
    glm::vec3(camPosX, camPosY, camPosZ),
    glm::vec3(camAtX, camAtY, camAtZ),
    glm::vec3(upX, upY, upZ));

// Matrice de projection (perpective) : 45° Field of View, w/h ratio, 
//display range : 0.1 unit <-> 2000 units FAIRE ATTENTION A CE DERNIER PARAMETRE sinon on ne verra rien !
glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)windowW / (float)windowH, 0.1f, 2000.0f);

//la matrice MVP
glm::mat4 mvp = Projection * View * Model; //matrix multiplication is the other way around

//echelle avec degrade de couleur realiste
double echelleCouleurHauteur[9][3] = {
    {0, 0.40, 1},
    {0.11, 0.67, 0.11},
    {0.17, 0.83, 0.20},
    {0.86, 0.84, 0},
    {0.78, 0.57, 0.13},
    {0.65, 0.45, 0.12},
    {0.59, 0.62, 0.62},
    {0.78, 0.9, 0.87},
    {1, 1, 1}
};

//Echelle de couleur par hauteur
void choixCouleurHauteur(double& r, double& g, double& b, int height)
{
    height = height / (255 / 9);
    r = echelleCouleurHauteur[height][0];
    g = echelleCouleurHauteur[height][1];
    b = echelleCouleurHauteur[height][2];
}


// ======================= DEFINITION DES FONCTIONS DE CALLBACKS POUR LE RENDU 3D ==================================

//callack clavier : deplacement de la camera dans le monde
void clavierCallback(GLFWwindow* window, int touche, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT )
    {
        switch (touche)
        {
        case GLFW_KEY_LEFT:
            deltaStrafe += 5;
            break;
        case GLFW_KEY_RIGHT:
            deltaStrafe -= 5;
            break;
        case GLFW_KEY_DOWN:
            deltaMove += 5;
            break;
        case GLFW_KEY_UP:
            deltaMove -= 5;
            break;
        case GLFW_KEY_I:
            deltaUp -= 5;
            break;
        case GLFW_KEY_O:
            deltaUp += 5;
            break;
        case GLFW_KEY_F:
            modeRendu = GL_FILL;
            break;
        case GLFW_KEY_L:
            modeRendu = GL_LINE;
            break;
        }
    }
}

//execution du deplacement de la camera
void moveCamera(float dMove, float dStrafe, float dUp)
{
    camPosX += dMove * sin(theta) + dStrafe * cos(theta); //deplacement droite gauche
    camPosZ += dUp;//deplacement haut bas
    camPosY += dStrafe * sin(-theta) + dMove * cos(theta); //deplacement avant arriere
    //Mise a jour de la position
    camAtX = camPosX + forwardViewX;
    camAtY = camPosY + forwardViewY;
    camAtZ = camPosZ + forwardViewZ;
}

//callback souris : pour changer l'orientation de la camera
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {//on presse le bouton gauche, on mémorise la position
            glfwGetCursorPos(window, &mouseX, &mouseY);
        }
        else
        {//on relache le bouton gauche, on actualise les angles, on dit qu'on a pas cliqué
            mouseX = -1;
            mouseY = -1;
            theta += deltaTheta;
            phi += deltaPhi;
        }
    }
}

//fonction pour normaliser un vecteur de composantes x, y z
void normaliser(float& x, float& y, float& z)
{
    float facteur = sqrt(x * x + y * y + z * z);
    x /= facteur;
    y /= facteur;
    z /= facteur;
}

//fonction qui effectue le produit vectoriel de (a,b,c) par (x,y,z) et le met dans (s,t,u)
void produitVectoriel(float& s, float& t, float& u, float& x, float& y, float& z, float& a, float& b, float& c)
{
    s = b * z - c * y;
    t = c * x - a * z;
    u = a * y - b * x;
}

//callback souris pour l'orientation de la camera
void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    // On ne fait quelque chose que si l'utilisateur
    // a deja clique quelque part avec le bouton gauche
    if (mouseX >= 0 || mouseY >= 0)
    {
        // mise a jour des deltas des angles theta et phi
        deltaTheta = -(x - mouseX) * sensibilite;
        deltaPhi = -(y - mouseY) * sensibilite;

        // Calcul du nouveau vecteur vision :
        forwardViewY = -cos(theta + deltaTheta);
        forwardViewX = -sin(theta + deltaTheta);
        forwardViewZ = -cos(phi + deltaPhi);

        normaliser(forwardViewX, forwardViewY, forwardViewZ);

        produitVectoriel(rightX, rightY, rightZ, forwardViewX, forwardViewY, forwardViewZ, upX, upY, upZ);

        camAtX = camPosX + forwardViewX;
        camAtZ = camPosZ + forwardViewZ;
        camAtY = camPosY + forwardViewY;
    }
}

void redimensionner(int w, int h)
{
    // eviter une division par 0
    if (h == 0)
        h = 1;
    float ratio = (float)w / (float)h;
    //std::cout << "Ratio : " << ratio << std::endl;
    // Projection
    glMatrixMode(GL_PROJECTION);

    // Resetting matrix
    glLoadIdentity();

    // Viewport
    glViewport(0, 0, w, h);

    // Retourne a la pile modelview
    glMatrixMode(GL_MODELVIEW);

    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
}


// =========================== FONCTION POUR CREER ET COMPILER UN SHADER ==================================

//fonction pour compiler un shader, on donne dans type, le type de shader voulu
static unsigned int compileShader(unsigned int type, const string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); // equivalent à &source[0]
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    //Error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char)); //car message[length] ne fonctionne pas en cpp
        glGetShaderInfoLog(id, length, &length, message);
        cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader" << endl;
        cout << message << endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}


//Permet de creer le programme a partir du vertex shader et du framgent shader
static unsigned int createShader(const string& vertexShader, const string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);
    glDeleteShader(vs); //on fait ça car les shaders
    glDeleteShader(fs); // ont ete linked a program
    return program;
}

// ========================= VARIABLES POUR L'EDITION DE HEIGHTMAP =================================

int X = 0; //nombre de colonnes
int Y = 0; //nombre de lignes
Scalar couleur(0, 0, 0, 0);
/*mode d'edition
0 dessin
1 flou*/
int mode = 0;
int key = 0;

//Outil pinceau
CircleBrush pinceau(10, 20, 0);
BlurBrush pinceauFlou(10);
//pour initialisation des trackbars
int rayonPinceauInt = 20;
int rayonPinceauExt = 30;

// ============================ CALLBACKS POUR L'EDITION DE HEIGHTMAP ================================

//callback souris pour le tracé 
void SourisCallback(int event, int x, int y, int flags, void* userdata)
{
    if ((event == EVENT_LBUTTONDOWN && mode == 0) || (event == EVENT_MOUSEMOVE && flags == EVENT_FLAG_LBUTTON && mode == 0))
    {
        pinceau.paint(x, y, img);
    }
    if ((event == EVENT_LBUTTONDOWN && mode == 1) || (event == EVENT_MOUSEMOVE && flags == EVENT_FLAG_LBUTTON && mode == 1))
    {
        pinceauFlou.blurPaint(img, x, y);
    }
};

//callback pour le mode d'edition
void trackbarCallbackMode(int value, void* userdata)
{
    mode = value;
}

//Couleur du pinceau en niveau de gris
void trackbarCallbackColor(int value, void* userdata)
{
    couleur = (value, value, value);
    pinceau.setColor(value);
}

//taille du pinceau 
void trackbarCallbackRadiusExt(int value, void* userdata)
{
    rayonPinceauExt = value;
    pinceau.setRadiusExt(value);

}

void trackbarCallbackRadiusInt(int value, void* userdata)
{
    rayonPinceauInt = value;
    pinceau.setRadiusInt(value);
    pinceauFlou.setRadius(value);
}

// ======================================== FONCTION MAIN ================================================

int main()
{
    //on demande le mode a l'utilisateur (affichage 3d ou modification heightmap)
    unsigned int choixUtilisateur = 0;
    do
    {
        cout << "Saisir le mode voulu : affichage de heightmap 0, edition de heightmap 1" << endl;
        cin >> choixUtilisateur;
    } while (choixUtilisateur > 1);

    //-------------------- Partie openCV : charge la heightmap -----------------------
    //Mat img;
    cout << "saisir le chemin du fichier de la heightmap voulue" << endl;
    string image_path;
    cin >> image_path;
    img = imread(image_path, IMREAD_GRAYSCALE);
    camPosX = img.rows;
    camPosY = img.cols;
    X = img.rows;
    Y = img.cols;
    if (img.empty())
    {
        std::cout << "Could not read the image: " << image_path << std::endl;
        return 1;
    }

    // ============================= SI LE MODE AFFICHAGE 3D EST CHOISI ================================
    if (choixUtilisateur == 0)
    {

        //Demande à l'utilisateur la précision voulue
        cout << "Entrer le pas voulu entre les pixels" << endl;
        int precision = 1;
        cin >> precision;

        imshow("Display window", img);
        cout << "taille :" << img.rows << "x" << img.cols << endl;

        //------------------------------ Partie GLFW & OpenGL -----------------------------

        //initialiser la librairie
        GLFWwindow* window;

        if (!glfwInit())
        {
            return -1;
        }

        //On dit quelle version d'OpenGL on veut utiliser
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        //Creation de la fenêtre et verification
        window = glfwCreateWindow(640, 640, "P1RV with modern OpenGL", NULL, NULL);
        if (!window)
        {
            glfwTerminate();
            return -1;
        }

        glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

        //mise en place des Callbacks
        glfwSetKeyCallback(window, clavierCallback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);

        //Creer le context OpenGL (initier les variables d'état)
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // start GLEW extension handler = charge les bonnes fonctions d'OpenGL
        glewExperimental = GL_TRUE;
        glewInit();

        //initialisation vertex, couleurs et indices dans des tableaux
        vector<float> tableauVertex;
        vector<float> tableauColor;
        vector<GLuint> tableauIndices;

        //nombre de vertices par ligne et par colonne
        // ATTENTION une simple division euclidienne ne fonctionne pas
        int nbVertexRow;
        int nbVertexCol;

        if (img.rows % precision == 0)
        {
            nbVertexRow = img.rows / precision;
        }
        else
        {
            nbVertexRow = (img.rows / precision) + 1;
        }

        if (img.cols % precision == 0)
        {
            nbVertexCol = img.cols / precision;
        }
        else
        {
            nbVertexCol = (img.cols / precision) + 1;
        }

        //donne le nombre de vertex à dessiner
        int nbVertex = nbVertexRow * nbVertexCol;

        //Boucle pour parcourir l'image et ainsi recuperer la hauteur en tout point et determiner la couleur du vertex
        for (int i = 0; i < img.rows; i = i + precision)
        {
            for (int j = 0; j < img.cols; j = j + precision)
            {
                //declaration de variables necessaires
                double couleurR, couleurG, couleurB;

                //on recupere la hauteur de l'image chargee par OpenCV
                int height = (int)img.at<uchar>(i, j);

                //on determine la couleur en fonction de la hauteur
                choixCouleurHauteur(couleurR, couleurG, couleurB, height);

                //on ajoute la couleur dans le tableauColor
                tableauColor.push_back(couleurR);
                tableauColor.push_back(couleurG);
                tableauColor.push_back(couleurB);

                //on ajoute le vertex dans le tableauVertex
                tableauVertex.push_back(i);
                tableauVertex.push_back(j);
                tableauVertex.push_back(height);
            }
        }

        //Ajoute les indices des vertex pour chaque triangle à dessiner
        //nbVertexRow : le nombre de vertices par ligne
        //nb VertexCol : le nombre de vertices par colonne
        for (int i = 0; i < nbVertexRow - 1; i++)
        {
            for (int j = 0; j < nbVertexCol - 1; j++)
            {
                tableauIndices.push_back(i * nbVertexCol + j);
                tableauIndices.push_back(i * nbVertexCol + (j + 1));
                tableauIndices.push_back((i + 1) * nbVertexCol + j);

                tableauIndices.push_back((i + 1) * nbVertexCol + j);
                tableauIndices.push_back(i * nbVertexCol + (j + 1));
                tableauIndices.push_back((i + 1) * nbVertexCol + (j + 1));
            }
        }

        //VAO et generer les buffers
        GLuint vao = 0, vertex_vbo = 0, color_vbo = 0, ebo = 0;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vertex_vbo);
        glGenBuffers(1, &color_vbo);
        glGenBuffers(1, &ebo);

        //on bind le vertex array en premier et apres on bind les vertex buffer
        glBindVertexArray(vao);

        //buffer pour les vertex
        glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
        glBufferData(GL_ARRAY_BUFFER, tableauVertex.size() * sizeof(float), &tableauVertex[0], GL_STATIC_DRAW);
        //la ligne en-dessous permet de designer le "layout" sur lequel se trouve les données pour le vertex shader
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        //buffer pour les couleurs
        glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
        glBufferData(GL_ARRAY_BUFFER, tableauColor.size() * sizeof(float), &tableauColor[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);

        //buffer pour les indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, tableauIndices.size() * sizeof(unsigned int), &tableauIndices[0], GL_STATIC_DRAW);


        // -----------------------------  PARTIE SHADERS ------------------------------
        const string vertexShader =
            "#version 400\n"
            "layout(location = 0) in vec3 vertexPosition_modelspace;"
            "layout(location = 1) in vec3 vertex_colour;"
            "uniform mat4 MVP;"
            "out vec3 colour;"
            "void main() {"
            "colour = vertex_colour;"
            "gl_Position =  MVP * vec4(vertexPosition_modelspace,1);"
            "}";

        const string fragmentShader =
            "#version 400\n"
            "in vec3 colour;"
            "out vec4 frag_colour;"
            "void main() {"
            "frag_colour = vec4(colour, 1.0);"
            "}";

        //Creer et utilise le shader
        unsigned int shader = createShader(vertexShader, fragmentShader);
        glUseProgram(shader);

        // Get a handle for our "MVP" uniform
        // Only during the initialisation
        GLuint MatrixID = glGetUniformLocation(shader, "MVP");

        // Z-Buffer
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // ------------------- Boucle principale d'affichage -------------------------

        while (!glfwWindowShouldClose(window))
        {
            glPolygonMode(GL_FRONT_AND_BACK, modeRendu);
            glClearColor(0.05f, 0.4f, 0.6f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glfwGetFramebufferSize(window, &windowW, &windowH);
            redimensionner(windowW, windowH);

            //Deplacement de la camera
            moveCamera(deltaMove, deltaStrafe, deltaUp);
            //remise a zero des variables
            deltaMove = 0;
            deltaStrafe = 0;
            deltaUp = 0;

            //Recalcule des matrices
            View = glm::lookAt(
                glm::vec3(camPosX, camPosY, camPosZ),
                glm::vec3(camAtX, camAtY, camAtZ),
                glm::vec3(upX, upY, upZ));

            Projection = glm::perspective(glm::radians(45.0f), (float)windowW / (float)windowH, 0.1f, 2000.0f);

            //recalcul de la MVP
            mvp = Projection * View * Model;

            //envoie de la MVP au shader, dans le "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

            //On dessine enfin 
            glBindVertexArray(vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); //ligne facultative ?
            glDrawElements(GL_TRIANGLES, tableauIndices.size(), GL_UNSIGNED_INT, 0);

            //swap buffers
            glfwSwapBuffers(window);

            //pool for and process event
            glfwPollEvents();
        }

        glfwTerminate();
    }

    // =============================== SI LE MODE EDITION EST CHOISI =======================================
    else if (choixUtilisateur == 1)
    {
        namedWindow("Edition de la heightmap");

        //callback souris pour le tracé
        setMouseCallback("Edition de la heightmap", SourisCallback);

        //Nouvelle fenêtre pour ne pas encombrer avec les trackbars
        namedWindow("Tools");
        moveWindow("Tools", 1000, 100);
        resizeWindow("Tools", 600, 200);

        //callabck pour le mode d'edition
        /*pour remplacer les boutons, on utilise une trackar avec seulement 2 positions : dessin ou flou*/
        createTrackbar("mode d'edition: dessin --- flou", "Tools", &mode, 1, trackbarCallbackMode);

        //callback pour la couleur du tracé
        createTrackbar("color", "Tools", NULL, 255, trackbarCallbackColor);
        //callback pour la taille du pinceau
        createTrackbar("radiusInt", "Tools", &rayonPinceauInt, X / 8, trackbarCallbackRadiusInt);
        createTrackbar("radiusExt", "Tools", &rayonPinceauExt, X / 8, trackbarCallbackRadiusExt);

        cv::resize(img, img, cv::Size(), 0.65, 0.65);
        while (waitKey(20) != 27)  // wait until ESC is pressed
        {
            imshow("Edition de la heightmap", img);   //force le réaffichage
        }
        cout << "Voulez-vous sauegarder l'image modifiée ? OUI : o ; NON : n" << endl;
        key = waitKey(-1);
        if (key == 'o')
        {
            cout << "Rentrez le nom de l'image" << endl;
            string nomImageModifiee;
            cin >> nomImageModifiee;
            imwrite(nomImageModifiee + ".png", img);
        }
    }

    return 0;
}