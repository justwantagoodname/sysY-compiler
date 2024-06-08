#pragma once

#ifndef FLAG_H
#define FLAG_H

#include "sysY.h"

#include <map>

class Flag {
private:

    std::map<std::string, std::variant<int, std::string> > flags;

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

    const std::string& getFlagFor(const std::string& key) const;
};

#endif