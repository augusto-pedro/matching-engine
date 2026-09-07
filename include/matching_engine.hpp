#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include <vector>

#include "order_book.hpp"
#include "trade.hpp"
#include "submission_result.hpp"
#include "modification_result.hpp"

class MatchingEngine
{
private:
    OrderBook book;
    unsigned long long next_order_id, next_priority;

    std::vector<Trade> match_order(OrderNode *incoming);  // tenta executar a ordem contra as ordens do lado oposto, essas execuções são guardadas em um vetor de Trade

    void update_pegged_orders(PegReference reference);

    void update_all_pegged_orders();

public:
    MatchingEngine();
    
    SubmissionResult submit_limit_order(Side side, int price, int quantity);  // esse método cria uma ordem, tenta executá-la contra o book e, se sobrar quantidade, coloca o restante no book (além de retornar o ID da ordem e a lista de trades que ela gerou)

    SubmissionResult submit_pegged_order(PegReference reference, int quantity);  // cria uma pegged order e a insere ao book caso exista um nível de referência, caso contrário ela entra com preço 0 na lista total de ordens e fica aguardando o livro não ficar vazio

    std::vector<Trade> submit_market_order(Side side, int quantity);  // tenta executar a ordem contra o book e, se sobrar quantidade ou não, a destrói em seguida

    void print_book();

    void print_book_aggregated();

    bool cancel_order(unsigned long long id);

    ModificationResult modify_order(unsigned long long id, int new_price, int new_quantity);

    bool modify_pegged_order(unsigned long long id, int new_quantity);
};

#endif