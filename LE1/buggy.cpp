#include <iostream>
#include <cstring>

struct Point {
    int x, y;

    Point () : x(), y() {}
    Point (int _x, int _y) : x(_x), y(_y) {}
};

class Shape {
    int vertices;
    Point** points;

    public :
    Shape (int _vertices) {
        vertices = _vertices;
        points = new Point*[vertices];
        for (int i = 0; i < vertices; i++) {
            points[i] = new Point();
        }
    }

    ~Shape() {
        for (int i = 0; i < vertices; i++) {
            delete points[i];
        }

        delete[] points;
    }

    void addPoints(Point pts[]/* formal parameter for unsized array called pts */) {
        for (int i = 0; i < vertices; i++) {
            memcpy(points[i], &pts[i%vertices], sizeof(Point));
        }
    }

    double area() {
        int temp = 0;
        for (int i = 0; i < vertices; i++) {
            // FIXME: there are two methods to access members of pointers
            //        use one to fix lhs and the other to fix rhs
            int lhs = points[i]->x * points[(i+1) % vertices]->y; // ->
            int rhs = (*points[(i+1) % vertices]).x * (*points[i]).y; // *
            temp += (lhs - rhs);
        }
        double area = abs(temp)/2.0;
        return area;
    }
};

int main() {
    // FIXME: create the following points using the three different methods
    //        of defining structs:
            Point tri1(0, 0); //constructor

            Point tri2; //individual
            tri2.x = 1;;
            tri2.y = 2;


            Point tri3 = Point(2, 0); //set equal

    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape* tri = new Shape(3);
    tri->addPoints(triPts);

    // FIXME: create the following points using your preferred struct
    //        definition:
            Point quad1(0, 0);
            Point quad2(0, 2);
            Point quad3(2, 2);
            Point quad4(2, 0);

    // adding points to quad
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape* quad = new Shape(4);
    quad->addPoints(quadPts);

    // FIXME: print out area of tri and area of quad
    std::cout << "Tri-area: " << tri->area() << " | " << "Quad area: " << quad->area() << std::endl;

    delete tri;
    delete quad;

    return 0;
}
