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