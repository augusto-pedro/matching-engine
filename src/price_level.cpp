#include "price_level.hpp"

PriceLevel::PriceLevel(int price)
{
    this->price = price;
    this->size = 0;
    this->head = nullptr;
    this->tail = nullptr;
}

int PriceLevel::get_price() const
{
    return this->price;
}

int PriceLevel::get_size() const
{
    return this->size;
}

OrderNode *PriceLevel::front() const
{
    return this->head;
}

bool PriceLevel::empty() const
{
    return this->size == 0;
}

void PriceLevel::append(OrderNode *node)
{
    node->previous = this->tail;

    if(this->tail == nullptr)
    {
        this->head = node;
        this->tail = node;
    }
    else
    {
        this->tail->next = node;
        this->tail = node;
    }

    node->next = nullptr;
    node->level = this;
    this->size++;
}

void PriceLevel::detach(OrderNode *node)
{
    if(node->next == nullptr && node->previous == nullptr)
    {
        head = nullptr;
        tail = nullptr;
    }
    else if(node->next == nullptr && node->previous != nullptr)
    {
        this->tail = node->previous;
        this->tail->next = nullptr;
        node->previous = nullptr;
    }
    else if(node->next != nullptr && node->previous == nullptr)
    {
        this->head = node->next;
        this->head->previous = nullptr;
        node->next = nullptr;
    }
    else
    {
        node->previous->next = node->next;
        node->next->previous = node->previous;
        node->next = nullptr;
        node->previous = nullptr;
    }

    node->level = nullptr;
    this->size--;
}

void PriceLevel::insert_by_priority(OrderNode *node)
{
    if(this->head == nullptr)
    {
        append(node);
        return;
    }

    OrderNode *current = this->head;  // não usamos new porquê não queremos criar um novo nó na memória, queremos apenas criar um ponteiro que aponta para um nó já existente

    while(current != nullptr && node->order.priority >= current->order.priority)
    {
        current = current->next;
    }

    if(current == nullptr)
    {
        append(node);
        return;
    }

    if(current->previous == nullptr)
    {
        current->previous = node;
        node->next = current;
        node->previous = nullptr;
        this->head = node;
        node->level = this;
        this->size++;
        return;
    }

    current->previous->next = node;
    node->previous = current->previous;
    node->next = current;
    current->previous = node;
    node->level = this;
    this->size++;
}