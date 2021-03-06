#pragma once

#include <gmock/gmock.h>
#include <fw/numerical/IBasisEvaluator.hpp>

class IBasisEvaluatorMock:
    public fw::IBasisEvaluator
{
public:
    MOCK_CONST_METHOD1(evaluate, std::vector<double>(
        double parameter
    ));
};
