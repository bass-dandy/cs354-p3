#ifndef __SCENEGRAPH_H__
#define __SCENEGRAPH_H__

#include "nodes.h"

class SceneGraph {
    
    private:

        SGNode *root;
        SGNode *current;

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
            root = new ObjectNode("root");
            current = root;

            TransformNode *t = new TransformNode();
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
            SGNode *n;
            if(current->getNodeType() == NODE_OBJECT || current->getNodeType() == NODE_TRANSFORM) {
                switch(type) {
                    case NODE_OBJECT:
                        n = new ObjectNode();
                        break;
                    case NODE_TRANSFORM:
                        n = new TransformNode();
                        break;
                    case NODE_LIGHT:
                        // TODO
                        break;
                }
                static_cast<ParentNode*>(current)->addChild(n);
            }
            return n;
        }

        void deleteChild(int idx) {
            if(current->getNodeType() == NODE_OBJECT || current->getNodeType() == NODE_TRANSFORM) {
                static_cast<ParentNode*>(current)->deleteChild(idx);
            }
        }

        void display() {
            draw(root);
        }
};

#endif
