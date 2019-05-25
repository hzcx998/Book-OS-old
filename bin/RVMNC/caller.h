#pragma once
#include <vector>
enum CallerType {
    RegisterProgram,
    DeleteProgram,

};
struct CallMessage {
    bool Empty;
    CallerType DataType;
    vector<void*>* datas;
};


