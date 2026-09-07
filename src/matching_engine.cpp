#include "matching_engine.hpp"

#include <algorithm>
#include <stdexcept>

MatchingEngine::MatchingEngine()
{
    this->next_order_id = 1;
    this->next_priority = 1;
}

SubmissionResult MatchingEngine::submit_limit_order(Side side, int price, int quantity)  // esse método cria uma ordem, tenta executá-la contra o book e, se sobrar quantidade, coloca o restante no book (além de retornar o ID da ordem e a lista de trades que ela gerou)
{
    if(price <= 0)
    {
        throw std::invalid_argument("Preço inválido");
    }

    if(quantity <= 0)
    {
        throw std::invalid_argument("Quantidade inválida");
    }

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

    this->update_all_pegged_orders();

    return SubmissionResult(id, trades);  // retorna o ID da ordem e a lista de trades que ela gerou
}

SubmissionResult MatchingEngine::submit_pegged_order(PegReference reference, int quantity)
{
    if (quantity < 0)
    {
        throw std::invalid_argument("Quantidade inválida");
    }

    if(reference != PegReference::Bid && reference != PegReference::Offer)
    {
        throw std::invalid_argument("Refernência da peg inválida");
    }
    
    unsigned long long id = this->next_order_id++, priority = this->next_priority++;
    Side side;
    PriceLevel *reference_level;
    int price = 0;

    if(reference == PegReference::Bid)
    {
        side = Side::Buy;
        reference_level = this->book.best_bid();
    }
    else
    {
        side = Side::Sell;
        reference_level = this->book.best_offer();
    }

    if(reference_level != nullptr)  // caso exista um nível de referência (book não vazio) a peg receberá esse preço
    {
        price = reference_level->get_price();
    }

    Order order(id, OrderType::Pegged, side, price, quantity, priority, reference);

    OrderNode *node = this->book.create_order(order);

    if(price > 0)  // se tudo deu certo ela é adicionada ao book, caso contrário teremos uma ordem na lista total de ordens mas fora do book por falta de referência
    {
        this->book.add_to_book(node);
    }

    return SubmissionResult(id, {});  // o vetor é vazio porque criar peg order não gera match
}

std::vector<Trade> MatchingEngine::submit_market_order(Side side, int quantity)
{
    if (quantity <= 0)
    {
        throw std::invalid_argument("Quantidade inválida");
    }
    
    unsigned long long id = this->next_order_id++, priority = this->next_priority++;

    Order order(id, OrderType::Market, side, 0, quantity, priority, PegReference::None);  // criamos a ordem com preço 0 porque como é Market, e o algoritmo sabe disso, o preço não influencia

    OrderNode *node = this->book.create_order(order);

    std::vector<Trade> trades = this->match_order(node);

    this->book.delete_order(node);

    this->update_all_pegged_orders();

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

            if(incoming->order.type != OrderType::Market && incoming->order.price < opposite_level->get_price())
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

            if(incoming->order.type != OrderType::Market && incoming->order.price > opposite_level->get_price())
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

bool MatchingEngine::cancel_order(unsigned long long id)
{
    OrderNode *node = this->book.find_order(id);

    if(node == nullptr)
    {
        return false;
    }

    this->book.delete_order(node);

    this->update_all_pegged_orders();

    return true;
}

ModificationResult MatchingEngine::modify_order(unsigned long long id, int new_price, int new_quantity)
{
    OrderNode *node = this->book.find_order(id);

    if(node == nullptr || node->order.type == OrderType::Pegged || new_price <= 0 || new_quantity <= 0)
    {
        return ModificationResult(false, {});
    }

    bool price_changed = (node->order.price != new_price);  // se o preço antigo é diferente do novo, então price mudou
    bool quantity_increased = (node->order.quantity < new_quantity);  // se a quantidade antiga é menor que a nova, então quantidade cresceu

    if(!price_changed && !quantity_increased)  // mesmo preço mas quantidade não aumentou (prioridade continua, nos outros não)
    {
        node->order.quantity = new_quantity;

        this->update_all_pegged_orders();

        return ModificationResult(true, {});
    }
    else if(!price_changed && quantity_increased)  // mesmo preço mas quantidade aumentou (perde prioridade mas não causa trade)
    {
        this->book.detach(node);

        node->order.quantity = new_quantity;
        node->order.priority = this->next_priority++;

        this->book.add_to_book(node);

        this->update_all_pegged_orders();

        return ModificationResult(true, {});
    }
    else if(price_changed && !quantity_increased)  // preço mudou e quantidade não aumentou (pode causar trade, lembrar que a quantidade não aumentar não significa que ela ficou constante)
    {
        this->book.detach(node);

        node->order.price = new_price;
        node->order.quantity = new_quantity;
        node->order.priority = this->next_priority++;

        std::vector<Trade> trades = this->match_order(node);

        if(node->order.quantity > 0)
        {
            this->book.add_to_book(node);
        }
        else
        {
            this->book.delete_order(node);
        }

        this->update_all_pegged_orders();

        return ModificationResult(true, trades);
    }
    else  // preço mudou e quantidade aumentou (pode causar trade)
    {
        this->book.detach(node);

        node->order.price = new_price;
        node->order.quantity = new_quantity;
        node->order.priority = this->next_priority++;

        std::vector<Trade> trades = this->match_order(node);

        if(node->order.quantity > 0)
        {
            this->book.add_to_book(node);
        }
        else
        {
            this->book.delete_order(node);
        }

        this->update_all_pegged_orders();

        return ModificationResult(true, trades);
    }
}

bool MatchingEngine::modify_pegged_order(unsigned long long id, int new_quantity)
{
    OrderNode *node = this->book.find_order(id);
    int old_quantity;
    bool was_active;

    if(node == nullptr || node->order.type != OrderType::Pegged || new_quantity <= 0)
    {
        return false;
    }

    old_quantity = node->order.quantity;
    was_active = (node->level != nullptr);  // se o a ordem está em algum nível do book, então ela está ativa

    if(new_quantity <= old_quantity)  // mantém prioridade e posição
    {
        node->order.quantity = new_quantity;

        return true;
    }

    if(was_active)  // perde a prioridade e posição
    {
        this->book.detach(node);
        node->order.priority = this->next_priority++;
        node->order.quantity = new_quantity;
        this->book.add_to_book(node);
    }
    else
    {
        node->order.priority = this->next_priority++;
        node->order.quantity = new_quantity;
    }

    return true;
}

void MatchingEngine::update_pegged_orders(PegReference reference)
{
    std::vector<OrderNode*> pegged_orders;
    PriceLevel *reference_level;
    int new_price;

    if(reference == PegReference::Bid)
    {
        reference_level = this->book.best_bid();
    }
    else
    {
        reference_level = this->book.best_offer();
    }

    if(reference_level == nullptr)  // se não existir preço de referência, nada acontece
    {
        return;
    }

    new_price = reference_level->get_price();

    pegged_orders = this->book.get_pegged_orders(reference);

    for(auto it = pegged_orders.begin(); it != pegged_orders.end(); it++)
    {
        OrderNode *node = *it;  // it é um objeto do tipo iterador do vector. Já *it é o conteúdo desse iterador (que no caso é um *node_atual)
    
        if(node->order.price == new_price)
        {
            continue;
        }

        this->book.detach(node);
        node->order.price = new_price;
        this->book.add_to_book_by_priority(node);
    }
}

void MatchingEngine::update_all_pegged_orders()
{
    this->update_pegged_orders(PegReference::Bid);
    this->update_pegged_orders(PegReference::Offer);
}