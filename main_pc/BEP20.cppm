module;

#include <string>

export module BEP20;

export class BEP20 {
    std::string address;
public:
    explicit BEP20(std::string address) : address(std::move(address)) {}


};