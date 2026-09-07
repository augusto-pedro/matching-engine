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

    // TEST 4 - Modify: quantity decrease keeps priority
    {
        MatchingEngine engine;

        SubmissionResult first =
            engine.submit_limit_order(
                Side::Buy,
                1000,
                100
            );

        SubmissionResult second =
            engine.submit_limit_order(
                Side::Buy,
                1000,
                200
            );

        ModificationResult result =
            engine.modify_order(
                first.order_id,
                1000,
                50
            );

        assert(result.success);
        assert(result.trades.empty());

        // Se first manteve prioridade, market sell deve bater nela.
        std::vector<Trade> trades =
            engine.submit_market_order(
                Side::Sell,
                25
            );

        assert(trades.size() == 1);
        assert(trades[0].buy_order_id == first.order_id);
        assert(trades[0].quantity == 25);

        std::cout << "\nModify quantity decrease test passed!\n";
    }

    // TEST 5 - Modify: quantity increase loses priority
    {
        MatchingEngine engine;

        SubmissionResult first =
            engine.submit_limit_order(
                Side::Buy,
                1000,
                100
            );

        SubmissionResult second =
            engine.submit_limit_order(
                Side::Buy,
                1000,
                200
            );

        ModificationResult result =
            engine.modify_order(
                first.order_id,
                1000,
                150
            );

        assert(result.success);

        // first perdeu prioridade.
        // second agora deve ser executada primeiro.
        std::vector<Trade> trades =
            engine.submit_market_order(
                Side::Sell,
                50
            );

        assert(trades.size() == 1);
        assert(trades[0].buy_order_id == second.order_id);

        std::cout << "\nModify quantity increase test passed!\n";
    }

    // TEST 6 - Modify price triggers matching
    {
        MatchingEngine engine;

        SubmissionResult buy =
            engine.submit_limit_order(
                Side::Buy,
                1000,
                100
            );

        SubmissionResult sell =
            engine.submit_limit_order(
                Side::Sell,
                1050,
                100
            );

        ModificationResult result =
            engine.modify_order(
                buy.order_id,
                1100,
                100
            );

        assert(result.success);

        assert(result.trades.size() == 1);
        assert(result.trades[0].price == 1050);
        assert(result.trades[0].quantity == 100);

        // Ordem resting era a sell @ 10.50
        assert(result.trades[0].buy_order_id == buy.order_id);
        assert(result.trades[0].sell_order_id == sell.order_id);

        std::cout << "\nModify price matching test passed!\n";
    }

    // TEST 7 - Modify missing order
    {
        MatchingEngine engine;

        ModificationResult result =
            engine.modify_order(
                999,
                1000,
                100
            );

        assert(!result.success);
        assert(result.trades.empty());

        std::cout << "\nModify order not found test passed!\n";
    }

    // TEST 8 - Peg to bid repricing and priority
    {
        MatchingEngine engine;

        SubmissionResult first =
            engine.submit_limit_order(
                Side::Buy,
                1000,
                200
            );

        SubmissionResult peg =
            engine.submit_pegged_order(
                PegReference::Bid,
                150
            );

        SubmissionResult better =
            engine.submit_limit_order(
                Side::Buy,
                1010,
                300
            );

        // Esperado em 10.10:
        //
        // peg (150) -> better (300)
        //
        // porque peg é mais antiga.

        std::vector<Trade> trades =
            engine.submit_market_order(
                Side::Sell,
                200
            );

        assert(trades.size() == 2);

        assert(trades[0].price == 1010);
        assert(trades[0].quantity == 150);
        assert(trades[0].buy_order_id == peg.order_id);

        assert(trades[1].price == 1010);
        assert(trades[1].quantity == 50);
        assert(trades[1].buy_order_id == better.order_id);

        std::cout << "\nPeg bid repricing test passed!\n";
    }

    // TEST 9 - Inactive peg becomes active
    {
        MatchingEngine engine;

        SubmissionResult peg =
            engine.submit_pegged_order(
                PegReference::Bid,
                150
            );

        // Não havia bid, então a peg está inativa.

        SubmissionResult bid =
            engine.submit_limit_order(
                Side::Buy,
                1000,
                200
            );

        // submit_limit_order chama update_all_pegged_orders(),
        // então agora a peg também deve estar em 10.00.

        std::vector<Trade> trades =
            engine.submit_market_order(
                Side::Sell,
                150
            );

        assert(trades.size() == 1);

        // A peg é mais antiga que a limit.
        assert(trades[0].buy_order_id == peg.order_id);
        assert(trades[0].price == 1000);
        assert(trades[0].quantity == 150);

        std::cout << "\nInactive peg activation test passed!\n";
    }

    // TEST 10 - Peg to offer repricing and priority
    {
        MatchingEngine engine;

        SubmissionResult first =
            engine.submit_limit_order(
                Side::Sell,
                1050,
                200
            );

        SubmissionResult peg =
            engine.submit_pegged_order(
                PegReference::Offer,
                150
            );

        SubmissionResult better =
            engine.submit_limit_order(
                Side::Sell,
                1040,
                300
            );

        // Peg deve sair de 10.50 e acompanhar 10.40.
        // Como é mais antiga que "better", fica na frente.

        std::vector<Trade> trades =
            engine.submit_market_order(
                Side::Buy,
                200
            );

        assert(trades.size() == 2);

        assert(trades[0].price == 1040);
        assert(trades[0].quantity == 150);
        assert(trades[0].sell_order_id == peg.order_id);

        assert(trades[1].price == 1040);
        assert(trades[1].quantity == 50);
        assert(trades[1].sell_order_id == better.order_id);

        std::cout << "\nPeg offer repricing test passed!\n";
    }

    std::cout << "\nAll MatchingEngine tests passed!\n";

    return 0;
}