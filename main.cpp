#include <stdlib.h>
#include <GL/glut.h>
#include <stack>
#include <cmath>
#include <iostream>
#include <string>
#include "loader.h"

class Camera {

    private:

        Point pos;
        Point look;
        Point up;
        float theta;
        float phi;

    public:
        
        float dist;

        Camera() : up(Point(0.0f, 1.0f, 0.0f)), dist(0.0f), theta(0.0f), phi(0.0f) {}

        void update() {
            pos.x = look.x + dist * cos(phi) * sin(theta);
            pos.y = look.y + dist * sin(phi) * sin(theta);
            pos.z = look.z + dist * cos(theta);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            gluLookAt(pos.x, pos.y, pos.z, look.x, look.y, look.z, up.x, up.y, up.z);
        }

        void zoom(float delta) {
            dist += delta;
            if(dist < 1.0f)
                dist = 1.0f;
        }

        void orbit(float deltaX, float deltaY) {
            theta += deltaX * 0.001f;
            phi   += deltaY * 0.001f;
        }

        void pan(float deltaX, float deltaY) {
            GLfloat view[16];
            glGetFloatv(GL_MODELVIEW_MATRIX, view);
            
            Point right(view[0], view[4], view[8]);
            Point    up(view[1], view[5], view[9]);
            
            right = right.normalize() * deltaX * 0.001f;
            up    = up.normalize()    * deltaY * 0.001f;

            look += right += up;
        }

        void lookAt(Point p) {
            look = p;
        }
};


// Mouse control
GLfloat clickCoords[2];
bool isLeftClick   = false;
bool isRightClick  = false;
bool isMiddleClick = false;

// Display variables
int mode = MODE_LIT;
bool doFaceNormals   = false;
bool doVertexNormals = false;
bool useCameraCoordinates = false;
int viewportWidth;
int viewportHeight;

// Model stack
std::stack<Trimesh> models;
Camera camera;


void reshape(int w, int h) {
    viewportWidth  = w;
    viewportHeight = h;
    glutPostRedisplay();
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!models.empty()) {
        float zNear = 0.01f;
        float zFar  = zNear + camera.dist + models.top().getMaxDelta();

        glViewport(0, 0, viewportWidth, viewportHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, viewportWidth/viewportHeight, zNear, zFar);

        camera.update();
            
        models.top().draw(mode, doVertexNormals, doFaceNormals);
    }
    glFlush();
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
        camera.orbit(clickCoords[0] - x, clickCoords[1] - y);
        glutPostRedisplay();
    }
    else if(isRightClick && !isLeftClick && !isMiddleClick) {
        camera.zoom(y - clickCoords[1]);
        glutPostRedisplay();
    }
    else if(isMiddleClick && !isRightClick && !isLeftClick) {
        camera.pan(clickCoords[0] - x, y - clickCoords[1]);
        glutPostRedisplay();
    }
}


void loadModel(std::string filename) {
    Trimesh *tm = new Trimesh();
    TrimeshLoader ldr;
    ldr.loadOBJ(filename.c_str(), tm);
    models.push(*tm);

    camera.lookAt(tm->getOrigin());
    camera.dist = 0.01f + tm->getMaxDelta() * 1.5f;
}


void parseCommands(std::istream &is) {
    char ln[256];
    is.getline(ln, 256);
    char *ctok = strtok(ln, " ");

    if(ctok) {
        std::string tok(ctok);

        if(tok == "L") {
            std::string arg(strtok(NULL, " "));
            std::cout << "Loading file " << arg << "..." << std::endl;
            loadModel(arg);
        } else if(tok == "D") {
            std::cout << "Deleting current object..." << std::endl;  
            models.pop();
        } else if(tok == "I") {
            std::cout << "Loading identity matrix..." << std::endl;   
            models.top().identity();
        } else if(tok == "T") {
            std::cout << "Translating model..." << std::endl;  
            float x = std::stof(std::string(strtok(NULL, " ")));
            float y = std::stof(std::string(strtok(NULL, " ")));
            float z = std::stof(std::string(strtok(NULL, " ")));

            if(useCameraCoordinates) {
                GLfloat view[16];
                glGetFloatv(GL_MODELVIEW_MATRIX, view);
                
                Point right(view[0], view[4], view[8]);
                Point    up(view[1], view[5], view[9]);
                Point  back(view[2], view[6], view[10]);
                
                right = right.normalize() * x;
                up    = up.normalize()    * y;
                back  = back.normalize()  * z;

                x = right.x + up.x + back.x;
                y = right.y + up.y + back.y;
                z = right.z + up.z + back.z;
            }
            models.top().translate(x, y, z);
        } else if(tok == "S") {
            std::cout << "Scaling model..." << std::endl;  
            float x = std::stof(std::string(strtok(NULL, " ")));
            float y = std::stof(std::string(strtok(NULL, " ")));
            float z = std::stof(std::string(strtok(NULL, " ")));

            if(useCameraCoordinates) {
                GLfloat view[16];
                glGetFloatv(GL_MODELVIEW_MATRIX, view);
                
                Point right(view[0], view[4], view[8]);
                Point    up(view[1], view[5], view[9]);
                Point  back(view[2], view[6], view[10]);
                
                right = right.normalize() * x;
                up    = up.normalize()    * y;
                back  = back.normalize()  * z;

                x = right.x + up.x + back.x;
                y = right.y + up.y + back.y;
                z = right.z + up.z + back.z;
            }
            models.top().scale(x, y, z);
        } else if(tok == "R") {
            std::cout << "Rotating model counterclockwise..." << std::endl;
            float t = std::stof(std::string(strtok(NULL, " ")));
            float x = std::stof(std::string(strtok(NULL, " ")));
            float y = std::stof(std::string(strtok(NULL, " ")));
            float z = std::stof(std::string(strtok(NULL, " ")));
            
            if(useCameraCoordinates) {

            }
            models.top().rotate(t, x, y, z);
        } else if(tok == "V") {
            std::cout << "Using camera coordinates..." << std::endl;
            useCameraCoordinates = true;
        } else if(tok == "W") {
            std::cout << "Using world coordinates..." << std::endl;
            useCameraCoordinates = false;
        } else if(tok == "wire") {
            mode = MODE_WIRE;
        } else if(tok == "solid") {
            mode = MODE_SOLID;  
        } else if(tok == "lit") {
            mode = MODE_LIT;   
        } else if(tok == "point") {
            mode = MODE_POINT;
        } else {
            std::cout << "Command " << tok << " is not valid." << std::endl;
        }
        glutPostRedisplay();
    }
}


void keyboard(unsigned char key, int x, int y) {
    if(key == 'c') {
        std::cout << "Enter command: ";
        parseCommands(std::cin);
    }
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

    // TODO delete
    loadModel(std::string("models/cessna.obj"));

    glutMainLoop();
    return 0;
}
