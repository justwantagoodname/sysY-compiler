#include "sysY.h"
#include "flag.h"
#include "utils.h"

Flag& Flag::getFlagInstance() {
    static Flag flag;
    return flag;
}

/**
 * 这个函数读取命令行选项并解析，目前可以指定输入输出文件，以及优化等级，添加了新的选项后一定要去更新 Help 函数
 * @param argc
 * @param argv
 */

void Flag::init(int argc, const char** argv) {
    assert(argc > 0);
    assert(argv != nullptr);

    if (argc == 1) {
        print_help(argv[0]);
    }

    std::vector<std::string> arguments; // 所有不是选项的参数

    auto& flags = Flag::getFlagInstance().flags;

    auto is_option = [](const std::string& arg) {
        return arg[0] == '-';
    };

    auto parser_option = [&](int index) -> int {
        std::string option(argv[index]);

        assert(option[0] == '-' && option.length() > 1);
        switch (option[1]) {
            case 'h':
                print_help(argv[0]);
                break;
            case 'v':
                print_version();
                break;
            case 'S':
                flags["ASM"] = true;
                return 0;
            case 'o':
                flags["output"] = (std::string) argv[index + 1];
                return 1;
            case 'O':
                flags["Olevel"] = atoi(option.substr(2).c_str());
                return 0;
            case 'f':
                flags[option.substr(2)] = true;
                return 1;
            default:
                printf("Unknown option: %s\n", argv[index]);
                print_help(argv[0]);
                return 0;
        }
        return 0;
    };

    // 设置默认值
    flags["output"] = std::string("-"); // 默认输出到标准输出
    flags["dump-raw"] = false;
    flags["dump-optimized-tree"] = false;
    flags["dump-generated-tree"] = false;
    flags["debug-flag"] = false;
    flags["ASM"] = true;
    flags["Olevel"] = 0;
    
    // arg 0 is the program name
    for (int i = 1; i < argc; i++) {
        if (is_option(argv[i])) {
            i += parser_option(i);
        } else {
            arguments.push_back(argv[i]);
        }
    }

    flags["input"] = std::string(arguments[0]);

    if (Flag::getFlagInstance().by<bool>("debug-flag")) {
        for (auto &flag : flags) {
            std::cout << "Cmdline Flag: " << flag.first;
            std::cout << " Type: " << flag.second.index();
            if (flag.second.index() == 0) {
                std::cout << " " << std::get<int>(flag.second) << std::endl;
            } else if(flag.second.index() == 1) {
                std::cout << " " << std::get<std::string>(flag.second) << std::endl;
            } else {
                std::cout << " " << std::boolalpha << std::get<bool>(flag.second) << std::endl;
            }
        }
    }
}
