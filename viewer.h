#include <stack>
#include <cmath>

#include "loader.h"
#include "geom.h"

class ModelViewer {

    private:

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

                const Point &getPos() {
                    return this->pos;
                }
        };

        // Display variables
        bool isCameraCoordinates = false;
        bool isFaceNormals       = false;
        bool isVertexNormals     = false;
        int mode = MODE_LIT;
        int w;
        int h;

        // Model stack
        std::stack<Trimesh> models;
        Camera camera;

    public:

        void display() {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if(!models.empty()) {
                float zNear = 0.01f;
                float zFar  = zNear + camera.dist + models.top().getMaxDelta();

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluPerspective(45.0, w/h, zNear, zFar);

                camera.update();
                models.top().draw(mode, isVertexNormals, isFaceNormals);
            }
            glFlush();
        }


        void loadModel(std::string filename) {
            Trimesh tm;
            TrimeshLoader ldr;
            ldr.loadOBJ(filename.c_str(), &tm);
            models.push(tm);

            camera.lookAt(tm.getOrigin());
            camera.dist = 0.01f + tm.getMaxDelta() * 1.5f;
        }


        void deleteModel() {
            if(!models.empty()) {
                models.pop();
            }
        }

        void loadIdentity() {
            if(!models.empty()) {
                models.top().identity();
            }
        }

        void setViewport(int w, int h) {
            this->w = w;
            this->h = h;
        }

        void useCameraCoordinates(bool isCameraCoordinates) {
            this->isCameraCoordinates = isCameraCoordinates;
        }

        void showFaceNormals(bool show) {
            isFaceNormals = show;
        }

        void showVertexNormals(bool show) {
            isVertexNormals = show;
        }

        Point getTranslation() {
            return models.top().getTranslation();
        }

        void setTranslation(Point p) {
            models.top().setTranslation(p);
        }

        Point getScale() {
            return models.top().getScale();
        }

        void setScale(Point p) {
            models.top().setScale(p);
        }

        float *getRotation() {
            return models.top().getRotation();
        }

        void setRotation(float *r) {
            models.top().setRotation(r);
        }

        void setMode(int mode) { this->mode = mode; }

        void orbit(float x, float y) { this->camera.orbit(x, y); }

        void zoom(float delta) { this->camera.zoom(delta); }

        void pan(float x, float y) { this->camera.pan(x, y); }
};
