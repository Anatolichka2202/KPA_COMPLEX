// core/imodel.h
#pragma once
#include <string>
#include <vector>

namespace bkd::core {

// Описание портов модели (для отладки, можно использовать позже)
struct ModelPort {
    std::string name;
    double value;
};

class IModel {
public:
    virtual ~IModel() = default;

    // Инициализация: загрузка параметров, выделение памяти и т.д.
    virtual bool init() = 0;

    // Шаг модели с фиксированным количеством входов и выходов (обычно массивы double)
    // dt - шаг по времени в секундах
    // inputs - указатель на массив входных значений (размер getNumInputs())
    // outputs - указатель на массив выходных значений (размер getNumOutputs())
    virtual void step(double dt, const double* inputs, double* outputs) = 0;

    virtual size_t getNumInputs() const = 0;
    virtual size_t getNumOutputs() const = 0;
    virtual std::string getName() const = 0;
};

} // namespace
