#ifndef __GEOM_H__
#define __GEOM_H__

#include <vector>
#include <iostream>
#include <cfloat>
#include <cmath>

// Rendering modes
enum {
    MODE_POINT,
    MODE_WIRE,
    MODE_SOLID,
    MODE_LIT
};

// A point in 3-space, includes a normal if it is a trimesh vertex
struct Point {
    float x;
    float y;
    float z;

    Point *normal;

    Point(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}

    Point &operator+=(const Point &other) {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;
        return *this;
    }

    Point &operator*=(const Point &other) {
        this->x *= other.x;
        this->y *= other.y;
        this->z *= other.z;
        return *this;
    }

    Point &operator*=(float scale) {
        this->x *= scale;
        this->y *= scale;
        this->z *= scale;
        return *this;
    }

    Point &operator/=(float scale) {
        this->x /= scale;
        this->y /= scale;
        this->z /= scale;
        return *this;
    }

    Point operator+(const Point &other) {
        return Point(*this) += other;
    }

    Point operator*(const Point &other) {
        return Point(*this) *= other;
    }

    Point operator*(float scale) {
        return Point(*this) *= scale;
    }

    Point normalize() {
        float magnitude = sqrt(x*x + y*y + z*z);
        return Point(x/magnitude, y/magnitude, z/magnitude);
    }
};


class Trimesh {

    private:

        struct Face {
            int ids[3];
            Point normal;

            Face(const int *ids, std::vector<Point> &verts) {
                for(int i = 0; i < 3; ++i) {
                    this->ids[i] = ids[i];
                }
                computeNormal(verts);    
            }

            private:

                void computeNormal(std::vector<Point> &verts) {
                    Point a = verts[ids[0]];
                    Point b = verts[ids[1]];
                    Point c = verts[ids[2]];
                    
                    Point u(b.x - a.x, b.y - a.y, b.z - a.z);
                    Point v(c.x - a.x, c.y - a.y, c.z - a.z);

                    // Compute cross product
                    float nx = u.y * v.z - u.z * v.y;
                    float ny = u.z * v.x - u.x * v.z;
                    float nz = u.x * v.y - u.y * v.x;

                    normal = Point(nx, ny, nz).normalize();

                    // Add normal to each vertex on face
                    for(int i = 0; i < 3; ++i) {
                        *(verts[ids[i]].normal) += normal;
                    }
                }
        };

        std::vector<Point> verts;
        std::vector<Face>  faces;
        
        // For computing initial viewing matrix
        Point origin;
        float minX = FLT_MAX;
        float maxX = FLT_MIN;
        float minY = FLT_MAX;
        float maxY = FLT_MIN;
        float minZ = FLT_MAX;
        float maxZ = FLT_MIN;

        // For computing transformations
        float rotation[16];
        Point translation;
        Point scaling;

