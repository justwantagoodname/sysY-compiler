#pragma once

#ifndef ASM_HELPER_H
#define ASM_HELPER_H

#include "sysY.h"
#include <cstdarg>

class AssemblyBuilder {
private:
    FILE* output;
    long int mark_point;
    char* buffer{nullptr};
    size_t buffer_size{0};
public:
    
    AssemblyBuilder(const char* filename) {
        assert(filename != nullptr);
        if (strcmp(filename, "-") == 0) {
            output = stdout;
        } else {
            output = fopen(filename, "w+");
        }
        mark_point = 0;
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

    AssemblyBuilder& mark() {
        mark_point = ftell(output);
        return *this;
    }

    AssemblyBuilder& beginBack() {
        fseek(output, 0, SEEK_END);
        auto size = ftell(output);

        fseek(output, mark_point, SEEK_SET);
        // use buffer to store the content between mark_point and the end of file
        buffer = new char[size - mark_point + 128];
        buffer_size = fread(buffer, 1, size - mark_point, output);
        fseek(output, mark_point, SEEK_SET);
        return *this;
    }

    AssemblyBuilder& endBack() {
        fwrite(buffer, 1, buffer_size, output);
        fseek(output, 0, SEEK_END);
        delete[] buffer;
        buffer = nullptr;
        buffer_size = 0;
        return *this;
    }
};

#endif
