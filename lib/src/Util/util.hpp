//
// Created by mimixtop on 06.08.25.
//

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>

namespace util {

    //match operator
    template <typename ...Lambdas>
    struct match : Lambdas... {
        using Lambdas::operator()...;
    };

    template <typename ...Lambdas>
    match(Lambdas... lambdas) -> match<Lambdas...>;

    class StringWorker {
    public:
        static std::string getFirstLemma(std::string &line);
    };
} // util

#endif //UTIL_HPP
