#include "sysY.h"
#include "utils.h"

using std::vector;

size_t multi_dimensional_index_to_linear_index(const vector<int> &dim_sizes, const vector<int> &locator_access_size) {
    assert(dim_sizes.size() == locator_access_size.size());
    for (int i = 0;i < dim_sizes.size(); i++) {
        assert(dim_sizes[i] > 0);
        assert(0 <= locator_access_size[i] && locator_access_size[i] < dim_sizes[i]);
    }

    vector<int> real_dim_sizes = dim_sizes;
    real_dim_sizes.push_back(1);

    for (auto i = real_dim_sizes.size() - 1;i > 0; i--) {
        real_dim_sizes[i - 1] = real_dim_sizes[i - 1] * real_dim_sizes[i];
    }

    size_t linear_index = 0;
    for (int i = 0;i < real_dim_sizes.size(); i++) {
        linear_index += locator_access_size[i] * real_dim_sizes[i + 1];
    }
    return linear_index;
}

void print_version() {
static const char* easter = 
#include "chillet.h"
;
    printf("%s\n", easter);
#ifdef VERSION_TEXT
    printf("Chillet sysy Compiler Version: %s\n", VERSION_TEXT);
#else
    printf("Chillet sysy Compiler Version: Unknown\n");
#endif
    printf("Compile Time: %s %s\n", __DATE__, __TIME__);
    printf("CopyRight (C) 2024 Dev Team %s. All Rights Reserved.\n", "编译成蓝色疾旋鼬");
    exit(0);
}

void print_help(const char* program_name) {
    printf("Usage: %s [file] [-hv] [-S] [-o output] [-O level]\n", program_name);
    printf("Options:\n");
    printf("  -h: Print this help message\n");
    printf("  -v: Print version information\n");
    printf("  -S: Generate assembly code\n");
    printf("  -o: Output file\n");
    printf("  -O: Optimization level\n");
    exit(0);
}

bool is_integer(double x) {
    return std::floor(x) == x;
}