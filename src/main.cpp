#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>

#include "matching_engine.hpp"
#include "utils.hpp"

bool parser_side(const std::string &text, Side &side);  // converte um texto para compra ou venda. Recebe um texto e se esse texto for buy ou sell, a variável side receberá Side::Buy ou Side::sell

bool parser_price(const std::string &text, int &price);  // converte um texto para um preço em centavos

bool parser_quantity(const std::string &text, int &quantity);  // converte um texto para uma quantidade

bool parser_order(const std::string &text, unsigned long long &id);  // converte um texto para o ID de uma ordem

void print_trades(const std::vector<Trade> &trades);  // recebe um vetor de trades imprime todos os trades feitos, agregando os que tem o mesmo valor 

int main()
{
    MatchingEngine engine;
    std::string line;

    std::cout << "Matching Engine\n";
    std::cout << "Type 'help' for available commands\n\n";

    while(true)
    {
        std::cout << ">>> ";

        if(!std::getline(std::cin, line))  // std::getline(std::cin, line) lê uma linha inteira digitada pelo usuário e salva em line
        {
            break;
        }

        std::cout << "\n";

        std::istringstream input(line);  // pega a linha inteira digitada pelo usuário e permite lê-la "por pedaços" separados por espaços (input se comporta como se a string fosse uma entrada de teclado)

        std::string command;  // cria uma string chamda command

        input >> command;  // pega a primeira palavra de input e coloca em command

        if(command.empty())
        {
            continue;
        }

        if(command == "exit")
        {
            break;
        }

        if(command == "help")
        {
            std::cout 
                << "limit buy <price> <qty>\n"                         //falta aquele detalhe
                << "limit sell <price> <qty>\n"                        //falta aquele detalhe
                << "market buy <qty>\n"                                //ok
                << "market sell <qty>\n"                               //ok
                << "peg bid buy <qty>\n"                               //ok
                << "peg offer sell <qty>\n"                            //ok
                << "cancel order <order_id>\n"                         //ok
                << "modify order <order_id> <price> <qty>\n"           //ok
                << "modify peg <order_id> <qty>\n"                     //
                << "print book\n"                                      //ok
                << "print book aggregated\n"                           //ok
                << "exit\n\n";                                         //ok
            
            continue;
        }

        if(command == "limit")
        {
            std::string side_text, price_text, quantity_text;

            input >> side_text >> price_text >> quantity_text;  // pega a segunda, terceira e quarta palavra de input e coloca nessas strings
            
            Side side;
            int price, quantity;

            if(!parser_side(side_text, side) || !parser_price(price_text, price) || !parser_quantity(quantity_text, quantity))
            {
                std::cout << "Invalid command\n\n";

                continue;
            }

            try
            {
                SubmissionResult result = engine.submit_limit_order(side, price, quantity);

                std::cout
                    << "Order created: "
                    << side_text << " "
                    << quantity
                    << " @ "
                    << format_price(price)
                    << " order_"
                    << result.order_id
                    << "\n\n";
            }
            catch(const std::exception &error)
            {
                std::cout << "Error: " << error.what() << "\n\n";
            }

            continue;
        }

        if(command == "print")
        {
            std::string what, option;

            input >> what;

            if(what != "book")
            {
                std::cout << "Invalid command\n\n";
                continue;
            }

            input >> option;

            if(option.empty())
            {
                engine.print_book();
            }
            else if(option == "aggregated")
            {
                engine.print_book_aggregated();
            }
            else
            {
                std::cout << "Invalid command\n";
            }

            std::cout << "\n";

            continue;
        }

        if(command == "market")
        {
            std::string side_text, quantity_text;
            Side side;
            int quantity;

            input >> side_text >> quantity_text;

            if(!parser_side(side_text, side) || !parser_quantity(quantity_text, quantity))
            {
                std::cout << "Invalis command\n";

                continue;
            }

            try
            {
                std::vector<Trade> trades;

                trades = engine.submit_market_order(side, quantity);

                print_trades(trades);

                std::cout << "\n";
            }
            catch(const std::exception &error)
            {
                std::cout << "Error: " << error.what() << "\n\n";
            }

            continue;
        }

        if(command == "cancel")
        {
            std::string word, id_text;
            unsigned long long id;

            input >> word >> id_text;

            if(word != "order" || !parser_order(id_text, id))
            {
                std::cout << "Invalid command\n\n";

                continue;
            }

            if(engine.cancel_order(id))
            {
                std::cout << "Order cancelled\n\n";
            }
            else
            {
                std::cout << "Order not found\n\n";
            }

            continue;
        }

        if(command == "modify")
        {
            std::string type;

            input >> type;

            if(type == "order")
            {
                std::string id_text, price_text, quantity_text;
                unsigned long long id;
                int price, quantity;

                ModificationResult result;

                input >> id_text >> price_text >> quantity_text;

                if(!parser_order(id_text, id) || !parser_price(price_text, price) || !parser_quantity(quantity_text, quantity))
                {
                    std::cout << "Invalid command \n\n";

                    continue;
                }

                result = engine.modify_order(id, price, quantity);

                if(!result.success)
                {
                    std::cout << "Order not found\n\n";

                    continue;
                }

                std::cout << "Order modified\n";

                print_trades(result.trades);

                std::cout << "\n";

                continue;
            }
        }

        if(command == "peg")
        {
            std::string reference_text, side_text, quantity_text;
            PegReference reference;
            Side side;
            int quantity;
            SubmissionResult result;

            input >> reference_text >> side_text >> quantity_text;

            if(reference_text == "bid" && side_text == "buy")
            {
                reference = PegReference::Bid;
            }
            else if(reference_text == "offer" && side_text == "sell")
            {
                reference = PegReference::Offer;
            }
            else
            {
                std::cout << "Invalid command\n\n";
                continue;
            }

            if(!parser_side(side_text, side) || !parser_quantity(quantity_text, quantity))
            {
                std::cout << "Invalid command\n\n";
                continue;
            }

            try
            {
                result = engine.submit_pegged_order(reference, quantity);

                std::cout
                    << "Order created: peg "
                    << reference_text << " "
                    << side_text << " "
                    << quantity
                    << " order_"
                    << result.order_id
                    << "\n\n";
            }
            catch(const std::exception &error)
            {
                std::cout << "Error: " << error.what() << "\n\n";
            }

            continue;
        }

        std::cout << "Invalid command\n\n";
    }

    return 0;
}

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

