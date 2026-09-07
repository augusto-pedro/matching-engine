#include "matching_engine.hpp"

#include <algorithm>

MatchingEngine::MatchingEngine()
{
    this->next_order_id = 1;
    this->next_priority = 1;
}

SubmissionResult MatchingEngine::submit_limit_order(Side side, int price, int quantity)  // esse método cria uma ordem, tenta executá-la contra o book e, se sobrar quantidade, coloca o restante no book (além de retornar o ID da ordem e a lista de trades que ela gerou)
{
    unsigned long long id = this->next_order_id++, priority = this->next_priority++;  // cria o id único e a prioridade temporal da nova ordem. Nesse caso o a = b++ joga o valor de 'b' em 'a' e depois incrementa 'b'

    Order order(id, OrderType::Limit, side, price, quantity, priority, PegReference::None);  // cria a ordem

    OrderNode *node = this->book.create_order(order);  // cria o nó da fila de ordens

    std::vector<Trade> trades = this->match_order(node);  // tenta executar a ordem contra as ordens do lado oposto, essas execuções são guardadas em trades

    if(node->order.quantity > 0)
    {
        this->book.add_to_book(node);  // se sobrar adiociona ao book o restante
    }
    else
    {
        this->book.delete_order(node);  // se não sobrar quantidade, deleta a ordem
    }

    return SubmissionResult(id, trades);  // retorna o ID da ordem e a lista de trades que ela gerou
}

std::vector<Trade> MatchingEngine::submit_market_order(Side side, int quantity)
{
    unsigned long long id = this->next_order_id++, priority = this->next_priority++;

    Order order(id, OrderType::Market, side, 0, quantity, priority, PegReference::None);  // criamos a ordem com preço 0 porque como é Market, e o algoritmo sabe disso, o preço não influencia

    OrderNode *node = this->book.create_order(order);

    std::vector<Trade> trades = this->match_order(node);

    this->book.delete_order(node);

    return trades;
}

std::vector<Trade> MatchingEngine::match_order(OrderNode *incoming)  // tenta executar a ordem contra as ordens do lado oposto, essas execuções são guardadas em um vetor de Trade
{
    std::vector<Trade> trades;

    while(incoming->order.quantity > 0)
    {
        PriceLevel *opposite_level = nullptr;  // irá pegar a melhor orfeta do lado oposto

        if(incoming->order.side == Side::Buy)  // caso a ordem seja de compra, irá verificar se existe uma melhor ordem do lado oposto e se o preço bate
        {
            opposite_level = this->book.best_offer();

            if(opposite_level == nullptr)
            {
                break;
            }

            if(incoming->order.type == OrderType::Limit && incoming->order.price < opposite_level->get_price())
            {
                break;
            }
        }
        else  // caso a ordem seja de venda, irá verificar se existe uma melhor ordem do lado oposto e se o preço bate
        {
            opposite_level = this->book.best_bid();

            if(opposite_level == nullptr)
            {
                break;
            }

            if(incoming->order.type == OrderType::Limit && incoming->order.price > opposite_level->get_price())
            {
                break;
            }
        }

        OrderNode *resting = opposite_level->front();  // pega a melhor ordem do lado oposto

        int trade_quantity = std::min(incoming->order.quantity, resting->order.quantity);
        int trade_price = resting->order.price;
        unsigned long long buy_id, sell_id;

        if(incoming->order.side == Side::Buy)  // pega os ids das ordens de compra e venda
        {
            buy_id = incoming->order.id;
            sell_id = resting->order.id;
        }
        else  // pega os ids das ordens de compra e venda
        {
            buy_id = resting->order.id;
            sell_id = incoming->order.id;
        }

        trades.push_back(Trade(trade_price, trade_quantity, buy_id, sell_id));  // cria o objeto Trade e adiciona ao nosso vetor de trades

        incoming->order.quantity = incoming->order.quantity - trade_quantity;
        resting->order.quantity = resting->order.quantity - trade_quantity;

        if(resting->order.quantity == 0)  // caso a ordem tenha se esgotado ela é deletada e depois o loop segue para a próxima enquanto houver demanda e oferta
        {
            this->book.delete_order(resting);
        }
    }

    return trades;
}

void MatchingEngine::print_book()
{
    this->book.print_book();
}