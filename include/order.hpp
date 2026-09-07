#ifndef ORDER_HPP
#define ORDER_HPP

enum class Side  // cria um tipo de objeto (Side) que só pode assumir um conjunto fechado de valores (Buy e Sell)
{
    Buy, Sell
};

enum class OrderType
{
    Limit, Market, Pegged
};

enum class PegReference
{
    None, Bid, Offer
};

struct Order  // Order está sendo feita como struct e não class porque em struct os membros são públicos por padrão. Como order é apenas um conjunto de dados achei mais prudente usar struct
{
    int price, quantity;
    unsigned long long id, priority;
    Side side;
    OrderType type;
    PegReference peg_reference;

    Order(){}
    
    Order
    (
        unsigned long long id,
        OrderType type,
        Side side,
        int price,
        int quantity,
        unsigned long long priority,
        PegReference peg_reference
    )
    {
        this->id = id;
        this->type = type;
        this->side = side;
        this->price = price;
        this->quantity = quantity;
        this->priority = priority;
        this->peg_reference = peg_reference;
    }
};

#endif