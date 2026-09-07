#include "utils.hpp"

#include <iomanip>
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