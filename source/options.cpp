
//
// Created by apridgen on 5/29/21.
//

#include "options.h"
#include "../include/options.h"
#include <string>

const int Options::OPTION1_DEFAULT = 42;
const std::string Options::OPTION2_DEFAULT = "hello world";


Options::Options() :
        myOptionDesc(new bpo::options_description("simple-buffer options")),
        myOptionVars(new bpo::variables_map()),
        myOption1(Options::OPTION1_DEFAULT),
        myOption2(Options::OPTION2_DEFAULT)
{
    myOptionDesc->add_options()
            ("option1", bpo::value<int>(&myOption1)->default_value(Options::OPTION1_DEFAULT), "Option1")
            ("option2", bpo::value<std::string>(&myOption2)->default_value(Options::OPTION2_DEFAULT), "Option2");
}

Options::~Options() {

    if (myOptionDesc != nullptr) {
        delete myOptionDesc;
        myOptionDesc = nullptr;
    }

    if (myOptionVars != nullptr) {
        delete myOptionVars;
        myOptionVars = nullptr;
    }
}

void Options::parse(int argc, char* argv[]) {
    bpo::store(bpo::parse_command_line(argc, argv, *myOptionDesc), *myOptionVars);
    bpo::notify(*myOptionVars);
}

ostream& operator<<(ostream& os, Options& options) {
    os << "Your basic option is: " << options.myOption1 << std::endl;
    return os;
}