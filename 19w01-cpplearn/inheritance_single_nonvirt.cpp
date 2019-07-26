#include <iostream>

//#define VIRTBASE

#ifdef VIRTBASE 
#define VIRTUALKEYWORD virtual
#else
#define VIRTUALKEYWORD
#endif

namespace {

class Base {

public: 
    Base() { std::cout << "In Base constructor\n"; }

    void doSomething() { std::cout << "Doing something in Base\n"; }

    ~Base() { std::cout << "In Base destructor\n"; }
};

class Derived : VIRTUALKEYWORD public Base {

public: 
    Derived() { std::cout << "In Derived constructor\n"; }

    void doSomething() { std::cout << "Doing something in Derived\n"; }

    ~Derived() { std::cout << "In Derived destructor\n"; }
};


void scenario1() {
    Derived d; // NOTE1: Using Derived d() here becomes a function declaraion
    d.doSomething();
}

void scenario2() {
    Derived d;
    d.doSomething();

    Derived& rd = d;
    rd.doSomething();

    Derived* pd = &d;
    pd->doSomething();
}

void scenario3() {
    Derived d;
    d.doSomething();

    Base &rb = d; // NOTE2: Upcasting to reference is implicit
    rb.doSomething();

    Base* pb = &d;
    pb->doSomething(); // NOTE3: Upcasting to pointer is implicit
}

void scenario4() {
    Derived* d = new Derived();
    d->doSomething();
    delete d;
}

void scenario5() {
    Derived * d = new Derived();
    d->doSomething();

    Base * b = d;
    b->doSomething();

    delete b;
}


#ifndef VIRTBASE
void scenario6() {
    Derived * d = new Derived();
    d->doSomething();

    Base * b = d;
    b->doSomething();

    //Derived* x = b; // NOTE4: Downcasting leads to compilation error conversion is not permissive(implicit)
    //Derived* x = dynamic_cast<Derived*>(b); // NOTE5: Downcasting using dymanic leads to compilation error since source type is not polymorphic(inclusion or subtyping)
    Derived* x = static_cast<Derived*>(b);
    x->doSomething();

    delete x;
}
#else
void scenario6() {
    // NOTE v1: It isn't possible to downcast from a virtual base
}
#endif

#ifndef VIRTBASE
void scenario7() {
    Derived d;
    d.doSomething();

    Base& b = d;
    b.doSomething();

    Derived& x = static_cast<Derived&>(b);
    x.doSomething();
}
#else
void scenario7() {
    // NOTE v1: It isn't possible to downcast from a virtual base
}
#endif

}

int main() {

    std::cout << "### Scenario 1\n";
    ::scenario1();
    std::cout << "### Scenario 2\n";
    ::scenario2();
    std::cout << "### Scenario 3\n";
    ::scenario3();
    std::cout << "### Scenario 4\n";
    ::scenario4();
    std::cout << "### Scenario 5\n";
    ::scenario5();
    std::cout << "### Scenario 6\n";
    ::scenario6();
    std::cout << "### Scenario 7\n";
    ::scenario6();
}

