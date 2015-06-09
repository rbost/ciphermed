/*
 * Copyright 2013-2015 Raphael Bost & Stephen Tu
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

// compile: g++ -Wall -std=c++11 -o json -I/usr/include/jsoncpp json.cpp -ljsoncpp -ljson
// run: ./json filename

#include <json/reader.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <iterator>
using namespace std;

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

template <typename T>
inline ostream &
operator<<(ostream &o, const vector<T> &v)
{
  o << "[";
  bool f = true;
  for (auto &t : v) {
    if (!f)
      o << ", ";
    f = false;
    o << t;
  }
  o << "]";
  return o;
}

int
main(int argc, char **argv)
{
  Json::Value root;
  Json::Reader reader;
  ifstream ifs(argv[1]);
  if (!reader.parse(ifs, root))
    throw runtime_error("foo");
  assert(root.isObject());

  const Json::Value &prior = root["prior"];
  const Json::Value &conditionals = root["conditionals"];
  const size_t n_classes = prior.size();
  cerr << "n_classes: " << n_classes << endl;

  const vector<double> prior_vec(json_const_iterator<double>(prior.begin()), json_const_iterator<double>(prior.end()));
  cerr << "priors: " << prior_vec << endl;

  const vector<vector<vector<double>>> conditionals_vec(
    json_const_iterator< vector<vector<double>> >(conditionals.begin()),
    json_const_iterator< vector<vector<double>> >(conditionals.end()));

  const size_t n_features = conditionals_vec[0].size();
  cerr << "n_features: " << n_features << endl;

  for (size_t i = 0; i < n_classes; i++)
    for (size_t j = 0; j < n_features; j++)
      cerr << "class=" << i << ", feature=" << j << ", p(F=f|Y=y)=" << conditionals_vec[i][j] << endl;

  return 0;
}
