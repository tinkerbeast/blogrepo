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
    void doSomething(std::string arg) { std::cout << "Doing something in Base " << arg << "\n"; }
    ~Base() { std::cout << "In Base destructor\n"; }
};    

class VBase {
public: 
    VBase() { std::cout << "In VBase constructor\n"; }
    virtual void doSomething(std::string arg) { std::cout << "Doing something in VBase " << arg << "\n"; }
    ~VBase() { std::cout << "In VBase destructor\n"; }

};



class DerivedFromBase : VIRTUALKEYWORD public Base {
public: 
    DerivedFromBase() { std::cout << "In DerivedFromBase constructor\n"; }
    void doSomething(std::string arg) { std::cout << "Doing something in DerivedFromBase " << arg << "\n"; }
    ~DerivedFromBase() { std::cout << "In DerivedFromBase destructor\n"; }
};

class DerivedFromVBase : VIRTUALKEYWORD public VBase {
public: 
    DerivedFromVBase() { std::cout << "In DerivedFromVBase constructor\n"; }
    void doSomething(std::string arg) { std::cout << "Doing something in DerivedFromVBase " << arg << "\n"; }
    ~DerivedFromVBase() { std::cout << "In DerivedFromVBase destructor\n"; }
};

class VDerivedFromBase : VIRTUALKEYWORD public Base {
public: 
    VDerivedFromBase() { std::cout << "In VDerivedFromBase constructor\n"; }
    virtual void doSomething(std::string arg) { std::cout << "Doing something in VDerivedFromBase " << arg << "\n"; }
    virtual ~VDerivedFromBase() { std::cout << "In VDerivedFromBase destructor\n"; }
};

class VDerivedFromVBase : VIRTUALKEYWORD public VBase {
public: 
    VDerivedFromVBase() { std::cout << "In VDerivedFromVBase constructor\n"; }
    virtual void doSomething(std::string arg) { std::cout << "Doing something in VDerivedFromVBase " << arg << "\n"; }
    virtual ~VDerivedFromVBase() { std::cout << "In VDerivedFromVBase destructor\n"; }
};

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

    scenario5<DerivedFromBase, Base>();
    scenario5<DerivedFromVBase, VBase>();
    scenario5<VDerivedFromBase, Base>();
    scenario5<VDerivedFromVBase, VBase>();
    
}


