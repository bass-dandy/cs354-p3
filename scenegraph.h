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

            if(n->getNodeType() == NODE_TRANSFORM) {
                TransformNode *t = (TransformNode*) n;
                t->draw();
                for(int i = 0; i < t->children.size(); ++i) {
                    draw(t->children[i]);
                }
            }
            glPopMatrix();
        }

    public:

        SceneGraph() {
            root = new TransformNode("root");
            current = root;

            TransformNode *t = new TransformNode("child");
            static_cast<TransformNode*>(root)->addChild(t);
        }

        SGNode *getCurrent() {
            return current;
        }

        SGNode *selectChild(int idx) {
            int type = current->getNodeType();
            if(type == NODE_TRANSFORM || type == NODE_OBJECT) {
                TransformNode *t = static_cast<TransformNode*>(current);
                if(idx < t->children.size()) {
                    current = t->children[idx];
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

        void display() {
            draw(root);
        }
};

#endif
