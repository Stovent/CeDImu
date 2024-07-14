#include "MutexPtr.hpp"

#include <cstdio>

struct A
{
    ~A() { printf("A destroyed\n"); }

    void Print() const { printf("A prints\n"); }
};

/** \brief Function used to verify the behavior of the MutexPtr class. */
void testMutexPtr()
{
    {
        MutexPtr<int> mi{14};
        auto bi = mi.Lock();
        (*bi)++;
        printf("%d\n", *bi);
    }

    {
        printf("before\n");
        MutexPtr<A> m{};
        printf("locking\n");
        const MutexPtr<A>::Guard l = m.Lock();
        l->Print();
        printf("unlocking\n");
    }
}
