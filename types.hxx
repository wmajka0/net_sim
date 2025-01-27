#ifndef NETSIM_TYPES_HPP
#define NETSIM_TYPES_HPP
#include <cstdint>
#include <functional>
using ElementID = uint32_t;
using Time = unsigned long long int;
using TimeOffset = unsigned long long int;
using ProbabilityGenerator = std::function<double()>;

#endif
