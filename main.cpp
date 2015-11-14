#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <string>

#include "viewer.h"
#include "src/include/GL/glui.h"

// Mouse control
GLfloat clickCoords[2];
bool isLeftClick   = false;
bool isRightClick  = false;
bool isMiddleClick = false;

// Model viewer
ModelViewer *viewer;

// GLUI components
int main_window;
GLUI *glui, *glui2;

// GLUI live variables
char filename[128];
int renderMode = MODE_LIT;
int showFaceNormals = 0;
int showVertNormals = 0;
Point translation;
Point scaling;
float *rotation;

void reshape(int w, int h) {
    viewer->setViewport(w, h);
    GLUI_Master.auto_set_viewport();
}


void mouseButton(int button, int state, int x, int y) {
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


void mouseMotion(int x, int y) {
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


void loadObject(int id) {
    viewer->loadModel(filename);
}


void deleteModel(int id) {
    viewer->deleteModel();
}


void translate(int id) {
    viewer->setTranslation(translation);
}


void scale(int id) {
    viewer->setScale(scaling);
}

void identity(int id) {
    viewer->loadIdentity();
}


void idle() {
    glutSetWindow(main_window);
    glutPostRedisplay();
}


void display() {
    viewer->setMode(renderMode);   
    viewer->showFaceNormals(showFaceNormals);
    viewer->showVertexNormals(showVertNormals);
    viewer->display();
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 700);
    main_window = glutCreateWindow("Scene Graph - ctd487");

    // Set callbacks
    glutDisplayFunc( display );
    GLUI_Master.set_glutReshapeFunc( reshape );  
    GLUI_Master.set_glutSpecialFunc( NULL );
    GLUI_Master.set_glutMouseFunc( mouseButton );
    glutMotionFunc( mouseMotion );

    glEnable(GL_DEPTH_TEST);
    glClearColor (0.1, 0.1, 0.1, 1.0);

    viewer = new ModelViewer();
    viewer->loadModel("models/cessna.obj");
    translation = viewer->getTranslation();
    scaling = viewer->getScale();
    rotation = viewer->getRotation();

    // Setup right subwindow
    glui = GLUI_Master.create_glui_subwindow( main_window, GLUI_SUBWINDOW_RIGHT );
    
    // Render options panel
    GLUI_Panel *renderOptions = new GLUI_Panel( glui, "Render Options" );
    
    // Render mode dropdown menu
    GLUI_Listbox *list = new GLUI_Listbox(renderOptions, "Render Mode", &renderMode);
    list->add_item(MODE_LIT, "Lit");
    list->add_item(MODE_SOLID, "Solid");
    list->add_item(MODE_WIRE, "Wireframe");
    list->add_item(MODE_POINT, "Point");
  
    new GLUI_StaticText( renderOptions, "" );
    
    // Checkboxes to toggle normals
    new GLUI_Checkbox( renderOptions, "Draw Face Normals", &showFaceNormals );
    new GLUI_Checkbox( renderOptions, "Draw Vertex Normals", &showVertNormals );

    // Text box to enter a .obj filename
    glui->add_edittext( "Filename:", GLUI_EDITTEXT_TEXT, &filename  );
    glui->add_button( "Load .obj File", 0, loadObject );
    glui->add_button( "Delete Model", 0, deleteModel );

    // Setup bottom subwindow
    glui2 = GLUI_Master.create_glui_subwindow( main_window, GLUI_SUBWINDOW_BOTTOM );

    // Rotation widget
    GLUI_Rotation *view_rot = new GLUI_Rotation(glui2, "Rotate", rotation );
    view_rot->set_spin( 1.0 );
    
    // Translate X widget
    new GLUI_Column( glui2, true );
    GLUI_Translation *trans_x = new GLUI_Translation(glui2, "Translate X", GLUI_TRANSLATION_X, &translation.x, 0, translate);
    trans_x->set_speed( .005 );
    
    // Translate Y widget
    new GLUI_Column( glui2, false );
    GLUI_Translation *trans_y = new GLUI_Translation(glui2, "Translate Y", GLUI_TRANSLATION_Y, &translation.y, 0, translate);
    trans_y->set_speed( .005 );
    
    // Translate Z widget
    new GLUI_Column( glui2, false );
    GLUI_Translation *trans_z = new GLUI_Translation(glui2, "Translate Z", GLUI_TRANSLATION_Z, &translation.z, 0, translate);
    trans_z->set_speed( .005 );

    // Scaling controls
    new GLUI_Column( glui2, true );
    new GLUI_Spinner( glui2, "Scale X", &scaling.x, 0, scale );
    new GLUI_Spinner( glui2, "Scale Y", &scaling.y, 0, scale );
    new GLUI_Spinner( glui2, "Scale Z", &scaling.z, 0, scale );

    // Reset
    new GLUI_Column( glui2, true );
    new GLUI_Button( glui2, "Reset", 0, identity);

    glui->set_main_gfx_window( main_window );
    glui2->set_main_gfx_window( main_window );
    GLUI_Master.set_glutIdleFunc( idle );

    glutMainLoop();
    delete viewer;
    return 0;
}
