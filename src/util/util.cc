
#include <stdexcept>
#include <assert.h>
#include <iostream>

#include <util/util.hh>
#include <util/errstream.hh>

using namespace std;


void
assert_s (bool value, const string &msg)
throw (FHEError)
{
    if (!value) {
	cerr << "ERROR: " << msg;
	throw FHEError(msg);
    }
}

