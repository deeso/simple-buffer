//
// Created by apridgen on 5/29/21.
//

#ifndef SIMPLE_BUFFER_OPTIONS_H
#define SIMPLE_BUFFER_OPTIONS_H

#include <iostream>
#include <string>

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

using std::ostream;
using std::string;

class Options {
public:
    Options();
    Options(const Options& other) = default;
    Options(Options&& other) = default;
    ~Options();

    void parse(int argc, char* argv[]);
    friend ostream& operator<<(ostream& os, Options& options);

    int get_option1() {return myOption1;}
    std::string get_option2() {return myOption2;}

private:
    static const int OPTION1_DEFAULT;
    static const std::string OPTION2_DEFAULT;

    int myOption1;
    std::string myOption2;


    bpo::variables_map *myOptionVars;
    bpo::options_description *myOptionDesc;
};


#endif
