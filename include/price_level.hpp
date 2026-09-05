#ifndef PRICE_LEVEL_HPP
#define PRICE_LEVEL_HPP

#include "order_node.hpp"

class PriceLevel
{
private:
    int size, price;
    OrderNode *head, *tail;
public:
    PriceLevel(int price);

    int get_price() const;  // const indica que esse método não altera os atributos do objeto
    int get_size() const;

    OrderNode *front() const;  // retorna a cebeça da lista

    bool empty() const;  // indica se esse nível de preço está vazio

    void append(OrderNode *node);  // adiciona um novo nó ao final da lista
    void detach(OrderNode *node);  // destaca um nó da lista sem deletar
    void insert_by_priority(OrderNode *node);  // adiciona um nó em qualquer lugar da lista com base em sua prioridade
};

#endif