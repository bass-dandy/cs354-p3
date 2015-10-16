#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <string>

#include "viewer.h"

// Mouse control
GLfloat clickCoords[2];
bool isLeftClick   = false;
bool isRightClick  = false;
bool isMiddleClick = false;

// Model viewer
ModelViewer *viewer;


void reshape(int w, int h) {
    viewer->setViewport(w, h);
    glutPostRedisplay();
}


void MouseButton(int button, int state, int x, int y) {
    if(state == GLUT_DOWN) {
        clickCoords[0] = x;
        clickCoords[1] = y;
        isLeftClick   = (button == GLUT_LEFT_BUTTON)   ? true : false;
        isRightClick  = (button == GLUT_RIGHT_BUTTON)  ? true : false;
        isMiddleClick = (button == GLUT_MIDDLE_BUTTON) ? true : false;
    } else {
        isLeftClick   = false;
        isRightClick  = false;
        isMiddleClick = false;
    }
}


void MouseMotion(int x, int y) {
    if(isLeftClick && !isRightClick && !isMiddleClick) {
        viewer->orbit(clickCoords[0] - x, clickCoords[1] - y);
        glutPostRedisplay();
    }
    else if(isRightClick && !isLeftClick && !isMiddleClick) {
        viewer->zoom(y - clickCoords[1]);
        glutPostRedisplay();
    }
    else if(isMiddleClick && !isRightClick && !isLeftClick) {
        viewer->pan(clickCoords[0] - x, y - clickCoords[1]);
        glutPostRedisplay();
    }
}


void parseCommand(char *ln) {
    char *ctok = strtok(ln, " ");

    if(ctok) {
        std::string tok(ctok);

        if(tok == "L") {
            std::string arg(strtok(NULL, " "));
            if(arg.length() >= 4) {
                if(arg.compare(arg.length() - 4, 4, ".obj")) {
                    arg += ".obj";
                }
            }
            std::cout << "Loading file " << arg << "..." << std::endl;
            viewer->loadModel(arg);
        } 
        else if(tok == "D") {
            std::cout << "Deleting current object..." << std::endl;  
            viewer->deleteModel();
        } 
        else if(tok == "I") {
            std::cout << "Loading identity matrix..." << std::endl;   
            viewer->loadIdentity();
        } 
        else if(tok == "T" || tok == "S" || tok == "R") {
            float x = std::stof(std::string(strtok(NULL, " ")));
            float y = std::stof(std::string(strtok(NULL, " ")));
            float z = std::stof(std::string(strtok(NULL, " ")));

            if(tok == "T") {
                std::cout << "Translating model..." << std::endl;  
                viewer->translate(x, y, z);
            }
            else if(tok == "S") {
                std::cout << "Scaling model..." << std::endl;  
                viewer->scale(x, y, z);
            }
            else {
                std::cout << "Rotating model counterclockwise..." << std::endl;
                float t = std::stof(std::string(strtok(NULL, " ")));
                viewer->rotate(x, y, z, t);
            }
        } 
        else if(tok == "V") {
            std::cout << "Using camera coordinates..." << std::endl;
            viewer->useCameraCoordinates(true);
        } 
        else if(tok == "W") {
            std::cout << "Using world coordinates..." << std::endl;
            viewer->useCameraCoordinates(false);
        } 
        else if(tok == "N") {
            std::string arg(strtok(NULL, " "));
            if(arg == "face") {
                bool show = viewer->toggleFaceNormals();
                std::cout << "Show face normals: " << (show ? "ON" : "OFF") << std::endl;
            }
            else if(arg == "vertex") {
                bool show = viewer->toggleVertexNormals();
                std::cout << "Show vertex normals: " << (show ? "ON" : "OFF") << std::endl;
            }
        }
        else if(tok == "M") {
            std::string arg(strtok(NULL, " "));
            if(arg == "wire") {
                viewer->setMode(MODE_WIRE);
            } 
            else if(arg == "solid") {
                viewer->setMode(MODE_SOLID);  
            } 
            else if(arg == "lit") {
                viewer->setMode(MODE_LIT);   
            } 
            else if(arg == "point") {
                viewer->setMode(MODE_POINT);
            } 
        }
        else {
            std::cout << "Command " << tok << " is not valid." << std::endl;
        }
        glutPostRedisplay();
    }
}


void keyboard(unsigned char key, int x, int y) {
    if(key == 'c') {
        std::cout << "Enter command: ";
        char ln[256];
        std::cin.getline(ln, 256);
        parseCommand(ln);
    }
}


void display() {
    viewer->display();
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(700, 700);
    glutCreateWindow("Model Viewer - ctd487");

    // Set callbacks
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutKeyboardFunc(keyboard);

    glEnable(GL_DEPTH_TEST);
    glClearColor (0.1, 0.1, 0.1, 1.0);

    viewer = new ModelViewer();

    // Load initialization file
    if(argc == 3) {
        using namespace std;
        string flag(argv[1]);
        string arg(argv[2]);

        if(flag == "-f") {
            cout << "Running commands from file " << arg << "..." << endl;
            ifstream ifs(arg);

            while(ifs) {
                char ln[256];
                ifs.getline(ln, 256);
                parseCommand(ln);
            }
        }

    }
    glutMainLoop();
    delete viewer;
    return 0;
}
