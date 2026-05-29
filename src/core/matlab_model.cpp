// core/matlab_model.cpp (фрагмент)
#include "matlab_model.h"
#include <iostream>
#include <windows.h>  // или Windows: LoadLibrary

namespace bkd::core {

MatlabModel::MatlabModel(const std::string& dllPath, const std::string& modelName)
    : dllPath_(dllPath), name_(modelName) {}

MatlabModel::~MatlabModel() {
    if (dllHandle_) {
        dlclose(dllHandle_);
    }
}

bool MatlabModel::init() {
    dllHandle_ = dlopen(dllPath_.c_str(), RTLD_LAZY);
    if (!dllHandle_) {
        std::cerr << "Failed to load " << dllPath_ << ": " << dlerror() << std::endl;
        return false;
    }
    // Ожидаем, что DLL экспортирует функции:
    // int getNumInputs();
    // int getNumOutputs();
    // void step(double dt, const double* inputs, double* outputs);
    auto getNumInputs = (int(*)())dlsym(dllHandle_, "getNumInputs");
    auto getNumOutputs = (int(*)())dlsym(dllHandle_, "getNumOutputs");
    stepFunc_ = (void(*)(double, const double*, double*))dlsym(dllHandle_, "step");

    if (!getNumInputs || !getNumOutputs || !stepFunc_) {
        std::cerr << "Missing required symbols in DLL" << std::endl;
        return false;
    }
    numInputs_ = getNumInputs();
    numOutputs_ = getNumOutputs();
    return true;
}

void MatlabModel::step(double dt, const double* inputs, double* outputs) {
    if (stepFunc_) stepFunc_(dt, inputs, outputs);
}

} // namespace
