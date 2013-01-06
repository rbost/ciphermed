
#include <stdexcept>
#include <assert.h>
#include <errstream.h>

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


