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

#include <mpc/svm_classifier.hh>
#include <assert.h>
#include <algorithm>

using namespace std;
using namespace NTL;





//
//SimpleClassifier_Client::SimpleClassifier_Client()
//: paillier_(Paillier_priv::keygen()), millionaire_()
//{
//}
//
//vector<ZZ> SimpleClassifier_Client::encryptVector(const vector<ZZ> &vec)
//{
//    vector<ZZ> enc(vec.size());
//    
//    for (size_t i = 0; i < vec.size(); i++) {
//        enc[i] = paillier_.encrypt(vec[i]);
//    }
//    
//    return enc;
//}
//
//vector<array <pair<ZZ,ZZ>,2> > SimpleClassifier_Client::decryptAndStartMillionaire(const ZZ &c)
//{
//    ZZ v = paillier_.decrypt(c);
//    return millionaire_.genTable(NumBits(v),v);
//}
//
//bool SimpleClassifier_Client::compareResult(const vector< pair<ZZ,ZZ> > &c) const
//{
//    return millionaire_.decryptRound(c);
//}
//
//
//SimpleClassifier_Server::SimpleClassifier_Server(const std::vector<long> v)
//: model_(v), m_length_(v.size()), queries_count_(0), randomness_(0)
//{
//    pthread_mutex_init(&mutex_queries_, NULL);
//}
//
//SimpleClassifier_Server::~SimpleClassifier_Server()
//{
//    pthread_mutex_destroy(&mutex_queries_);
//}
//
//vector<pair<size_t,ZZ> > SimpleClassifier_Server::randomizedDotProduct(vector<ZZ> &vec, size_t nQueries, const vector<ZZ> &pk_paillier)
//{
//    assert(vec.size() == m_length_);
//
//    ZZ n = pk_paillier[0];
//    Paillier p(pk_paillier);
//    
//    vector<pair<size_t,ZZ> >values(nQueries);
//    ZZ result = p.encrypt(to_ZZ(0));
//    
//    for (size_t i = 0; i < m_length_; i++) {
//        result = p.add(result,  p.constMult(model_[i],vec[i]));
//    }
//    
//    for (size_t i = 0; i < nQueries; i++) {
//        
//        size_t i_query;
//        
//        ZZ val;
//        ZZ rnd = RandomBnd(n);
//        val = p.add(result, p.encrypt(rnd));
//        
//        pthread_mutex_lock(&mutex_queries_);
//        
//        randomness_.push_back(rnd);
//        i_query = queries_count_;
//        queries_count_++;
//        
//        pthread_mutex_unlock(&mutex_queries_);
//        
//        values[i] = make_pair(i_query,val);
//    }
//    
//    return values;
//}
//
//vector<pair<size_t,ZZ> > SimpleClassifier_Server::randomizedDotProduct_smallError(vector<ZZ> &vec, size_t nQueries, const vector<ZZ> &pk_paillier, const ZZ &errorBound)
//{
//    assert(vec.size() == m_length_);
//    
//    ZZ n = pk_paillier[0];
//    Paillier p(pk_paillier);
//    
//    vector<pair<size_t,ZZ> >values(nQueries);
//    ZZ result = p.encrypt(to_ZZ(0));
//    
//    for (size_t i = 0; i < m_length_; i++) {
//        result = p.add(result,  p.constMult(model_[i],vec[i]));
//    }
//    
//    ZZ L;
//    
//    if (errorBound < 0) {
//        L = -errorBound;
//    }else if(errorBound > 0){
//        L = errorBound;
//    }else{
//        // if the given error bound is 0, compute the L_2 norm of the model and set L to be 2^(100)*norm
//        
//        ZZ B;
//        
//        for (size_t i = 0; i < m_length_; i++) {
//            B += model_[i]*model_[i];
//        }
//        
//        L = B << 100;
//        
//        cout << "B = " << B;
//        cout << "\nL = " << L << endl;
//    }
//    
//    for (size_t i = 0; i < nQueries; i++) {
//        
//        size_t i_query;
//        
//        ZZ val;
//        ZZ rnd = RandomBnd(2*L) - L + n/2;
//        val = p.add(result, p.encrypt(rnd));
//        
//        pthread_mutex_lock(&mutex_queries_);
//        
//        randomness_.push_back(rnd);
//        i_query = queries_count_;
//        queries_count_++;
//        
//        pthread_mutex_unlock(&mutex_queries_);
//        
//        values[i] = make_pair(i_query,val);
//    }
//    
//    return values;
//}
//
//vector< pair<ZZ,ZZ> > SimpleClassifier_Server::compareRandomness(const vector< array<pair<ZZ,ZZ>,2> > &T,const std::vector<NTL::ZZ> &mil_pubparam, size_t i_query)
//{
//    Millionaire_Bob millionaire_(mil_pubparam);
//    assert(i_query < m_length_);
//    ZZ r = randomness_[i_query];
//    long nbits = std::max<size_t>(T.size(),NumBits(r));
//    
//    return millionaire_.encryptRound(T, nbits, r);
//}
