#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <string>

#include "scenegraph.h"
#include "src/include/GL/glui.h"

enum {
    ID_TRANSLATE,
    ID_SCALE,
    ID_ROTATE,
    ID_IDENTITY,
    ID_SELECT_CHILD,
    ID_SELECT_PARENT
};

SceneGraph *sg;

// GLUI components
int main_window;
GLUI *glui, *glui2;
GLUI_Listbox *childList;
GLUI_StaticText *selectedNodeName;

// GLUI live variables
char filename[128];
int renderMode = MODE_LIT;
int showFaceNormals = 0;
int showVertNormals = 0;

int childIdx;
int childCnt;

int nodeType;

Point translation;
Point scaling;
float rotation[16];


void reshape(int w, int h) {
    //viewer->setViewport(w, h);
    GLUI_Master.auto_set_viewport();
}

void idle() {
    glutSetWindow(main_window);
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1, 0.1, 200);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);

    sg->display();

    glFlush();
}

void readLiveVars(SGNode *n) {
    if(n->getNodeType() == NODE_TRANSFORM) {
        TransformNode *t = static_cast<TransformNode*>(n);

        translation.x = t->translation.x;
        translation.y = t->translation.y;
        translation.z = t->translation.z;
        
        scaling.x = t->scaling.x;
        scaling.y = t->scaling.y;
        scaling.z = t->scaling.z;
        
        for(int i = 0; i < 16; ++i) {
            rotation[i] = t->rotation[i];
        }
    }
    if(n->getNodeType() == NODE_TRANSFORM || n->getNodeType() == NODE_OBJECT) {
        ParentNode *p = static_cast<ParentNode*>(n);

        for(int i = 0; i < childCnt; ++i) {
            childList->delete_item(i);
        }
        childCnt = p->children.size();
        if(childCnt == 0) {
            childList->add_item(0, "");
            childCnt = 1;
        } else {
            for(int i = 0; i < childCnt; ++i) {
                childList->add_item(i, p->children[i]->getName().c_str());
            }
        }
    }
    selectedNodeName->set_text(std::string("Selected node: " + n->getName()).c_str());
    GLUI_Master.sync_live_all();
}

void transform_cb(int id) {
    TransformNode *t = static_cast<TransformNode*>(sg->getCurrent());
    switch(id) {
        case ID_TRANSLATE:
            t->translation.x = translation.x;
            t->translation.y = translation.y;
            t->translation.z = translation.z;
            break;
        case ID_SCALE:
            t->scaling.x = scaling.x;
            t->scaling.y = scaling.y;
            t->scaling.z = scaling.z;
            break;
        case ID_ROTATE:
            for(int i = 0; i < 16; ++i) {
                t->rotation[i] = rotation[i];
            }
            break;
    }
}

