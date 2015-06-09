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

#include <mpc/lsic.hh>
#include <mpc/private_comparison.hh>
#include <typeinfo>

void runProtocol(Comparison_protocol_A *party_a, Comparison_protocol_B *party_b, gmp_randstate_t state)
{
    if(typeid(*party_a) == typeid(LSIC_A)) {
        runProtocol(reinterpret_cast<LSIC_A*>(party_a),reinterpret_cast<LSIC_B*>(party_b),state);
    }else if(typeid(*party_a) == typeid(Compare_A))
    {
        runProtocol(reinterpret_cast<Compare_A*>(party_a),reinterpret_cast<Compare_B*>(party_b),state);
    }

}
