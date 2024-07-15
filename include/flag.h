#pragma once

#ifndef FLAG_H
#define FLAG_H

#include "sysY.h"

#include <map>

class Flag {
private:

    std::map<std::string, std::variant<int, std::string, bool> > flags;

    /* 声明单例对象 */
    Flag() = default;
    
    ~Flag() = default;

    Flag(const Flag&) = delete;

    const Flag& operator=(const Flag&) = delete;
    
    static Flag& getFlagInstance();

public:

    static const Flag& getFlag() {
        return Flag::getFlagInstance();
    }

    static void init(int argc, const char** argv);

    template <typename T>
    const T& getFlagFor(const std::string& key) const {
        assert(flags.find(key) != flags.end());

        return std::get<T>(flags.find(key)->second);
    }
};

#endif