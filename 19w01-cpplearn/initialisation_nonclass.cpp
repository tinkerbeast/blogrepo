#include <iostream>

namespace {

    void scenario1(void) {
        // default initilaisation: T object ;   (1)
        // clause 1: when a variable with automatic, static, or thread-local storage duration is declared with no initializer
        int x;
        double d;
        char ∗ p;
        char arr[4];

        std::cout << "int=" << x << ", double=" << d << ", pointer=" << p << "\n";

        std::cout << "arr={" << arr[0] << ", " << arr[1] << ", " << arr[2] << ", "<< arr[3] "}\n";
    }

}

template <typename D, typename B>
void scenario3() {
    std::cout << "### scenario3\n";

    D d;
    d.doSomething("");

    B &rb = d;
    rb.doSomething("reference");

    B* pb = &d;
    pb->doSomething("pointer");
}

template <typename D, typename B>
void scenario5() {
    std::cout << "### scenario5\n";

    D * d = new D;
    d->doSomething("pointer");

    B * b = d;
    b->doSomething("pointer");

    delete b;
}

int main() {

    ::scenario1();
    
}