        void drawVerts() {
            glPointSize(3.0);
            glBegin(GL_POINTS);
            for(int i = 0; i < verts.size(); ++i) {
                Point p = verts[i];
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }

        void drawNormals(bool isVertexNormals, bool isFaceNormals) {
            if(isVertexNormals) {
                glColor3f(0.0f, 1.0f, 1.0f);
                
                for(int i = 0; i < verts.size(); ++i) {
                    Point p = verts[i];
                    Point n = (p.normal)->normalize();

                    glBegin(GL_LINES);
                    glVertex3f(p.x, p.y, p.z);
                    glVertex3f(p.x + n.x, p.y + n.y, p.z + n.z);
                    glEnd();
                }
            }
            if(isFaceNormals) {
                glColor3f(1.0f, 0.0f, 1.0f);
                
                for(int i = 0; i < faces.size(); ++i) {
                    Face f = faces[i];
                    Point p(0.0f, 0.0f, 0.0f);
                    
                    // Compute center of face
                    p += verts[f.ids[0]];
                    p += verts[f.ids[1]];
                    p += verts[f.ids[2]];
                    p /= 3.0f;

                    Point n = f.normal.normalize();

                    glBegin(GL_LINES);
                    glVertex3f(p.x, p.y, p.z);
                    glVertex3f(n.x + p.x, n.y + p.y, n.z + p.z);
                    glEnd();
                }
            }
        }

        void draw() {
            for(int i = 0; i < faces.size(); ++i) {
                Face f = faces[i];
            
                glBegin(GL_TRIANGLES);
                for(int j = 0; j < 3; ++j) { 
                    Point p = verts[f.ids[j]];
                    Point n = p.normal->normalize();
                    glNormal3f(n.x, n.y, n.z);
                    glVertex3f(p.x, p.y, p.z);
                }
                glEnd();
            }
        }

    public:

        Trimesh() : scaling(Point(1.0f, 1.0f, 1.0f)), rotation{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 } {}

        void addFace(const int *ids) {
            Face f(ids, verts);
            faces.push_back(f);
        }

        void addVertex(const float *values) {
            float x = values[0];
            float y = values[1];
            float z = values[2];

            Point p(x, y, z);
            p.normal = new Point(0.0f, 0.0f, 0.0f);
            verts.push_back(p);

            // Update the model's origin
            if(x < minX)
                minX = x;
            else if(x > maxX)
                maxX = x;
            if(y < minY)
                minY = y;
            else if(y > maxY)
                maxY = y;
            if(z < minZ)
                minZ = z;
            else if(z > maxZ)
                maxZ = z;

            origin = Point((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f);
        }

        const Point getOrigin() {
            return this->origin;
        }

        float getMaxDelta() {
            using namespace std;
            float maxDelta = abs(maxX - minX);

            maxDelta = abs(maxY - minY) > maxDelta ? abs(maxY - minY) : maxDelta;
            maxDelta = abs(maxZ - minZ) > maxDelta ? abs(maxZ - minZ) : maxDelta;

            return maxDelta;
        }

        void identity() {
            // Reset all transformations
            this->translation = Point();
            this->scaling     = Point(1.0f, 1.0f, 1.0f);
            //this->rotation    = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
        }

        void draw(int mode, bool isVertexNormals, bool isFaceNormals) {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef(translation.x, translation.y, translation.z);
            glScalef(scaling.x, scaling.y, scaling.z);
            glMultMatrixf(rotation);
            switch(mode) {
                case MODE_POINT:
                    glColor3f(1.0f, 0.0f, 0.0f);
                    drawVerts();
                    break;
                case MODE_WIRE:
                    glColor3f(0.0f, 1.0f, 0.0f);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    draw();
                    break;
                case MODE_SOLID:
                    glColor3f(0.6f, 0.6f, 0.6f);
                    glPolygonMode(GL_FRONT, GL_FILL);
                    draw();
                    break;
                case MODE_LIT:
                    glColor3f(0.6f, 0.6f, 0.6f);
                    glPolygonMode(GL_FRONT, GL_FILL);
                    glEnable(GL_LIGHTING);
                    glEnable(GL_LIGHT0);
                    glEnable(GL_COLOR_MATERIAL);
                    glEnable(GL_NORMALIZE);
                    draw();
                    glDisable(GL_LIGHTING);
                    break;
            }
            drawNormals(isVertexNormals, isFaceNormals);
            glPopMatrix();
        }

        Point getTranslation() {
            return translation;
        }

        void setTranslation(Point p) {
            translation.x = p.x;
            translation.y = p.y;
            translation.z = p.z;
        }

        Point getScale() {
            return scaling;
        }

        void setScale(Point p) {
            scaling.x = p.x;
            scaling.y = p.y;
            scaling.z = p.z;
        }

        float *getRotation() {
            return rotation;
        }

        void setRotation(float *r) {
            for(int i = 0; i < 16; ++i) {
                rotation[i] = r[i];
            }
        }
};

#endif
