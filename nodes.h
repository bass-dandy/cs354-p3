#ifndef __NODES_H__
#define __NODES_H__

#include <string>
#include <vector>

#include "geom.h"

// Node types
enum {
    NODE_OBJECT,
    NODE_TRANSFORM,
    NODE_GEOM,
    NODE_ATTR,
    NODE_LIGHT,
    NODE_CAMERA
};

// A generic scene graph node
class SGNode {
    
    protected:
        
        std::string name;
        SGNode *parent;

        SGNode(std::string name) : name(name) {}
    
    public:

        std::string getName() { return name; }

        void setParent(SGNode *p) { parent = p; }

        SGNode *getParent() { return parent; }

        virtual int getNodeType() = 0;
};

// A generic parent node
class ParentNode : public SGNode {

    protected:

        ParentNode(std::string name) : SGNode(name) {}

    public:

        std::vector<SGNode*> children;

        void addChild(SGNode *n) {
            n->setParent(this);
            children.push_back(n);
        }

        void removeChild(int idx) {
            children.erase(children.begin() + idx - 1);
        }
};

class TransformNode : public ParentNode {

    public:
        
        Point translation;
        Point scaling;
        float rotation[16];
 
        TransformNode(std::string name) : 
            scaling(Point(1.0f, 1.0f, 1.0f)), 
            rotation{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 },
            ParentNode(name) {}

        int getNodeType() {
            return NODE_TRANSFORM;
        }

        void draw() {
            glTranslatef(translation.x, translation.y, translation.z);
            glScalef(scaling.x, scaling.y, scaling.z);
            glMultMatrixf(rotation);

            glBegin( GL_LINES );
         
            // X Axis
            glColor3f( 1.0, 0.0, 0.0 );
            glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 1.0, 0.0, 0.0 );

            // Y Axis
            glColor3f( 0.0, 1.0, 0.0 );
            glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 1.0, 0.0 );

            // Z Axis
            glColor3f( 0.0, 0.0, 1.0 );
            glVertex3f( 0.0, 0.0, 0.0 );  glVertex3f( 0.0, 0.0, 1.0 );

            glEnd();
        }
};

class CameraNode {

};

#endif
