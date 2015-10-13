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

struct Point {
    float x;
    float y;
    float z;

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

struct MatrixR3 {

    float mat[9];

    static MatrixR3 identity() {
        MatrixR3 i;
        i[0] = 1.0f; i[3] = 0.0f; i[6] = 0.0f;
        i[1] = 0.0f; i[4] = 1.0f; i[7] = 0.0f;
        i[2] = 0.0f; i[5] = 0.0f; i[8] = 1.0f;
        return i;
    }

    float &operator[](int idx) {
        return mat[idx];
    }

    const float operator[](int idx) const {
        return mat[idx];
    }

    MatrixR3 operator+(const MatrixR3 &other) {
        MatrixR3 sum;
        for(int i = 0; i < 9; ++i) {
            sum[i] = this->mat[i] + other[i];
        }
        return sum;
    }

    MatrixR3 operator*(const MatrixR3 &other) {
        MatrixR3 p;

        // Column 0
        p[0] = this->mat[0] * other[0] + this->mat[3] * other[1] + this->mat[6] * other[2];
        p[1] = this->mat[1] * other[0] + this->mat[4] * other[1] + this->mat[7] * other[2];
        p[2] = this->mat[2] * other[0] + this->mat[5] * other[1] + this->mat[8] * other[2];

        // Column 1
        p[3] = this->mat[0] * other[3] + this->mat[3] * other[4] + this->mat[6] * other[5];
        p[4] = this->mat[1] * other[3] + this->mat[4] * other[4] + this->mat[7] * other[5];
        p[5] = this->mat[2] * other[3] + this->mat[5] * other[4] + this->mat[8] * other[5];

        // Column 2
        p[6] = this->mat[0] * other[6] + this->mat[3] * other[7] + this->mat[6] * other[8];
        p[7] = this->mat[1] * other[6] + this->mat[4] * other[7] + this->mat[7] * other[8];
        p[8] = this->mat[2] * other[6] + this->mat[5] * other[7] + this->mat[8] * other[8];

        return p;
    }

    MatrixR3 operator*(float f) {
        MatrixR3 prod;
        for(int i = 0; i < 9; ++i) {
            prod[i] = this->mat[i] * f;
        }
        return prod;
    }

    Point operator*(const Point &p) {
        float x = mat[0] * p.x + mat[3] * p.y + mat[6] * p.z;
        float y = mat[1] * p.x + mat[4] * p.y + mat[7] * p.z;
        float z = mat[2] * p.x + mat[5] * p.y + mat[8] * p.z;
        return Point(x, y, z);
    }
};

class Trimesh {

    private:

        struct Face {
            std::vector<Point> normals;
            std::vector<Point> verts;

            Face(const int *ids, int count, std::vector<Point> &verts) {
                for(int i = 0; i < count; ++i) {
                    this->verts.push_back(verts[ids[i] - 1]);
                }
                computeNormals();    
            }

            private:

                void computeNormals() {
                    Point pivot = verts[0];
                    Point prev  = verts[1];

                    for(int i = 2; i < verts.size(); ++i) {
                        Point curr = verts[i];
                        
                        Point u(prev.x - pivot.x, prev.y - pivot.y, prev.z - pivot.z);
                        Point v(curr.x - pivot.x, curr.y - pivot.y, curr.z - pivot.z);

                        // Compute cross product
                        float nx = u.y * v.z - u.z * v.y;
                        float ny = u.z * v.x - u.x * v.z;
                        float nz = u.x * v.y - u.y * v.x;

                        Point n(nx, ny, nz);
                        normals.push_back(n);
                        prev = curr;
                    }
                }
        };

        std::vector<Point> vertices;
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
        MatrixR3 rotation;
        Point    translation;
        Point    scaling;


        void drawVerts() {
            glPointSize(3.0);
            glBegin(GL_POINTS);
            for(int i = 0; i < vertices.size(); ++i) {
                Point p = rotation * ((vertices[i] + translation) * scaling);
                glVertex3f(p.x, p.y, p.z);
            }
            glEnd();
        }

        void draw() {
            for(int i = 0; i < faces.size(); ++i) {
                Face f = faces[i];
            
                glBegin(GL_TRIANGLE_FAN);
                for(int j = 0; j < f.verts.size(); ++j) { 
                    if(j > 2) {
                        glNormal3f(f.normals[j - 2].x, f.normals[j - 2].y, f.normals[j - 2].z);
                    }
                    else if(j == 0) {
                        glNormal3f(f.normals[0].x, f.normals[0].y, f.normals[0].z);
                    }
                    Point p = rotation * ((f.verts[j] + translation) * scaling);
                    glVertex3f(p.x, p.y, p.z);
                }
                glEnd();
            }
        }

    public:

        Trimesh() : scaling(Point(1.0f, 1.0f, 1.0f)), rotation(MatrixR3::identity()) {}

        void addFace(const int *ids, int count) {
            Face f(ids, count, vertices);
            faces.push_back(f);
        }

        void addVertex(const float *values) {
            float x = values[0];
            float y = values[1];
            float z = values[2];

            Point p(x, y, z);
            vertices.push_back(p);

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

        void translate(float x, float y, float z) {
            this->translation += Point(x, y, z);
        }

        void scale(float x, float y, float z) {
            this->scaling += Point(x, y, z);
        }

        void rotate(float theta, float x, float y, float z) {
            Point axis = Point(x, y, z).normalize();
            MatrixR3 i = MatrixR3::identity();
            MatrixR3 w;

            w[0] = 0.0f;    w[3] = -axis.z; w[6] = axis.y;
            w[1] = axis.z;  w[4] = 0.0f;    w[7] = -axis.x;
            w[2] = -axis.y; w[5] = axis.x;  w[8] = 0.0f;

            MatrixR3 rot = i + w * sin(theta) + w * w * (1 - cos(theta));
            rotation = rotation * rot;
        }

        void identity() {
            this->translation = Point();
            this->scaling     = Point(1.0f, 1.0f, 1.0f);
            this->rotation    = MatrixR3::identity();
        }

        void draw(int mode, bool doVNormals, bool doSNormals) {
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
        }
};

#endif
