#include "order_book.hpp"

OrderBook::OrderBook() {};  // não precisa iniciar porque os hash maps já são criados vazios por padrão

OrderBook::~OrderBook()
{
    for(auto it = this->orders_by_id.begin(); it != this->orders_by_id.end(); it++)
    {
        delete it->second;
    }
}

OrderNode *OrderBook::create_order(const Order &order)
{
    OrderNode *node = new OrderNode(order);

    this->orders_by_id[node->order.id] = node;

    return node;
}

OrderNode *OrderBook::find_order(unsigned long long id)  // return this->orders_by_id[id]; não serve porque quando fazemos map[id], o map associa id a nullptr caso id não exista
{
    auto it = this->orders_by_id.find(id);  // auto é apenas uma forma de não precisar por o tipo explícito, porque nesse caso seria std::unordered_map<unsigned long long, OrderNode*>::iterator

    if (it == orders_by_id.end())
    {
        return nullptr;
    }

    return it->second;
}

PriceLevel *OrderBook::best_bid()  // nosso mapa é o preço e o PriceLevel (cabeça da fila), como essa função devolve um ponteiro para fila, precisamos usar & pois retorna diretamente o endereço
{
    if(this->bids.empty())
    {
        return nullptr;
    }
    return &(this->bids.begin()->second);  // pega o mapa, pega a primeira dupla, pega o segundo membro da primeira dupla e passa o endereço disso
}

PriceLevel *OrderBook::best_offer()
{
    if(this->offers.empty())
    {
        return nullptr;
    }

    return &(this->offers.begin()->second);  // pega o mapa, pega a primeira dupla, pega o segundo elemento da primeira dupla e passa o endereço disso
}

/*void OrderBook::add_to_book(OrderNode *node)
{
    if(node->order.side == Side::Buy)
    {
        this->bids[node->order.price] = node->level;
    }
}*/