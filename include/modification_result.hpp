#ifndef MODIFICATION_RESULT_HPP
#define MODIFICATION_RESULT_HPP

#include <vector>

#include "trade.hpp"

struct ModificationResult  // estrutura auxiliar para agrupar o resultado de uma modificação de ordem, basicamente nos diz qual foi o resultado de modificar uma ordem (que é o sucesso/falha + trades produzidos)
{
    bool success;
    std::vector<Trade> trades;

    ModificationResult(){};

    ModificationResult(bool success, std::vector<Trade> trades)
    {
        this->success = success;
        this->trades = trades;
    }
};

#endif