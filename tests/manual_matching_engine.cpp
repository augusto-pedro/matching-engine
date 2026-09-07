#include <cassert>
#include <iostream>

#include "matching_engine.hpp"

int main()
{
    MatchingEngine engine;

    // BUY fica no livro
    SubmissionResult buy = engine.submit_limit_order(
        Side::Buy,
        1000,
        100
    );

    assert(buy.order_id == 1);
    assert(buy.trades.empty());

    // SELL a 20 também fica no livro
    SubmissionResult sell1 = engine.submit_limit_order(
        Side::Sell,
        2000,
        100
    );

    assert(sell1.order_id == 2);
    assert(sell1.trades.empty());

    // Segunda SELL no mesmo preço
    SubmissionResult sell2 = engine.submit_limit_order(
        Side::Sell,
        2000,
        200
    );

    assert(sell2.order_id == 3);
    assert(sell2.trades.empty());

    // Essa BUY cruza as duas sells
    SubmissionResult aggressive_buy = engine.submit_limit_order(
        Side::Buy,
        2000,
        150
    );

    assert(aggressive_buy.trades.size() == 2);

    // Primeiro consome order_2 inteira
    assert(aggressive_buy.trades[0].price == 2000);
    assert(aggressive_buy.trades[0].quantity == 100);
    assert(aggressive_buy.trades[0].sell_order_id == 2);

    // Depois consome 50 da order_3
    assert(aggressive_buy.trades[1].price == 2000);
    assert(aggressive_buy.trades[1].quantity == 50);
    assert(aggressive_buy.trades[1].sell_order_id == 3);

    engine.print_book();

    std::cout << "\nLimit matching test passed!\n";

    return 0;
}