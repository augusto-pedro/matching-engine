#include <cassert>
#include <iostream>

#include "order.hpp"
#include "order_node.hpp"
#include "price_level.hpp"

Order make_order(
    unsigned long long id,
    unsigned long long priority
) {
    return Order{
        id,
        OrderType::Limit,
        Side::Buy,
        1000,
        100,
        priority,
        PegReference::None
    };
}

int main()
{
    // ========================================
    // TEST 1: append
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 1));
        OrderNode B(make_order(2, 2));
        OrderNode C(make_order(3, 3));

        assert(level.empty());
        assert(level.get_size() == 0);
        assert(level.front() == nullptr);

        level.append(&A);

        assert(!level.empty());
        assert(level.get_size() == 1);

        assert(level.front() == &A);

        assert(A.previous == nullptr);
        assert(A.next == nullptr);
        assert(A.level == &level);

        level.append(&B);
        level.append(&C);

        // Esperado:
        //
        // A <-> B <-> C

        assert(level.get_size() == 3);

        assert(A.previous == nullptr);
        assert(A.next == &B);

        assert(B.previous == &A);
        assert(B.next == &C);

        assert(C.previous == &B);
        assert(C.next == nullptr);

        std::cout << "TEST 1 passed\n";
    }


    // ========================================
    // TEST 2: detach do meio
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 1));
        OrderNode B(make_order(2, 2));
        OrderNode C(make_order(3, 3));

        level.append(&A);
        level.append(&B);
        level.append(&C);

        level.detach(&B);

        // Antes:
        // A <-> B <-> C
        //
        // Depois:
        // A <-> C
        //
        // B isolado

        assert(level.get_size() == 2);

        assert(A.previous == nullptr);
        assert(A.next == &C);

        assert(C.previous == &A);
        assert(C.next == nullptr);

        assert(B.previous == nullptr);
        assert(B.next == nullptr);
        assert(B.level == nullptr);

        std::cout << "TEST 2 passed\n";
    }


    // ========================================
    // TEST 3: detach do head
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 1));
        OrderNode B(make_order(2, 2));
        OrderNode C(make_order(3, 3));

        level.append(&A);
        level.append(&B);
        level.append(&C);

        level.detach(&A);

        // Antes:
        // A <-> B <-> C
        //
        // Depois:
        // B <-> C

        assert(level.get_size() == 2);
        assert(level.front() == &B);

        assert(B.previous == nullptr);
        assert(B.next == &C);

        assert(C.previous == &B);
        assert(C.next == nullptr);

        assert(A.previous == nullptr);
        assert(A.next == nullptr);
        assert(A.level == nullptr);

        std::cout << "TEST 3 passed\n";
    }


    // ========================================
    // TEST 4: detach do tail
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 1));
        OrderNode B(make_order(2, 2));
        OrderNode C(make_order(3, 3));

        level.append(&A);
        level.append(&B);
        level.append(&C);

        level.detach(&C);

        // Resultado:
        //
        // A <-> B

        assert(level.get_size() == 2);

        assert(A.previous == nullptr);
        assert(A.next == &B);

        assert(B.previous == &A);
        assert(B.next == nullptr);

        assert(C.previous == nullptr);
        assert(C.next == nullptr);
        assert(C.level == nullptr);

        std::cout << "TEST 4 passed\n";
    }


    // ========================================
    // TEST 5: detach do único elemento
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 1));

        level.append(&A);

        assert(level.get_size() == 1);

        level.detach(&A);

        assert(level.empty());
        assert(level.get_size() == 0);
        assert(level.front() == nullptr);

        assert(A.previous == nullptr);
        assert(A.next == nullptr);
        assert(A.level == nullptr);

        std::cout << "TEST 5 passed\n";
    }


    // ========================================
    // TEST 6: insert_by_priority no meio
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 2));
        OrderNode B(make_order(2, 5));
        OrderNode C(make_order(3, 8));
        OrderNode P(make_order(4, 4));

        level.append(&A);
        level.append(&B);
        level.append(&C);

        level.insert_by_priority(&P);

        // Prioridades:
        //
        // A = 2
        // P = 4
        // B = 5
        // C = 8
        //
        // Esperado:
        //
        // A <-> P <-> B <-> C

        assert(level.get_size() == 4);

        assert(A.next == &P);

        assert(P.previous == &A);
        assert(P.next == &B);

        assert(B.previous == &P);
        assert(B.next == &C);

        assert(C.previous == &B);
        assert(C.next == nullptr);

        assert(P.level == &level);

        std::cout << "TEST 6 passed\n";
    }


    // ========================================
    // TEST 7: insert_by_priority no início
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 2));
        OrderNode B(make_order(2, 5));
        OrderNode P(make_order(3, 1));

        level.append(&A);
        level.append(&B);

        level.insert_by_priority(&P);

        // Esperado:
        //
        // P <-> A <-> B

        assert(level.front() == &P);

        assert(P.previous == nullptr);
        assert(P.next == &A);

        assert(A.previous == &P);
        assert(A.next == &B);

        assert(B.previous == &A);
        assert(B.next == nullptr);

        std::cout << "TEST 7 passed\n";
    }


    // ========================================
    // TEST 8: insert_by_priority no final
    // ========================================

    {
        PriceLevel level(1000);

        OrderNode A(make_order(1, 2));
        OrderNode B(make_order(2, 5));
        OrderNode P(make_order(3, 10));

        level.append(&A);
        level.append(&B);

        level.insert_by_priority(&P);

        // Esperado:
        //
        // A <-> B <-> P

        assert(A.next == &B);

        assert(B.previous == &A);
        assert(B.next == &P);

        assert(P.previous == &B);
        assert(P.next == nullptr);

        assert(P.level == &level);

        std::cout << "TEST 8 passed\n";
    }


    std::cout << "\nAll PriceLevel tests passed!\n";

    return 0;
}