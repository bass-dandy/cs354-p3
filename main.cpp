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
    ID_SELECT_PARENT,
    ID_ADD_CHILD,
    ID_DELETE_CHILD
};

SceneGraph *sg;

// GLUI components
int main_window;
GLUI *glui, *panel_transform;
GLUI_Panel *panel_geom, *panel_attr, *panel_camera;
GLUI_Listbox *childList;
GLUI_StaticText *selectedNodeName;

// GLUI live variables
char filename[128];
int renderMode = MODE_LIT;
int showFaceNormals = 0;
int showVertNormals = 0;

int attr_renderMode = MODE_LIT;
int attr_showFaceNormals = 0;
int attr_showVertNormals = 0;

float fv_zNear;
float fv_zFar;
float fv_fov;

int childIdx;
int childCnt;

int lv_createNodeType = NODE_OBJECT;

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
    sg->display();
    glFlush();
}

void readLiveVars(SGNode *n) {
    panel_transform->disable();
    panel_camera->disable();
    panel_geom->disable();
    panel_attr->disable();

    // If node is transform node, update transform panel
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
        panel_transform->enable();
    }
    // If node is an Object node, show proper panels
    else if(n->getNodeType() == NODE_OBJECT) {
        ObjectNode *o = static_cast<ObjectNode*>(n);
        if(o->geom != NULL) {
            panel_geom->enable();
        }
        if(o->attr != NULL) {
            panel_attr->enable();
            attr_showFaceNormals = o->attr->drawFaceNormals;
            attr_showVertNormals = o->attr->drawVertNormals;
            attr_renderMode = o->attr->renderMode;
        }
    }
    else if(n->getNodeType() == NODE_CAMERA) {
        panel_camera->enable();
        CameraNode *c = static_cast<CameraNode*>(n);
        fv_zNear = c->zNear;
        fv_zFar  = c->zFar;
        fv_fov   = c->fov;
    }
    // If node has children, update child list
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
    selectedNodeName->set_text(std::string("Name: " + n->getName()).c_str());
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
        case ID_IDENTITY:
            t->reset();
            readLiveVars(t);
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
    readLiveVars(n);
}

void crud_cb(int id) {
    switch(id) {
        case ID_ADD_CHILD:
            sg->addChild(lv_createNodeType);
            break;
        case ID_DELETE_CHILD:
            sg->deleteChild(childIdx);
            break;
    }
    readLiveVars(sg->getCurrent());
}

void node_cb(int id) {
    ObjectNode *o;
    CameraNode *c;
    switch(id) {
        case NODE_GEOM:
            o = static_cast<ObjectNode*>(sg->getCurrent());
            o->geom->loadModel(std::string(filename));
            break;
        case NODE_ATTR:
            o = static_cast<ObjectNode*>(sg->getCurrent());
            o->attr->renderMode = attr_renderMode;
            o->attr->drawFaceNormals = attr_showFaceNormals;
            o->attr->drawVertNormals = attr_showVertNormals;
            break;
        case NODE_CAMERA:
            c = static_cast<CameraNode*>(sg->getCurrent());
            c->zNear = fv_zNear;
            c->zFar  = fv_zFar;
            c->fov   = fv_fov;
            break;
    }
}

