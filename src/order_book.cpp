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
    auto it = this->orders_by_id.find(id);  // auto é apenas uma forma de não precisar por o tipo explícito, porque nesse caso seria std::unordered_map<unsigned long long, OrderNode*>::iterator  // it é um iterador que APONTA para a dupla desejada, caso exista

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

void OrderBook::add_to_book(OrderNode *node)
{
    int price = node->order.price;
    Side side = node->order.side;

    if(side == Side::Buy)
    {
        auto result = this->bids.try_emplace(price, price);  // try_emplace(a, b) é um método que mapas possuem que, caso a chave "a" não exista, ele cria um elemento com a chave sendo "a" e o "conteúdo" sendo "b", no caso "b" não é o conteúdo mas sim é passado como argumento para o PriceLevel, pois o mapa é chave sendo um int price e valor sendo um PriceLevel(int price). caso a chave "a" exista, ele não faz nada. result contém duas informações, result.first() é um iterador que APONTA para a dupla chave <-> valor gerada/encontrada e result.second() contém a informação se a dupla foi criada agora ou se já existia
        result.first->second.append(node);
    }
    else
    {
        auto result = this->offers.try_emplace(price, price);
        result.first->second.append(node);
    }
}

void OrderBook::add_to_book_by_priority(OrderNode *node)
{
    int price = node->order.price;
    Side side = node->order.side;

    if(side == Side::Buy)
    {
        auto result = this->bids.try_emplace(price, price);
        result.first->second.insert_by_priority(node);
    }
    else
    {
        auto result = this->offers.try_emplace(price, price);
        result.first->second.insert_by_priority(node);
    }
}

void OrderBook::detach(OrderNode *node)
{
    if(node == nullptr || node->level == nullptr)
    {
        return;
    }

    int price = node->order.price;
    Side side = node->order.side;
    
    PriceLevel *level = node->level;

    level->detach(node);

    if(level->empty())
    {
        if(side == Side::Buy)
        {
            this->bids.erase(price);
        }
        else
        {
            this->offers.erase(price);
        }
    }
}

void OrderBook::delete_order(OrderNode *node)
{
    if(node == nullptr)
    {
        return;
    }

    this->detach(node);  // tira a ordem do book
    
    this->orders_by_id.erase(node->order.id);  // tira a ordem do índice de IDs

    delete node;  // destroi a ordem
}