bool parser_order(const std::string &text, unsigned long long &id)  // converte um texto para o ID de uma ordem
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

void print_trades(const std::vector<Trade> &trades)  // recebe um vetor de trades imprime todos os trades feitos, agregando os que tem o mesmo valor
{
    if(trades.empty())
    {
        return;
    }

    int current_price;
    long long total_quantity = 0;

    current_price = trades[0].price;

    for(auto it = trades.begin(); it != trades.end(); it++)  // percorre todos os trades do vetor e se o preço ficar constante vai somando as quantidades, a partir do momento que o preço muda, ele imprime os trades mostrando o preço "anterior" e a quantidade somada, depois disso atualiza com os novos valores e quantidades de trade
    {
        if(it->price == current_price)
        {
            total_quantity = total_quantity + it->quantity;
        }
        else
        {
            std::cout << "Trade, price: " << format_price(current_price) << ", qty: " << total_quantity << "\n";

            total_quantity = it->quantity;
            current_price = it->price;
        }
    }

    std::cout << "Trade, price: " << format_price(current_price) << ", qty: " << total_quantity << "\n";  // essa parte é necessária porque como o for percorre o vetor mas nós sempre imprimimos com base na "casa anterior do vetor", ou com base na "soma" das "casas anteriores do vetor"; acaba que a "posição atual" só é impressa na próxima iteração e assim por diente, logo, quando chegamos na "última casa", estamos imprimindo a penúltima ainda. Dito isso, a última não é impressa dentro do for, por isso ao sair dele temos que fazer essa última impressão
}