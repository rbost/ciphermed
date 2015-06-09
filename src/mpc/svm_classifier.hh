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

#include <NTL/ZZ.h>
#include <vector>
#include <array>
#include <utility>
#include <pthread.h>


#include <mpc/millionaire.hh>
#include <crypto/paillier.hh>

//class SimpleClassifier_Client {
//public:
//    SimpleClassifier_Client();
//    
//    std::vector<NTL::ZZ> paillierPubKey() const {return paillier_.pubkey(); };
//    std::vector<NTL::ZZ> millionairePubParam() const {return millionaire_.pubparams(); };
//    
//    std::vector<NTL::ZZ> encryptVector(const std::vector<NTL::ZZ> &vec);
//    std::vector< std::array<std::pair<NTL::ZZ,NTL::ZZ>,2> > decryptAndStartMillionaire(const NTL::ZZ &c);
//    bool compareResult(const std::vector< std::pair<NTL::ZZ,NTL::ZZ> > &c) const;
//    
//protected:
//    Paillier_priv paillier_;
//    Millionaire_Alice millionaire_;
//};
//
//class SimpleClassifier_Server {
//public:
//    SimpleClassifier_Server(const std::vector<long> v);
//    ~SimpleClassifier_Server();
//    
//    std::vector<std::pair<size_t,NTL::ZZ> > randomizedDotProduct(std::vector<NTL::ZZ> &vec, size_t nQueries, const std::vector<NTL::ZZ> &pk_paillier);
//    std::vector<std::pair<size_t,NTL::ZZ> > randomizedDotProduct_smallError(std::vector<NTL::ZZ> &vec, size_t nQueries, const std::vector<NTL::ZZ> &pk_paillier, const NTL::ZZ &errorBound);
//
//    std::vector< std::pair<NTL::ZZ,NTL::ZZ> > compareRandomness(const std::vector< std::array<std::pair<NTL::ZZ,NTL::ZZ>,2> > &T,const std::vector<NTL::ZZ> &mil_pubparam, size_t i_query);
//    
//protected:
//    const std::vector<long> model_;
//    const size_t m_length_;
//    size_t queries_count_;
//    std::vector<NTL::ZZ> randomness_;
//    pthread_mutex_t mutex_queries_;
//};
//
