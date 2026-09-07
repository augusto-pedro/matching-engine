#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include <vector>

#include "order_book.hpp"
#include "trade.hpp"
#include "submission_result.hpp"

class MatchingEngine
{
private:
    OrderBook book;
    unsigned long long next_order_id, next_priority;

    std::vector<Trade> match_order(OrderNode *incoming);  // tenta executar a ordem contra as ordens do lado oposto, essas execuções são guardadas em um vetor de Trade

public:
    MatchingEngine();
    
    SubmissionResult submit_limit_order(Side side, int price, int quantity);  // esse método cria uma ordem, tenta executá-la contra o book e, se sobrar quantidade, coloca o restante no book (além de retornar o ID da ordem e a lista de trades que ela gerou)

    std::vector<Trade> submit_market_order(Side side, int quantity);  // tenta executar a ordem contra o book e, se sobrar quantidade ou não, a destrói em seguida

    void print_book();

    bool cancel_order(unsigned long long id);
};

#endif