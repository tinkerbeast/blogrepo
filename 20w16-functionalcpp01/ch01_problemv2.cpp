#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>
#include <unordered_map>
#include <utility>

#include <cstdio>

auto word_frequencies_even(const std::vector<std::string>& files) {

    std::unordered_map<std::string, int> word_freq;

    for (const auto& file_name: files) {
        std::ifstream in(file_name);
        std::string word;
        while (in >> word) {
            if (word.size() % 2 == 0) { // if added at inermost nested loop
                ++word_freq[word];
            }           
        }
    }

    return word_freq; 
}

auto string_is_even(const std::string& str) {
    return str.size() % 2 == 0;
}

auto filename_to_words(const std::string& file_name) {
    std::ifstream in_file(file_name);
    // we can use istream_iterator to iterate over a file
    std::istream_iterator<std::string> in(in_file); 
    // create vector using iterator instead of a while loop to push back
    std::vector<std::string> words(in, std::istream_iterator<std::string>());
    return words;
}

auto& flatten(std::vector<std::string>& a, const std::vector<std::string>& b) {
    // reduce two similar vector containers to one
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

auto word_frequencies_even_functional(const std::vector<std::string>& files) {

    // 1. transform file names to list of list of words
    std::vector< std::vector<std::string> > vec_words;
    std::transform(files.begin(), files.end(), 
            std::back_inserter(vec_words), filename_to_words);

    // 2. flatten list of list to a single list
    std::vector<std::string> nowords;
    auto&& words = std::accumulate(vec_words.begin(), vec_words.end(), 
            nowords, flatten);

    // 4.filter the words on even size criteria
    std::vector<std::string> filtered_words;
    std::copy_if(words.begin(), words.end(), 
            std::back_inserter(filtered_words), string_is_even);

    // 3. we still can't avoid this loop yet
    std::unordered_map<std::string, int> word_freq;
    for (const auto& word: filtered_words) {
        ++word_freq[word];
    }

    return word_freq;
}


void print_top(const std::unordered_map<std::string, int>& word_freq, int count) {

    // This function is written in functional style to get the reader to be
    // familiar with functional coding style. If the reader does not understand
    // all parts of the following code, that's perfectly normal.

    // frequency to sting mapping needed to print top results
    std::vector<std::pair<int, std::string>> freq_word; 

    // convert word-freq to freq-word
    std::for_each(std::begin(word_freq), std::end(word_freq), 
            [&freq_word](const auto& pair) {
                freq_word.push_back(std::make_pair(pair.second, pair.first));                
            });

    // sort, reverse and print
    std::sort(std::begin(freq_word), std::end(freq_word));
    std::reverse(std::begin(freq_word), std::end(freq_word));
    std::for_each(std::begin(freq_word), std::begin(freq_word) + count,
            [](const auto& pair) {
                std::cout << pair.second << " " << pair.first << "\n";
            });
}


int main() {
    std::vector<std::string> files{"book1.txt", "book2.txt", "book3.txt", "book4.txt", "book5.txt"};

    auto&& imperative_result = word_frequencies_even(files);
    std::cout << "Top 5 even results by imperative code:\n";
    print_top(imperative_result, 5);
    
    auto&& functional_result = word_frequencies_even_functional(files);
    std::cout << "Top 5 even results by functional code:\n";
    print_top(functional_result, 5);
}
