#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;
using namespace boost::multiprecision;
using namespace boost::property_tree;

cpp_int base_to_decimal(string value_str, int base) {
    transform(value_str.begin(), value_str.end(), value_str.begin(),
              [](unsigned char c) { return tolower(c); });
    cpp_int res = 0;
    cpp_int power = 1;
    for (int i = value_str.size() - 1; i >= 0; i--) {
        char c = value_str[i];
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            digit = c - 'a' + 10;
        } else {
            throw invalid_argument("Invalid character in value string");
        }
        if (digit >= base) {
            throw invalid_argument("Digit exceeds base");
        }
        res += digit * power;
        power *= base;
    }
    return res;
}

int main() {
    vector<string> filenames = {"test1.json", "test2.json"};
    for (string filename : filenames) {
        ptree pt;
        try {
            read_json(filename, pt);
        } catch (const json_parser_error& e) {
            cerr << "Error reading JSON file: " << e.what() << endl;
            return 1;
        }

        ptree keys = pt.get_child("keys");
        int n = keys.get<int>("n");
        int k = keys.get<int>("k");

        vector<pair<cpp_int, cpp_int>> points;

        for (auto& item : pt) {
            string key = item.first;
            if (key == "keys") continue;
            cpp_int x;
            try {
                x = cpp_int(key);
            } catch (...) {
                cerr << "Invalid x value: " << key << endl;
                return 1;
            }
            ptree node = item.second;
            string base_str = node.get<string>("base");
            string value_str = node.get<string>("value");
            int base_int = stoi(base_str);
            cpp_int y;
            try {
                y = base_to_decimal(value_str, base_int);
            } catch (const invalid_argument& e) {
                cerr << "Error converting value: " << e.what() << endl;
                return 1;
            }
            points.push_back(make_pair(x, y));
        }

        sort(points.begin(), points.end(),
             [](const pair<cpp_int, cpp_int>& a, const pair<cpp_int, cpp_int>& b) {
                 return a.first < b.first;
             });

        if (static_cast<int>(points.size()) < k) {
            cerr << "Not enough points: k is " << k << " but we have " << points.size() << endl;
            return 1;
        }

        vector<cpp_int> x_vals;
        vector<cpp_int> y_vals;
        for (int i = 0; i < k; i++) {
            x_vals.push_back(points[i].first);
            y_vals.push_back(points[i].second);
        }

        cpp_int constant_term = 0;
        for (int i = 0; i < k; i++) {
            cpp_int numerator = 1;
            cpp_int denominator = 1;
            for (int j = 0; j < k; j++) {
                if (j == i) continue;
                numerator *= -x_vals[j];
                denominator *= (x_vals[i] - x_vals[j]);
            }
            cpp_int term = y_vals[i] * numerator;
            term /= denominator;
            constant_term += term;
        }

        cout << "Secret for " << filename << ": " << constant_term << endl;
    }

    return 0;
}