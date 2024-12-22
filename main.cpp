#include <iostream>
#include "package.h"
#include "storage_types.h"


int main() {

    Package p1;

    Package p2(std::move(p1));

    std::cout << 1;

    std::cout << p2.get_id();


}