#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <utility>


bool not_divisible_by_3and7(unsigned i) {
    return !(i % 3 == 0 || i % 7 == 0);
}

unsigned multiply_by_10(unsigned i) {
    return (i * 10);
}

unsigned bernstein_hash(unsigned h, unsigned x) {
    return (33 * h + x);
}

#include <ranges>

auto special_number_ranges(unsigned start, unsigned end) {

    std::views::iota nums{start, end};
    auto transformed = nums | std::views::filter(not_divisible_by_3and7)
        | std::views::transform(multiply_by_10);


    unsigned h = std::accumulate(transformed.begin(), transformed.end(), 
             0, bernstein_hash);

     return h;
}


int main(int argc, char* argv[]) {
    unsigned start = atoi(argv[1]);
    unsigned stop = atoi(argv[2]);
    
    unsigned h = -1;
    for (int i = 0; i < 1000; ++i) {
        h = special_number_ranges(start, stop);
    }
    std::cout << h << "\n";
}
