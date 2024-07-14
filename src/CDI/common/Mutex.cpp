#include "Mutex.hpp"

#include <cstdio>

struct A
{
    ~A() { printf("A destroyed\n"); }

    void Print() const { printf("A prints\n"); }
};

/** \brief Function used to verify the behavior of the Mutex class. */
void testMutex()
{
    {
        Mutex<int> mi{};
        auto bi = mi.Lock();
        (*bi)++;
    }

    {
        printf("before\n");
        Mutex<A> m{};
        printf("locking\n");
        const Mutex<A>::Guard l = m.Lock();
        l->Print();
        printf("unlocking\n");
    }
}
