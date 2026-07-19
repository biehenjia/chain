#include <algebra/algebra.hpp>


namespace chains {

    #define INSTANTIATE(L) \
        template uint32_t add<L>(Interner<L>&, uint32_t, uint32_t); \
        template uint32_t mul<L>(Interner<L>&, uint32_t, uint32_t); \
        template uint32_t pow<L>(Interner<L>&, uint32_t, uint32_t); \
        template uint32_t log<L>(Interner<L>&, uint32_t, uint32_t); \
        template uint32_t sin<L>(Interner<L>&, uint32_t); \
        template uint32_t cos<L>(Interner<L>&, uint32_t); \
        template uint32_t tan<L>(Interner<L>&, uint32_t); \
        template uint32_t cot<L>(Interner<L>&, uint32_t);

    INSTANTIATE(double)
    INSTANTIATE(Symbolic)

    #undef INSTANTIATE

}
