//Link to Boost
#define BOOST_TEST_DYN_LINK
//Define our Module name (prints at testing)
#define BOOST_TEST_MODULE "basic_options"
//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>
#include "options.h"
// ------------- Tests Follow --------------
BOOST_AUTO_TEST_CASE( basic_options )
{
    Options obj;
    Options obj2;
//Check default constructor
//Use BOOST_CHECK for small equal checks - true or false
    BOOST_CHECK(obj.get_option1() == obj2.get_option1());

}
