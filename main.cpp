#include <iostream>
#include "my_vector.h"

int main()
{
    Vector <double> v1(3);
    v1.set_interpolation (BEZIER2);
    v1[0] = 1;
    v1[1] = 3;
    v1[2] = 1;
    std::cout << v1[0.5];
    Vector <bool> v3(2);
    v3[0] = true;
    v3[1] = false;
    return 0;
}
