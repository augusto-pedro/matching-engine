#include "order_book.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

std::string format_price(int price)  // pega um preço armazenado como inteiro e transforma em uma std::string no formato monetário com duas casas decimais
{
    std::ostringstream output;  // cria uma espécie de cout, só que em vez de imprimir na tela, ele monta uma "string" e armazena

    output << price / 100            // pega os reais
           << '.'                    // separador decimal
           << std::setw(2)           // diz que o próximo valor inserido deve ocupar pelo menos 2 caracteres
           << std::setfill('0')      // diz que, se faltar espaço para completar essa largura, esse espaço deve ser preenchido com 0
           << price % 100;           // pega os centavos
    
    return output.str();  // .str() transforma o conteúdo em uma std::string
}

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

void OrderBook::print_book()
{
    std::vector<std::string> buy_rows;  // linhas de compras
    std::vector<std::string> sell_rows;  // linhas de vendas

    for(auto it_level = this->bids.begin(); it_level != this->bids.end(); it_level++)  // percorre toda a coluna de compras
    {
        OrderNode *node = it_level->second.front();

        while(node != nullptr)  // percorre todas as ordens de compra de um determinado preço
        {
            std::string row = std::to_string(node->order.quantity) + " @ " + format_price(node->order.price);  // cria uma string no formato "qty @ price"

            buy_rows.push_back(row);  // coloca a string criada no nosso vetor de strings

            node = node->next;  // vai para a próxima ordem do mesmo preço
        }
    }

    for(auto it_level = this->offers.begin(); it_level != this->offers.end(); it_level++)  //percorre toda a coluna de vendas
    {
        OrderNode *node = it_level->second.front();

        while(node != nullptr)  // percorre todas as ordens de venda de um determinado preço
        {
            std::string row = std::to_string(node->order.quantity) + " @ " + format_price(node->order.price);  // cria uma string no formato "qty @ price"

            sell_rows.push_back(row);  // coloca a string criada no nosso vetor de strings

            node = node->next;  // vai para a próxima ordem do mesmo preço
        }
    }

    std::cout << "Ordens de Compra    | Ordens de Venda    \n";
    std::cout << "--------------------|--------------------\n";

    unsigned long long rows = (buy_rows.size() > sell_rows.size() ? buy_rows.size() : sell_rows.size());  // operador ternário, ele diz que, se tiverem mais linhas de compra, rows receberá esse número de linhas, caso contrário recebá o número de linhas de vendas

    for(unsigned long long i = 0; i < rows; i++)
    {
        if(i < buy_rows.size())
        {
            std::cout << std::left << std::setw(20) << buy_rows[i];  // std::left alinha o próximo texto à esquerda e std::setw(20) reserva uma largura de 20 caracteres para o próximo valor impresso
        }
        else
        {
            std::cout << std::setw(20) << "";
        }

        std::cout << "| ";

        if(i < sell_rows.size())
        {
            std::cout << sell_rows[i];  // aqui não precisa de std::left porque uma vez configurado ele só muda se mudarmos explicitamente, já std::setw(20) serve apenas para a próxima palavra, logo deve ser configurado em todo std::cout que se queira usar isso
        }

        std::cout << "\n";
    }
}

void OrderBook::print_book_aggregated()
{
    std::vector<std::string> buy_rows;  // linhas de compras
    std::vector<std::string> sell_rows;  // linhas de vendas

    for(auto it = this->bids.begin(); it != this->bids.end(); it++)
    {
        int total_quantity = 0;

        OrderNode *node = it->second.front();

        while(node != nullptr)
        {
            total_quantity = total_quantity + node->order.quantity;

            node = node->next;
        }

        std::string row = std::to_string(total_quantity) + " @ " + format_price(it->first);

        buy_rows.push_back(row);
    }

    for(auto it = this->offers.begin(); it != this->offers.end(); it++)
    {
        int total_quantity = 0;

        OrderNode *node = it->second.front();

        while(node != nullptr)
        {
            total_quantity = total_quantity + node->order.quantity;

            node = node->next;
        }

        std::string row = std::to_string(total_quantity) + " @ " + format_price(it->first);

        sell_rows.push_back(row);
    }

    std::cout << "Ordens de Compra    | Ordens de Venda    \n";
    std::cout << "--------------------|--------------------\n";

    unsigned long long rows = (buy_rows.size() > sell_rows.size() ? buy_rows.size() : sell_rows.size());

    for(unsigned long long i = 0; i < rows; i++)
    {
        if(i < buy_rows.size())
        {
            std::cout << std::left << std::setw(20) << buy_rows[i];
        }
        else
        {
            std::cout << std::setw(20) << "";
        }

        std::cout << "| ";

        if(i < sell_rows.size())
        {
            std::cout << sell_rows[i];
        }

        std::cout << "\n";
    }
}

std::vector<OrderNode*> OrderBook::get_pegged_orders(PegReference reference)
{
    std::vector<OrderNode*> pegged_orders;

    for(auto it = this->orders_by_id.begin(); it != this->orders_by_id.end(); it++)
    {
        OrderNode *node = it->second;

        if(node->order.type == OrderType::Pegged && node->order.peg_reference == reference)
        {
            pegged_orders.push_back(node);
        }
    }

    return pegged_orders;
}