void traverse_cb(int id) {
    SGNode *n;
    switch(id) {
        case ID_SELECT_CHILD:
            n = sg->selectChild(childIdx);
            break;
        case ID_SELECT_PARENT:
            n = sg->selectParent();
            break;
    }
    if(n->getNodeType() == NODE_TRANSFORM) {
        // TODO: enable transform controls
        readLiveVars(n);
    }
    else {
        // TODO: disable transform controls
    }
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

    glEnable(GL_DEPTH_TEST);
    glClearColor (0.1, 0.1, 0.1, 1.0);

    // Setup right subwindow
    glui = GLUI_Master.create_glui_subwindow( main_window, GLUI_SUBWINDOW_RIGHT );
    
    // Render options panel
    GLUI_Panel *renderOptions = new GLUI_Panel( glui, "Render Options" );
    
    // Render mode dropdown menu
    GLUI_Listbox *list = new GLUI_Listbox(renderOptions, "Render Mode: ", &renderMode);
    list->add_item(MODE_LIT, "Lit");
    list->add_item(MODE_SOLID, "Solid");
    list->add_item(MODE_WIRE, "Wireframe");
    list->add_item(MODE_POINT, "Point");
  
    new GLUI_StaticText( renderOptions, "" );
    
    // Checkboxes to toggle normals
    new GLUI_Checkbox( renderOptions, "Draw Face Normals", &showFaceNormals );
    new GLUI_Checkbox( renderOptions, "Draw Vertex Normals", &showVertNormals );

    // Text box to enter a .obj filename
    glui->add_edittext( "Filename: ", GLUI_EDITTEXT_TEXT, &filename  );
    //glui->add_button( "Load .obj File", 0, loadObject );
    //glui->add_button( "Delete Model", 0, deleteModel );
    
    // Traversal controls
    GLUI_Panel *sgOptions = new GLUI_Panel( glui, "Scene Graph" );
    selectedNodeName = new GLUI_StaticText(sgOptions, "Selected node: ");

    GLUI_Panel *traverseOptions = new GLUI_Panel( sgOptions, "Traversal" );

    childList = new GLUI_Listbox(traverseOptions, "Child: ", &childIdx);
    new GLUI_Column( traverseOptions, false );
    new GLUI_Button( traverseOptions, "Select Child", ID_SELECT_CHILD, traverse_cb );
    new GLUI_Button( traverseOptions, "Select Parent", ID_SELECT_PARENT, traverse_cb );

    GLUI_Panel *adOptions = new GLUI_Panel( sgOptions, "Add/Delete" );
    GLUI_Listbox *nodeTypeList = new GLUI_Listbox(adOptions, "Node Type: ", &nodeType);
    nodeTypeList->add_item(NODE_OBJECT, "Object");
    nodeTypeList->add_item(NODE_TRANSFORM, "Transform");
    nodeTypeList->add_item(NODE_GEOM, "Geometry");
    nodeTypeList->add_item(NODE_ATTR, "Attribute");
    nodeTypeList->add_item(NODE_LIGHT, "Light");
    nodeTypeList->add_item(NODE_CAMERA, "Camera");

    // Setup bottom subwindow
    glui2 = GLUI_Master.create_glui_subwindow( main_window, GLUI_SUBWINDOW_BOTTOM );

    // Rotation widget
    GLUI_Rotation *view_rot = new GLUI_Rotation(glui2, "Rotate", rotation, ID_ROTATE, transform_cb);
    view_rot->set_spin( 1.0 );
    
    // Translate X widget
    new GLUI_Column( glui2, true );
    GLUI_Translation *trans_x = new GLUI_Translation(glui2, 
                                                     "Translate X", 
                                                     GLUI_TRANSLATION_X, 
                                                     &translation.x, 
                                                     ID_TRANSLATE,
                                                     transform_cb);
    trans_x->set_speed( .005 );
    
    // Translate Y widget
    new GLUI_Column( glui2, false );
    GLUI_Translation *trans_y = new GLUI_Translation(glui2, 
                                                     "Translate Y", 
                                                     GLUI_TRANSLATION_Y, 
                                                     &translation.y, 
                                                     ID_TRANSLATE, 
                                                     transform_cb);
    trans_y->set_speed( .005 );
    
    // Translate Z widget
    new GLUI_Column( glui2, false );
    GLUI_Translation *trans_z = new GLUI_Translation(glui2, 
                                                     "Translate Z", 
                                                     GLUI_TRANSLATION_Z, 
                                                     &translation.z, 
                                                     ID_TRANSLATE, 
                                                     transform_cb);
    trans_z->set_speed( .005 );

    // Scaling controls
    new GLUI_Column(glui2, true);
    new GLUI_Spinner(glui2, "Scale X", &scaling.x, ID_SCALE, transform_cb);
    new GLUI_Spinner(glui2, "Scale Y", &scaling.y, ID_SCALE, transform_cb);
    new GLUI_Spinner(glui2, "Scale Z", &scaling.z, ID_SCALE, transform_cb);

    // Reset transform
    new GLUI_Column(glui2, true);
    new GLUI_Button( glui2, "Reset", ID_IDENTITY, transform_cb);

    glui->set_main_gfx_window( main_window );
    glui2->set_main_gfx_window( main_window );
    GLUI_Master.set_glutIdleFunc( idle );

    // Setup scene graph and live vars
    sg = new SceneGraph();
    readLiveVars(sg->getCurrent());

    glutMainLoop();
    delete sg;
    return 0;
}
