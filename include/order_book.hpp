#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#include "price_level.hpp"

class OrderBook
{
private:
    std::map<int, PriceLevel, std::greater<int>> bids;  // comporá o livro de compras, está organizado do maior para o menor preço
    std::map<int, PriceLevel> offers;  // comporá o livro de vendas, estpa organizado do menor para o maior preço

    std::unordered_map<unsigned long long, OrderNode*> orders_by_id;  // mais otimizado para achar uma ordem específica, sem ficar procurando em tudo

public:
    OrderBook();
    ~OrderBook();

    OrderNode *create_order(const Order &order);  // cria um nó de ordem, adiciona esse nó no mapa de ids e retorna o nó
    OrderNode *find_order(unsigned long long id);

    void add_to_book(OrderNode *node);  // 
    void add_to_book_by_priority(OrderNode *node);

    void detach(OrderNode *node);  // apenas destaca a ordem mas não a destroi
    void delete_order(OrderNode *node);  // destaca, tira do índice de ordens e destroi

    PriceLevel *best_bid();
    PriceLevel *best_offer();

    void print_book();
    void print_book_aggregated();

    std::vector<OrderNode*> get_pegged_orders(PegReference reference);  // pega todas as ordens pegged fixadas a uma referência
};

#endif