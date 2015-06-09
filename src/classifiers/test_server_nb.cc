/*
 * Copyright 2013-2015 Raphael Bost
 *
 * This file is part of ciphermed.

 *  ciphermed is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  ciphermed is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with ciphermed.  If not, see <http://www.gnu.org/licenses/>. 2
 *
 */

#include <classifiers/nb_classifier.hh>
#include <util/benchmarks.hh>
#include <ctime>
#include <fstream>


#include <json/reader.h>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <vector>
#include <iterator>


template <typename T>
class json_const_iterator : public iterator<forward_iterator_tag, const T> {
public:
    json_const_iterator() = default;
    json_const_iterator(const Json::Value::const_iterator &impl) : impl_(impl) {}
    
    inline const T & operator*() const;
    inline const T * operator->() const;
    
    inline bool operator==(const json_const_iterator &o) const { return impl_ == o.impl_; }
    inline bool operator!=(const json_const_iterator &o) const { return impl_ != o.impl_; }
    
    inline json_const_iterator &
    operator++()
    {
        ++impl_;
        return *this;
    }
    
    inline json_const_iterator
    operator++(int)
    {
        json_const_iterator cur = *this;
        ++(*this);
        return cur;
    }
    
private:
    Json::Value::const_iterator impl_;
    mutable T v_;
};

template <typename T>
struct extractor {};

template <>
struct extractor<double> {
    inline double operator()(const Json::Value &v) const { return v.asDouble(); }
};

template <typename T>
struct extractor<vector<T>> {
    inline vector<T>
    operator()(const Json::Value &v) const
    {
        return vector<T>(json_const_iterator<T>(v.begin()), json_const_iterator<T>(v.end()));
    }
};

template <typename T>
inline const T &
json_const_iterator<T>::operator*() const
{
    v_ = extractor<T>()(*impl_);
    return v_;
}

template <typename T>
inline const T *
json_const_iterator<T>::operator->() const
{
    v_ = extractor<T>()(*impl_);
    return &v_;
}

static void read_prob_file(const string &filename, vector<double> &prior_vec, vector<vector<vector<double>>> &conditionals_vec)
{
    Json::Value root;
    Json::Reader reader;
    ifstream ifs(filename);
    if (!reader.parse(ifs, root))
        throw runtime_error("foo");
    assert(root.isObject());
    
    const Json::Value &prior = root["prior"];
    const Json::Value &conditionals = root["conditionals"];
    const size_t n_classes = prior.size();
    cerr << "n_classes: " << n_classes << endl;
    
    prior_vec = vector<double>(json_const_iterator<double>(prior.begin()), json_const_iterator<double>(prior.end()));
//    cerr << "priors: " << prior_vec << endl;
    
    conditionals_vec = vector<vector<vector<double>>>(
                                                      json_const_iterator< vector<vector<double>> >(conditionals.begin()),
                                                      json_const_iterator< vector<vector<double>> >(conditionals.end()));
    
}

static void test_nb_classifier_server(const vector<vector<vector<double>>> &conditionals_vec, const vector<double> &prior_vec)
{
#ifdef BENCHMARK
    cout << "BENCHMARK flag set" << endl;
    BENCHMARK_INIT
#endif
    
    gmp_randstate_t randstate;
    gmp_randinit_default(randstate);
    gmp_randseed_ui(randstate,time(NULL));
    
    srand(time(NULL));
    
    //    assert(nbits_max > model_size + 1);
    //    unsigned int nbits = nbits_max - model_size - 1;
    //
    //    long two_nbits = 1 << nbits;
    //
    //    vector<mpz_class> model(model_size+1);
    //    for (size_t i = 0; i <= model_size; i++) {
    //        model[i] = rand()%two_nbits;
    //        if (rand()%2) {
    //            model[i] *= -1;
    //        }
    //    }
    
    cout << "Server for linear classifier\n";
    cout << "Init server" << endl;
    
    Naive_Bayes_Classifier_Server server(randstate,1024,100,conditionals_vec,prior_vec);
    
    cout << "Start server" << endl;
    server.run();
}


int main(int argc, char* argv[])
{
    vector<vector<vector<double>>> conditionals_vec;
    vector<double> prior_vec;
    
    if (argc != 2)
    {
        std::cerr << "Usage: server_nb <filename>" << std::endl;
        return 1;
    }
    
    
    
    read_prob_file(argv[1],prior_vec,conditionals_vec);
    test_nb_classifier_server(conditionals_vec, prior_vec);
    
    return 0;
}