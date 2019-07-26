Single, One level, non-polymorphic inheritance scenarios
===========================================================

We have the following classes:

    class Base {

        public: 
            Base() { std::cout << "In Base constructor\n"; }

            void doSomething() { std::cout << "Doing something in Base\n"; }

            ~Base() { std::cout << "In Base destructor\n"; }
    };
    
    class Derived : public Base {

        public: 
            Derived() { std::cout << "In Derived constructor\n"; }

            void doSomething() { std::cout << "Doing something in Derived\n"; }

            ~Derived() { std::cout << "In Derived destructor\n"; }
    };

Rules followed here are:

* No multiple inheritance
* No virtual members in base or derived classes
* Public non-virtual inheritance

### Scenario 1: Constructor and destructor invokation of derived class

    void scenario1() {
        Derived d;
        d.doSomething();
    }

    In Base constructor
    In Derived constructor
    Doing something in Derived
    In Derived destructor
    In Base destructor


### Scenario 2: Behaviour of references and pointers

    void scenario2() {
        Derived d;
        d.doSomething();

        Derived& rd = d;
        rd.doSomething();

        Derived* pd = &d;
        pd->doSomething();
    }

    In Base constructor
    In Derived constructor
    Doing something in Derived
    Doing something in Derived
    Doing something in Derived
    In Derived destructor
    In Base destructor

### Scenario 3: Upcasting

    void scenario3() {
        Derived d;
        d.doSomething();

        Base &rb = d;
        rb.doSomething();

        Base* pb = &d;
        pb->doSomething();
    }

    In Base constructor
    In Derived constructor
    Doing something in Derived
    Doing something in Base
    Doing something in Base
    In Derived destructor
    In Base destructor

Probably not the behaviour people from Java domain expect.    

### Scenario 4: Constructor and destructor invokation of allocated derived class 

    void scenario4() {
        Derived* d = new Derived();
        d->doSomething();
        delete d;
    }

    In Base constructor
    In Derived constructor
    Doing something in Derived
    In Derived destructor
    In Base destructor

### Scenario 5: Deleting base pointer of allocated derived

    void scenario5() {
        Derived * d = new Derived();
        d->doSomething();

        Base * b = d;
        b->doSomething();

        delete b;
    }

    In Base constructor
    In Derived constructor
    Doing something in Derived
    Doing something in Base
    In Base destructor

This positively leads to an error since, the Derived destructor is not called.

### Scenario 6: Upcasting pointer

    void scenario6() {
        Derived * d = new Derived();
        d->doSomething();

        Base * b = d;
        b->doSomething();

        Derived* x = static_cast<Derived*>(b);
        x->doSomething();

        delete x;
    }


    In Base constructor
    In Derived constructor
    Doing something in Derived
    Doing something in Base
    Doing something in Derived
    In Derived destructor
    In Base destructor

### Scenario 7: Upcasting reference

    void scenario7() {
        Derived d;
        d.doSomething();

        Base& b = d;
        b.doSomething();

        Derived& x = static_cast<Derived&>(b);
        x.doSomething();
    }

    In Base constructor
    In Derived constructor
    Doing something in Derived
    Doing something in Base
    Doing something in Derived
    In Derived destructor
    In Base destructor


Using virtual base class
------------------------

    class Base { ... };

    class Derived : virtual public Base { ... };


Third rules is modified here:

* No multiple inheritance
* No virtual members in base or derived classes
* Public virtual inheritance

### Scenarios 1 to 5

The behaviour remains extactly the same.


### Scenario 6 and 7

These are impossible. See https://stackoverflow.com/questions/53993917/downcasting-in-c-with-virtual-base-class.

The gist of it is that for virtual base, the pointer to Base is maintained in the Derived using similar data structures like vptr. Once we point to Base, there is no way of knowing what the Derived is.


### Note 1

As per the C++ standard there is no difference between the two following statements

    class Derived : virtual public Base { ... };

    class Derived : public virtual Base { ... };



Takeaways
---------

* Destruction happens in reverse step of construction (as expected).
* Upcasting non-polymorphic derived to non-polymorphic base type and using members is uses the base members not derived
    * This leads to a pitfall when destructing the class. Derived destructor is not called.
* Note 1: `Derived d()` syntax declare a function, not an object.
* Note 2 and 3: Upcasting non-polymorphic derived to non-polymorphic base is implicit
* Note 4: Downcasting non-polymorphic derived to non-polymorphic base is explicit - Needs to be static_cast.
* Note 5: dynamic_cast can only be used if base class is polymorphic.
* Upcasting non-polymorphic derived to virtual non-polymorphic base type and then downcasting back is impossible


Definition
----------


Polymorphic base: A base class which has at least one virtual member.

Virtual base: A base class inherited using the virtual keyword.



Behaviour:

Base    Derived         Base dest   Derived dest    Method called from casted base      Deletion of casted base
Non-Pol Poly/Non-Poly   non-virt    virt/non-virt   Base method called                  Only base destructor called.
Poly    Poly            virt        virt/non-virt   Derived method called               Derived dest, followed by base dest
Poly    Poly            non-virt    virt/non-virt   Derived method called               Only base destructor called.
Poly    Poly            Pure-virt   virt/non-virt       compilation error – linkage error
Poly    Poly            virt        pure-virt           compilation error – abstract class initialisation


Takeaways
---------

* Only the polymorphic bases decides the behaviour of the method being invoked.
