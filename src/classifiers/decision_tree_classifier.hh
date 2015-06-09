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

#pragma once

#include <vector>
#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <mpc/enc_comparison.hh>
#include <mpc/rev_enc_comparison.hh>
#include <mpc/linear_enc_argmax.hh>
#include <mpc/tree_enc_argmax.hh>

#include <net/client.hh>
#include <net/server.hh>

#include <tree/tree.hh>
#include <tree/m_variate_poly.hh>

#include <utility>

using namespace std;

#define N_LEVELS 3

class Decision_tree_Classifier_Server : public Server {
public:
    Decision_tree_Classifier_Server(gmp_randstate_t state, unsigned int keysize, const Tree<long> &model, unsigned int n_variables, const vector<pair <vector<long>,long> > &criteria);
  
    Server_session* create_new_server_session(tcp::socket &socket);

    static Key_dependencies_descriptor key_deps_descriptor()
    {
        return Key_dependencies_descriptor(false,false,false,true,true,true);
    }

    Multivariate_poly< vector<long> > model_poly() const { return model_poly_; }
    unsigned int n_variables() const { return n_variables_; }
    vector<pair <vector<long>,long> > criteria() const { return criteria_; }

protected:
    Multivariate_poly< vector<long> > model_poly_;
    const unsigned int n_variables_;
    vector<pair <vector<long>,long> > criteria_;
};


class  Decision_tree_Classifier_Server_session : public Server_session{
public:
    
    Decision_tree_Classifier_Server_session(Decision_tree_Classifier_Server *server, gmp_randstate_t state, unsigned int id, tcp::socket &socket)
    : Server_session(server,state,id,socket), tree_server_(server) {};
    
    void run_session();
    
protected:
    Decision_tree_Classifier_Server *tree_server_;
};

class Decision_tree_Classifier_Client : public Client{
public:
    Decision_tree_Classifier_Client(boost::asio::io_service& io_service, gmp_randstate_t state, unsigned int keysize, vector<long> &query, unsigned int n_nodes);
    
    void run();
    
protected:
    vector<long> query_;
    unsigned int n_nodes_;
};
