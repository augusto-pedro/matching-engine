#ifndef SUBMISSION_RESULT_HPP
#define SUBMISSION_RESULT_HPP

#include <vector>

#include "trade.hpp"

struct SubmissionResult  // estrutura auxiliar para agrupar o resultado de uma submissão de ordem, basicamente nos diz qual foi o resultado de enviar uma ordem para a engine (que é o ID criado + trades produzidos)
{
    unsigned long long order_id;
    std::vector<Trade> trades;

    SubmissionResult(){}

    SubmissionResult(unsigned long long order_id, std::vector<Trade> trades)
    {
        this->order_id = order_id;
        this->trades = trades;
    }
};

#endif