#include "sysY.h"
#include "flag.h"
#include "utils.h"

#include <unistd.h>

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

    auto& flags = Flag::getFlagInstance().flags;

    if (argc == 1) {
        print_help(argv[0]);
    }

    flags["input"] = argv[1];
    flags["output"] = "-"; // 默认输出到标准输出

    int ch;
    while ((ch = getopt(argc, (char* const*)argv, "hvSo:O::")) != -1) {
        switch (ch) {
            case 'S':
                flags["S"] = "true";
                break;
            case 'o':
                flags["output"] = optarg;
                break;
            case 'O':
                flags["Olevel"] = atoi(optarg);
                break;
            case 'h':
                print_help(argv[0]); // 这个函数直接终止程序
            case 'v':
                print_version(); // 这个函数直接终止程序
            default:
                printf("Unknown option: %c\n", ch);
                break;
        }
    }

#ifdef OUTPUT_FLAG
    for (auto &flag : flags) {
        std::cout << "Cmdline Flag: " << flag.first;
        // print std::variant
        if (flag.second.index() == 0) {
            std::cout << " " << std::get<int>(flag.second) << std::endl;
        } else {
            std::cout << " " << std::get<std::string>(flag.second) << std::endl;
        }
    }
#endif

}


const std::string& Flag::getFlagFor(const std::string& key) const {
    assert(flags.find(key) != flags.end());

    return std::get<std::string>(flags.find(key)->second);
}

