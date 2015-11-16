#ifndef __NODES_H__
#define __NODES_H__

#include <string>
#include <vector>

#include "geom.h"
#include "loader.h"

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
        SGNode *parent = NULL;

        SGNode(std::string name) : name(name) {}
    
    public:

        std::string getName() { return name; }

        void setParent(SGNode *p) { parent = p; }

        SGNode *getParent() { return parent; }

        virtual int getNodeType() = 0;
        
        virtual void draw() {};
};

// A generic parent node
class ParentNode : public SGNode {

    protected:

        ParentNode(std::string name) : SGNode(name) {}

    public:

        std::vector<SGNode*> children;

        void addChild(SGNode *n) {
            if(n != NULL) {
                n->setParent(this);
                children.push_back(n);
            }
        }

        void deleteChild(int idx) {
            if(idx < children.size()) {
                delete children[idx];
                children.erase(children.begin() + idx);
            }
        }
};

class TransformNode : public ParentNode {

    private:

        const float identity[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };

    public:
        
        Point translation;
        Point scaling;
        float rotation[16];
 
        TransformNode(std::string name) : 
            scaling(Point(1.0f, 1.0f, 1.0f)), 
            rotation{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 },
            ParentNode(name) {}

        TransformNode() : TransformNode("Transform") {}

        int getNodeType() {
            return NODE_TRANSFORM;
        }

        void reset() {
            translation = Point();
            scaling     = Point(1.0f, 1.0f, 1.0f);
            for(int i = 0; i < 16; ++i) {
                rotation[i] = identity[i];
            }
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

class GeometryNode : public SGNode {

    private:

        Trimesh *model = NULL;

    public:

        GeometryNode() : SGNode("Geometry") {}

        void loadModel(std::string filename) {
            if(model != NULL) {
                delete model;
            }
            model = new Trimesh();
            TrimeshLoader ldr;
            ldr.loadOBJ(filename.c_str(), model);
        }

        int getNodeType() {
            return NODE_GEOM;
        }

        void draw(int mode, bool drawFaceNormals, bool drawVertNormals) {
            if(model != NULL) {
                model->draw(mode, drawFaceNormals, drawVertNormals);
            }
        }
};

class AttributeNode : public SGNode {
    
    public:

        int  renderMode;
        bool drawFaceNormals;
        bool drawVertNormals;

        AttributeNode() : SGNode("Attributes") {}

        int getNodeType() {
            return NODE_ATTR;
        }
};

class ObjectNode : public ParentNode {

    public:

        GeometryNode  *geom = NULL;
        AttributeNode *attr = NULL;

        ObjectNode(std::string name) : ParentNode(name) {}

        ObjectNode() : ObjectNode("Object") {}

        int getNodeType() {
            return NODE_OBJECT;
        }

        void draw() {
            if(geom != NULL && attr != NULL) {
                geom->draw(attr->renderMode, attr->drawFaceNormals, attr->drawVertNormals);
            }
            else if(geom != NULL) {
                geom->draw(MODE_LIT, false, false);
            }
        }
};

class LightNode : public SGNode {

    private:
    
        const float pos[4] = {0, 0, 0, 1};

    public:

        LightNode() : SGNode("Light") {}

        int getNodeType() {
            return NODE_LIGHT;
        }

        void draw() {
            glLightfv( GL_LIGHT1, GL_POSITION, pos);
            glEnable( GL_LIGHT1 );
        }
};

class CameraNode : public SGNode {

    public:

        CameraNode() : SGNode("Camera") {}

        int getNodeType() {
            return NODE_CAMERA;
        }

        void draw() {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            parent->draw();
        }
};

#endif
