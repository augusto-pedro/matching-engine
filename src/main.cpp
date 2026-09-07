#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>

#include "matching_engine.hpp"

bool parser_side(const std::string &text, Side &side)  // converte um texto para compra ou venda. Recebe um texto e se esse texto for buy ou sell, a variável side receberá Side::Buy ou Side::sell
{
    if(text == "buy")
    {
        side = Side::Buy;

        return true;
    }

    if(text == "sell")
    {
        side = Side::Sell;

        return true;
    }

    return false;
}

bool parser_price(const std::string &text, int &price)  // converte um texto para um preço em centavos
{
    std::size_t dot;  // é um tipo inteiro e sem sinal usado para representar tamanhos, quantidades e índices
    std::string whole, decimal;

    dot = text.find('.');  // .find() procura o caractere dentro da string e retorna a posição/índice onde ele foi enocntrado

    if(dot == std::string::npos)  // std::string::npos é um valor especial que rpesenta "não foi encontrado"
    {
        whole = text;  // valor dos reais sem centavos
        decimal = "";  // valor dos centavos (nesse caso não tem)
    }
    else
    {
        if(text.find('.', dot + 1) != std::string::npos)  // .find('.', dot + 1) manda procurar outro ponto, depois/a partir da posição do primiero (dot + 1), a intenção é não achar logo == std::string::npos
        {
            return false;  // mais de 1 '.' na string
        }

        whole = text.substr(0, dot);  // .substr() é para pegar a substring, aqui pegamos de [0; dot)
        decimal = text.substr(dot + 1);  // aqui pegamos a substring de [dot + 1; infinito)
    }

    if(whole.empty() || decimal.size() > 2)  // se não tem parte inteira ou se o tamanho da decimal foi maior que 2 (ex: .55 ou 23.4568)
    {
        return false;
    }

    for(int i = 0; i < whole.size(); i++)  // percorre a string
    {
        if(whole[i] < '0' || whole[i] > '9')  // verifica se a parte inteira é número
        {
            return false;
        }
    }

    for(int i = 0; i < decimal.size(); i++)
    {
        if(decimal[i] < '0' || decimal[i] > '9')
        {
            return false;
        }
    }

    long long whole_value = std::stoll(whole);  // converte a string para um long long. stoll é [S]tring [TO] [L]ong [L]ong

    int decimal_value = 0;

    if(decimal.size() == 1)
    {
        decimal_value = (decimal[0] - '0') * 10;  // pegamos o decimal em número. Pegamos quantas unidades ele está depois de '0' na tabela ascii, isso gera o número dele em inteiro, depois é só multiplicar por 10
    }
    if(decimal.size() == 2)
    {
        decimal_value = (decimal[0] - '0') * 10 + (decimal[1] - '0');
    }

    long long total = (whole_value * 100) + decimal_value;

    if(total <= 0 || total > std::numeric_limits<int>::max())  // verifica se o total é negativo ou se o número passa da capacidade máxima de um int (verificamos isso porque faremos um cast)
    {
        return false;
    }

    price = static_cast<int>(total);  // faz um cast (transforma long long em int)

    return true;
}

bool parser_quantity(const std::string &text, int &quantity)  // converte um texto para uma quantidade
{
    if(text.empty())
    {
        return false;
    }

    for(int i = 0; i < text.size(); i++)
    {
        if(text[i] < '0' || text[i] > '9')
        {
            return false;
        }
    }

    long long value = std::stoll(text);

    if(value <= 0 || value > std::numeric_limits<int>::max())
    {
        return false;
    }

    quantity = static_cast<int>(value);

    return true;
}

bool parser_order(const std::string &text, unsigned long long &id)
{
    const std::string prefix = "order_";

    if(text.compare(0, prefix.size(), prefix) != 0)  // string1.compare(começo, fim, string2) compara a string1 desde o elemento na posição "começo" até "fim" com a "string2" e retorna 0 se forem iguais
    {
        return false;
    }

    std::string number = text.substr(prefix.size());  // pega o número da ordem, por meio da sub string que começa em text[prefix.size()] e vai até o final
    
    if(number.empty())
    {
        return false;
    }

    for(int i = 0; i < number.size(); i++)
    {
        if(number[i] < '0' || number[i] > '9')
        {
            return false;
        }
    }

    id = std::stoull(number);  // converte a string para um unsigned long long. stoull é [S]tring [TO] [U]nsigned [L]ong [L]ong

    return id > 0;  // se id > 0 retorna true
}

