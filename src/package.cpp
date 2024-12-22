//
// Created by Wiktor Majka on 22.12.2024.
//
#include "package.h"

std::set<ElementID> Package::freed_IDs = {};
std::set<ElementID> Package::assigned_IDs = {};

Package::Package() {
    if (freed_IDs.empty()) {
        if (assigned_IDs.empty()) {
            ID_ = 1;
        } else {
            ID_ = *std::prev(assigned_IDs.end()) + 1;
        }
    } else {
        ID_ = *freed_IDs.begin();
        freed_IDs.erase(freed_IDs.begin());
    }
    assigned_IDs.insert(ID_);
}


Package::~Package() {
    freed_IDs.insert(ID_);
    assigned_IDs.erase(ID_);
}

Package& Package::operator=(Package&& package) noexcept{
    if (this == &package)
    {
        return *this;
    }
    assigned_IDs.erase(this -> ID_);
    freed_IDs.insert(this -> ID_);
    this -> ID_ = package.ID_;
    assigned_IDs.insert(this -> ID_);
    return *this;
}