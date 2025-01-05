//
// Created by Karol on 26.12.2024.
//

#include "nodes.h"


void ReceiverPrefences::add_receiver(IPackageReceiver* r)
{
    double i = 0;
    for (auto & preference : preferences_) {
        i += preference.second;
    }
    preferences_.insert(std::make_pair(r, 1-i));
}