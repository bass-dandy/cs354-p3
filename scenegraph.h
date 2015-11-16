#ifndef __SCENEGRAPH_H__
#define __SCENEGRAPH_H__

#include "nodes.h"

class SceneGraph {
    
    private:

        ParentNode *root;
        SGNode     *current;
        CameraNode *camera;

        void draw(SGNode *n) {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();

            if(n->getNodeType() == NODE_TRANSFORM || n->getNodeType() == NODE_OBJECT) {
                n->draw();

                ParentNode *p = static_cast<ParentNode*>(n);
                for(int i = 0; i < p->children.size(); ++i) {
                    draw(p->children[i]);
                }
            }
            glPopMatrix();
        }

    public:

        SceneGraph() {
            root = new ObjectNode("Root");
            current = root;

            TransformNode *t = new TransformNode("Cam Transform");
            static_cast<ParentNode*>(root)->addChild(t);
            
            CameraNode *c = new CameraNode();
            t->addChild(c);
            t->translation.z = -1;
            camera = c;

            t = new TransformNode();
            static_cast<ParentNode*>(root)->addChild(t);
        }

        SGNode *getCurrent() {
            return current;
        }

        SGNode *selectChild(int idx) {
            int type = current->getNodeType();
            if(type == NODE_TRANSFORM || type == NODE_OBJECT) {
                ParentNode *p = static_cast<ParentNode*>(current);
                if(idx < p->children.size()) {
                    current = p->children[idx];
                }
            }
            return current;
        }

        SGNode *selectParent() {
            if(current->getParent() != NULL) {
                current = current->getParent();
            }
            return current;
        }

        SGNode *addChild(int type) {
            SGNode *n = NULL;
            if(current->getNodeType() == NODE_OBJECT || current->getNodeType() == NODE_TRANSFORM) {
                switch(type) {
                    case NODE_OBJECT:
                        n = new ObjectNode();
                        static_cast<ParentNode*>(current)->addChild(n);
                        break;
                    case NODE_TRANSFORM:
                        n = new TransformNode();
                        static_cast<ParentNode*>(current)->addChild(n);
                        break;
                    case NODE_GEOM:
                        if(current->getNodeType() == NODE_OBJECT) {
                            ObjectNode *o = static_cast<ObjectNode*>(current);
                            if(o->geom == NULL) {
                                o->geom = new GeometryNode();
                                n = o->geom;
                            } else {
                                std::cout << "Error: cannot add multiple geometry nodes to one object" << std::endl;
                            }
                        } else {
                            std::cout << "Error: cannot add geometry node to non-object node" << std::endl;
                        }
                        break;
                    case NODE_ATTR:
                        if(current->getNodeType() == NODE_OBJECT) {
                            ObjectNode *o = static_cast<ObjectNode*>(current);
                            if(o->attr == NULL) {
                                o->attr = new AttributeNode();
                                n = o->attr;
                            } else {
                                std::cout << "Error: cannot add multiple attribute nodes to one object" << std::endl;
                            }
                        } else {
                            std::cout << "Error: cannot add attribute node to non-object node" << std::endl;
                        }
                        break;
                    case NODE_LIGHT:
                        n = new LightNode();
                        static_cast<ParentNode*>(current)->addChild(n);
                        break;
                }
            } else {
                std::cout << "Error: cannot add child to node types other than Object or Transform" << std::endl;
            }
            return n;
        }

        void deleteChild(int idx) {
            if(current->getNodeType() == NODE_OBJECT || current->getNodeType() == NODE_TRANSFORM) {
                static_cast<ParentNode*>(current)->deleteChild(idx);
            }
        }

        void display() {
            camera->draw();
            for(int i = 1; i < root->children.size(); ++i) {
                draw(root->children[i]);
            }
        }
};

#endif
