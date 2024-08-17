#ifndef SYSY_COMPILER_SYTYPE_H
#define SYSY_COMPILER_SYTYPE_H

const auto SyInt = "Int";
const auto SyVoid = "Void";
const auto SyFloat = "Float";


class SyType : public std::string {
public:

    SyType() : std::string() {}

    SyType(const std::string &str) : std::string(str) {}

    SyType(const char *str) : std::string(str) {}

    bool is_array() const {
        return this->at(0) == '[';
    }

    bool is_primitive() const {
        return *this == SyInt || *this == SyFloat;
    }

    SyType deref() {
        assert(this->at(0) == 'L');

        if (this->at(0) == 'L') return this->substr(1);
        else throw std::runtime_error("Not an lval type");
    }
};

#endif //SYSY_COMPILER_SYTYPE_H
