#ifndef ORDER_NODE_HPP
#define ORDER_NODE_HPP

#include "order.hpp"

class PriceLevel;  // Indica para o compilador que existe uma classe chamada PriceLevel e que ela será definida em outro lugar

struct OrderNode
{
    Order order;
    OrderNode *previous, *next;
    PriceLevel *level;

    OrderNode(const Order &order)  // const indica que esse método não altera o objeto/parâmetro recebido
    {
        this->order = order;
        this->previous = nullptr;
        this->next = nullptr;
        this->level = nullptr;
    }
};


#endif