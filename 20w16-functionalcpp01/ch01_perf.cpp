#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <utility>


auto special_number(unsigned start, unsigned end) {
    unsigned h = 0;
    for(auto i = start; i < end; ++i) {
        if (i % 3 == 0 || i % 7 == 0) {
            // do nothing
        } else {
            auto x = i * 10;
            h = 33 * h + x;
        }
    }
    return h;
}

template<typename T>
class MyRangeIterator {
public:
    MyRangeIterator(T value) 
    : i_(value), value_(value)
    { }

    bool operator!=(const MyRangeIterator<T>& obj) const { 
        return i_ != obj.i_; 
    }

    MyRangeIterator<T>& operator++() { 
        ++i_; 
        return *this;
    }

    T& operator*() {
        return i_;
    }

private:
    T i_;
    T value_;
};

template<typename T>
class MyRange {
public:
    MyRange(T start, T stop) 
    : start_(start), stop_(stop)
    {}

    auto begin() {
        return MyRangeIterator<T>(start_);
    }

    auto end() {
        return MyRangeIterator<T>(stop_);
    }

private:
    T start_;
    T stop_;
};


bool not_divisible_by_3and7(unsigned i) {
    return !(i % 3 == 0 || i % 7 == 0);
}

unsigned multiply_by_10(unsigned i) {
    return (i * 10);
}

unsigned bernstein_hash(unsigned h, unsigned x) {
    return (33 * h + x);
}

auto special_number_functional(unsigned start, unsigned end) {

    static std::vector<unsigned> filtered;
    static std::vector<unsigned> transformed;

    MyRange<unsigned> r(start, end);
    filtered.clear();
    std::copy_if(r.begin(), r.end(),
            std::back_inserter(filtered), not_divisible_by_3and7); 

    transformed.clear();
    std::transform(filtered.begin(), filtered.end(),
            std::back_inserter(transformed), multiply_by_10);

     unsigned h = std::accumulate(transformed.begin(), transformed.end(), 
             0, bernstein_hash);

     return h;
}


int main(int argc, char* argv[]) {
    unsigned start = atoi(argv[1]);
    unsigned stop = atoi(argv[2]);
    
    unsigned h = -1;
    for (int i = 0; i < 1000; ++i) {
#ifdef FUNCTIONAL_FLAG
        h = special_number_functional(start, stop);
#else
        h = special_number(start, stop);
#endif
    }
    std::cout << h << "\n";
}
