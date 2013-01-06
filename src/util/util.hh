#pragma once

#include <util/errstream.hh>

void assert_s (bool value, const std::string &msg)
    throw (FHEError);
 
