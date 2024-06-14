#pragma once

#ifndef ARMAS_H
#define ARMAS_H

#include "sysY.h"
#include <cstdarg>

class AssemblyBuilder {
private:
    FILE* output;

public:
    
    AssemblyBuilder(const char* filename) {
        assert(filename != nullptr);
        if (strcmp(filename, "-") == 0) {
            output = stdout;
        } else {
            output = fopen(filename, "w");
        }
    }

    ~AssemblyBuilder() {
        if (output != stdout) fclose(output);
    }
    
    AssemblyBuilder& raw(const char* str) {
        fputs(str, output);
        return *this;
    }

    AssemblyBuilder& line() {
        line("\n");
        return *this;
    }
    
    AssemblyBuilder& line(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vfprintf(output, format, args);
        if (format[strlen(format) - 1] != '\n') {
            fputc('\n', output);
        }
        va_end(args);
        return *this;
    }

    template<typename T>
    AssemblyBuilder& p(T line) {
        fputs(output, line);
        fputc('\n', output);
        return *this;
    }
    
    template<typename T, typename... Args>
    AssemblyBuilder& p(T line, Args... args) {
        fputs(output, line);
        return p(args...);
    }

    AssemblyBuilder& operator<<(const char* str) {
        fputs(str, output);
        return *this;
    }

    AssemblyBuilder& operator<<(const std::string& str) {
        fputs(str.c_str(), output);
        return *this;
    }

    AssemblyBuilder& operator|(const char* str) {
        line(str);
        return *this;
    }

    template<typename T>
    AssemblyBuilder& operator|(T str) {
        line(std::to_string(str).c_str());
        return *this;
    }

    template<typename T>
    AssemblyBuilder& operator<<(T str) {
        fputs(std::to_string(str).c_str(), output);
        return *this;
    }
};

#endif
