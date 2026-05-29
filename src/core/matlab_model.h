// core/matlab_model.h
#pragma once
#include "imodel.h"
#include <string>

namespace bkd::core {

class MatlabModel : public IModel {
public:
    MatlabModel(const std::string& dllPath, const std::string& modelName);
    ~MatlabModel() override;

    bool init() override;
    void step(double dt, const double* inputs, double* outputs) override;
    size_t getNumInputs() const override { return numInputs_; }
    size_t getNumOutputs() const override { return numOutputs_; }
    std::string getName() const override { return name_; }

private:
    std::string dllPath_;
    std::string name_;
    void* dllHandle_ = nullptr;
    void (*stepFunc_)(double dt, const double* inputs, double* outputs) = nullptr;
    size_t numInputs_ = 0;
    size_t numOutputs_ = 0;
};

} // namespace
