#include <cassert>
#include <iostream>
#include <vector>

#include "matching_engine.hpp"

int main()
{
    // TEST 1 - Limit matching
    {
        MatchingEngine engine;

        SubmissionResult buy = engine.submit_limit_order(
            Side::Buy,
            1000,
            100
        );

        assert(buy.order_id == 1);
        assert(buy.trades.empty());

        SubmissionResult sell1 = engine.submit_limit_order(
            Side::Sell,
            2000,
            100
        );

        assert(sell1.order_id == 2);
        assert(sell1.trades.empty());

        SubmissionResult sell2 = engine.submit_limit_order(
            Side::Sell,
            2000,
            200
        );

        assert(sell2.order_id == 3);
        assert(sell2.trades.empty());

        SubmissionResult aggressive_buy = engine.submit_limit_order(
            Side::Buy,
            2000,
            150
        );

        assert(aggressive_buy.trades.size() == 2);

        assert(aggressive_buy.trades[0].price == 2000);
        assert(aggressive_buy.trades[0].quantity == 100);
        assert(aggressive_buy.trades[0].sell_order_id == 2);

        assert(aggressive_buy.trades[1].price == 2000);
        assert(aggressive_buy.trades[1].quantity == 50);
        assert(aggressive_buy.trades[1].sell_order_id == 3);

        engine.print_book();

        std::cout << "\nLimit matching test passed!\n";
    }

    // TEST 2 - Market matching
    {
        MatchingEngine engine;

        engine.submit_limit_order(
            Side::Buy,
            1000,
            100
        );

        engine.submit_limit_order(
            Side::Sell,
            2000,
            100
        );

        engine.submit_limit_order(
            Side::Sell,
            2000,
            200
        );

        std::vector<Trade> trades1 =
            engine.submit_market_order(
                Side::Buy,
                150
            );

        assert(trades1.size() == 2);

        assert(trades1[0].price == 2000);
        assert(trades1[0].quantity == 100);

        assert(trades1[1].price == 2000);
        assert(trades1[1].quantity == 50);

        // Restam 150 unidades de venda @ 20
        std::vector<Trade> trades2 =
            engine.submit_market_order(
                Side::Buy,
                200
            );

        assert(trades2.size() == 1);
        assert(trades2[0].price == 2000);
        assert(trades2[0].quantity == 150);

        // As 50 restantes da market buy são descartadas.

        std::vector<Trade> trades3 =
            engine.submit_market_order(
                Side::Sell,
                200
            );

        assert(trades3.size() == 1);
        assert(trades3[0].price == 1000);
        assert(trades3[0].quantity == 100);

        engine.print_book();

        std::cout << "\nMarket order test passed!\n";
    }

    // TEST 3 - Cancellation
    {
        MatchingEngine engine;

        SubmissionResult buy1 = engine.submit_limit_order(
            Side::Buy,
            1000,
            100
        );

        SubmissionResult buy2 = engine.submit_limit_order(
            Side::Buy,
            1000,
            200
        );

        assert(buy1.order_id == 1);
        assert(buy2.order_id == 2);

        // Cancela a primeira ordem
        assert(engine.cancel_order(1));

        // Não pode cancelar novamente
        assert(!engine.cancel_order(1));

        // ID inexistente
        assert(!engine.cancel_order(999));

        // Se o cancelamento funcionou, uma market sell
        // deve encontrar order_2, e não order_1.
        std::vector<Trade> trades =
            engine.submit_market_order(
                Side::Sell,
                50
            );

        assert(trades.size() == 1);
        assert(trades[0].quantity == 50);
        assert(trades[0].price == 1000);
        assert(trades[0].buy_order_id == 2);

        engine.print_book();

        std::cout << "\nCancellation test passed!\n";
    }

    std::cout << "\nAll MatchingEngine tests passed!\n";

    return 0;
}