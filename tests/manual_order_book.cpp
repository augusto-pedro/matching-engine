#include <cassert>
#include <iostream>

#include "order_book.hpp"

int main()
{
    OrderBook book;

    // Livro ainda vazio
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

    return 0;
}