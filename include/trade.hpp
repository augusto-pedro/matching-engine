#ifndef TRADE_HPP
#define TRADE_HPP

struct Trade
{
    int price, quantity;
    unsigned long long buy_order_id, sell_order_id;

    Trade(int price, int quantity, unsigned long long buy_order_id, unsigned long long sell_order_id)
    {
        this->price = price;
        this->quantity = quantity;
        this->buy_order_id = buy_order_id;
        this->sell_order_id = sell_order_id;
    }
};

#endif