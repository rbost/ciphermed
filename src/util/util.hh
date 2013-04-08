#pragma once

#include <stdexcept>
#include <assert.h>
#include <iostream>

#include <util/errstream.hh>
#include <util/compiler.hh>

inline void
assert_s(bool value, const std::string &msg) throw (FHEError)
{
    if (unlikely(!value)) {
        std::cerr << "ERROR: " << msg << std::endl;
        throw FHEError(msg);
    }
}
