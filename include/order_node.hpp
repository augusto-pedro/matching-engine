#ifndef ORDER_NODE_HPP
#define ORDER_NODE_HPP

#include "order.hpp"

class PriceLevel;  // Indica para o compilador que existe uma classe chamada PriceLevel e que ela será definida em outro lugar (é necessário porque não estou fazendo #include "price_level.hpp")

struct OrderNode
{
    Order order;
    OrderNode *previous, *next;
    PriceLevel *level;

    OrderNode(const Order &order)  // const indica que esse método não altera o objeto/parâmetro recebido  // usamos o endereço de order (&order) porque queremos pegar a order original e não uma cópia (order) ou um ponteiro para order (*order)
    {
        this->order = order;
        this->previous = nullptr;
        this->next = nullptr;
        this->level = nullptr;
    }
};

#endif