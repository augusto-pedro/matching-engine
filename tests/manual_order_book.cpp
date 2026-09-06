#include <cassert>
#include <iostream>

#include "order_book.hpp"

int main()
{
    // TEST 1
    {
        OrderBook book;

        assert(book.best_bid() == nullptr);
        assert(book.best_offer() == nullptr);

        Order order(
            1,
            OrderType::Limit,
            Side::Buy,
            1000,
            100,
            1,
            PegReference::None
        );

        OrderNode *node = book.create_order(order);

        assert(node != nullptr);
        assert(node->order.id == 1);

        OrderNode *found = book.find_order(1);
        assert(found == node);

        OrderNode *not_found = book.find_order(999);
        assert(not_found == nullptr);

        std::cout << "Initial OrderBook tests passed!\n";
    }

    // TEST 2
    {
        OrderBook book;

        Order order1(
            1,
            OrderType::Limit,
            Side::Buy,
            1000,
            100,
            1,
            PegReference::None
        );

        Order order2(
            2,
            OrderType::Limit,
            Side::Buy,
            1010,
            200,
            2,
            PegReference::None
        );

        OrderNode *node1 = book.create_order(order1);
        OrderNode *node2 = book.create_order(order2);

        book.add_to_book(node1);
        book.add_to_book(node2);

        assert(book.best_bid() != nullptr);
        assert(book.best_bid()->get_price() == 1010);
        assert(book.best_bid()->front() == node2);

        std::cout << "Bid ordering test passed!\n";
    }

    // TEST 3
    {
        OrderBook book;

        Order order1(
            1,
            OrderType::Limit,
            Side::Sell,
            1060,
            100,
            1,
            PegReference::None
        );

        Order order2(
            2,
            OrderType::Limit,
            Side::Sell,
            1050,
            200,
            2,
            PegReference::None
        );

        OrderNode *node1 = book.create_order(order1);
        OrderNode *node2 = book.create_order(order2);

        book.add_to_book(node1);
        book.add_to_book(node2);

        assert(book.best_offer() != nullptr);
        assert(book.best_offer()->get_price() == 1050);
        assert(book.best_offer()->front() == node2);

        std::cout << "Offer ordering test passed!\n";
    }

    // TEST 4
    {
        OrderBook book;

        Order order1(
            1,
            OrderType::Limit,
            Side::Buy,
            1000,
            100,
            1,
            PegReference::None
        );

        Order order2(
            2,
            OrderType::Limit,
            Side::Buy,
            1000,
            200,
            2,
            PegReference::None
        );

        Order order3(
            3,
            OrderType::Limit,
            Side::Buy,
            1000,
            300,
            3,
            PegReference::None
        );

        OrderNode *A = book.create_order(order1);
        OrderNode *B = book.create_order(order2);
        OrderNode *C = book.create_order(order3);

        book.add_to_book(A);
        book.add_to_book(B);
        book.add_to_book(C);

        PriceLevel *level = book.best_bid();

        assert(level != nullptr);
        assert(level->get_size() == 3);

        assert(level->front() == A);

        assert(A->previous == nullptr);
        assert(A->next == B);

        assert(B->previous == A);
        assert(B->next == C);

        assert(C->previous == B);
        assert(C->next == nullptr);

        std::cout << "FIFO test passed!\n";
    }

    std::cout << "\nAll OrderBook tests passed!\n";

    return 0;
}