void object_cb(int id) {
    ObjectNode *o = static_cast<ObjectNode*>(sg->getCurrent());
    switch(id) {
        case 0:
            delete o->geom;
            o->geom = NULL;
            break;
        case 1:
            delete o->attr;
            o->attr = NULL;
            break;
    }
    readLiveVars(sg->getCurrent());
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
    GLUI_Master.set_glutIdleFunc( idle );

    glEnable(GL_DEPTH_TEST);
    glClearColor (0.1, 0.1, 0.1, 1.0);

    // Setup right subwindow
    glui = GLUI_Master.create_glui_subwindow( main_window, GLUI_SUBWINDOW_RIGHT );
    glui->set_main_gfx_window( main_window );

    /*************************************************************************/
    /* SG Options Panel ******************************************************/
    /*************************************************************************/

    GLUI_Panel *panel_current = new GLUI_Panel( glui, "Selected Node" );

    selectedNodeName = new GLUI_StaticText(panel_current, "Name: ");
    new GLUI_Column( panel_current, false );
    new GLUI_Button( panel_current, "Select Parent", ID_SELECT_PARENT, traverse_cb );

    GLUI_Panel *childOptions = new GLUI_Panel( glui, "Children" );

    childList = new GLUI_Listbox(childOptions, "Child: ", &childIdx);

    new GLUI_Column( childOptions, false );
    new GLUI_Button( childOptions, "Select Child",  ID_SELECT_CHILD, traverse_cb );
    new GLUI_Button( childOptions, "Delete Child",  ID_DELETE_CHILD, crud_cb);

    /*************************************************************************/
    /* Node Addition Panel ***************************************************/
    /*************************************************************************/

    GLUI_Panel *addOptions = new GLUI_Panel( glui, "" );

    GLUI_Listbox *nodeTypeList = new GLUI_Listbox( addOptions, "Type: ", &lv_createNodeType );
    nodeTypeList->add_item(NODE_OBJECT,    "Object");
    nodeTypeList->add_item(NODE_TRANSFORM, "Transform");
    nodeTypeList->add_item(NODE_GEOM,      "Geometry");
    nodeTypeList->add_item(NODE_ATTR,      "Attribute");
    nodeTypeList->add_item(NODE_LIGHT,     "Light");

    new GLUI_Column( addOptions, false );
    new GLUI_Button( addOptions, "Add Child", ID_ADD_CHILD, crud_cb );

    new GLUI_StaticText( glui, "" );

    /*************************************************************************/
    /* Node Options Panels ***************************************************/
    /*************************************************************************/

    panel_geom   = new GLUI_Panel( glui, "Geometry Node" );
    panel_attr   = new GLUI_Panel( glui, "Attribute Node" );
    panel_camera = new GLUI_Panel( glui, "Camera Node" );

    // Geometry node options
    glui->add_edittext_to_panel( panel_geom, "Path: ", GLUI_EDITTEXT_TEXT, &filename );
    new GLUI_Column( panel_geom, false );
    new GLUI_Button( panel_geom, "Load File", NODE_GEOM, node_cb );
    new GLUI_Button( panel_geom, "Delete Node", 0, object_cb );

    // Attribute node options
    GLUI_Listbox *attr_list = new GLUI_Listbox(panel_attr, "Render Mode: ", &attr_renderMode, NODE_ATTR, node_cb);
    attr_list->add_item(MODE_LIT, "Lit");
    attr_list->add_item(MODE_SOLID, "Solid");
    attr_list->add_item(MODE_WIRE, "Wireframe");
    attr_list->add_item(MODE_POINT, "Point");
  
    new GLUI_StaticText( panel_attr, "" );
    
    // Checkboxes to toggle normals
    new GLUI_Checkbox( panel_attr, "Draw Face Normals",   &attr_showFaceNormals, NODE_ATTR, node_cb );
    new GLUI_Checkbox( panel_attr, "Draw Vertex Normals", &attr_showVertNormals, NODE_ATTR, node_cb );
    new GLUI_StaticText( panel_attr, "" );
    new GLUI_Button( panel_attr, "Delete Node", 1, object_cb );

    // Camera node options
    new GLUI_Spinner(panel_camera, "Near Clip: ", &fv_zNear, NODE_CAMERA, node_cb);
    new GLUI_Spinner(panel_camera, "Far Clip: ",  &fv_zFar,  NODE_CAMERA, node_cb);
    new GLUI_Spinner(panel_camera, "FOV: ",       &fv_fov,   NODE_CAMERA, node_cb);

    /*************************************************************************/
    /* Transform Node Panel **************************************************/
    /*************************************************************************/

    // Create bottom subwindow
    panel_transform = GLUI_Master.create_glui_subwindow( main_window, GLUI_SUBWINDOW_BOTTOM );
    panel_transform->set_main_gfx_window( main_window );

    // Rotation widget
    GLUI_Rotation *view_rot = new GLUI_Rotation(panel_transform, "Rotate", rotation, ID_ROTATE, transform_cb);
    view_rot->set_spin( 1.0 );
    
    // Translate X widget
    new GLUI_Column( panel_transform, true );
    GLUI_Translation *trans_x = new GLUI_Translation(panel_transform, 
                                                     "Translate X", 
                                                     GLUI_TRANSLATION_X, 
                                                     &translation.x, 
                                                     ID_TRANSLATE,
                                                     transform_cb);
    trans_x->set_speed( .005 );
    
    // Translate Y widget
    new GLUI_Column( panel_transform, false );
    GLUI_Translation *trans_y = new GLUI_Translation(panel_transform, 
                                                     "Translate Y", 
                                                     GLUI_TRANSLATION_Y, 
                                                     &translation.y, 
                                                     ID_TRANSLATE, 
                                                     transform_cb);
    trans_y->set_speed( .005 );
    
    // Translate Z widget
    new GLUI_Column( panel_transform, false );
    GLUI_Translation *trans_z = new GLUI_Translation(panel_transform, 
                                                     "Translate Z", 
                                                     GLUI_TRANSLATION_Z, 
                                                     &translation.z, 
                                                     ID_TRANSLATE, 
                                                     transform_cb);
    trans_z->set_speed( .005 );

    // Scaling controls
    new GLUI_Column(panel_transform, true);
    new GLUI_Spinner(panel_transform, "Scale X", &scaling.x, ID_SCALE, transform_cb);
    new GLUI_Spinner(panel_transform, "Scale Y", &scaling.y, ID_SCALE, transform_cb);
    new GLUI_Spinner(panel_transform, "Scale Z", &scaling.z, ID_SCALE, transform_cb);

    // Reset transform button
    new GLUI_Column(panel_transform, true);
    new GLUI_Button(panel_transform, "Reset", ID_IDENTITY, transform_cb);

    // Setup scene graph and live vars
    sg = new SceneGraph();
    readLiveVars(sg->getCurrent());

    glutMainLoop();
    delete sg;
    return 0;
}
