//
// Created by Wiktor Majka on 22.12.2024.
//

#ifndef MOJEZ_NET_SIM_STORAGE_TYPES_H
#define MOJEZ_NET_SIM_STORAGE_TYPES_H

#include <iostream>
#include "package.h"
#include <list>

enum class PackageQueueType {
    FIFO,
    LIFO
};



class IPackageStockpile {
public:
    virtual void push(Package&& package) = 0;
    virtual bool empty() const = 0;

    virtual std::size_t size() const = 0;

    using const_iterator = std::list<Package>::const_iterator;
    virtual const_iterator begin() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual const_iterator end() const = 0;
    virtual const_iterator cend() const = 0;

    virtual ~IPackageStockpile() = default;

};

class IPackageQueue:public IPackageStockpile{
public:
    virtual Package pop() = 0;
    virtual PackageQueueType get_queue_type() const = 0;
};


class PackageQueue:public IPackageQueue {
public:
    PackageQueue(PackageQueueType type_list): queue_type{type_list} {}

    void push(Package&& package) override {queue.push_back(package);}
    bool empty() const override {return queue.empty();}

    std::size_t size() const override {
        return (*this).queue.size();
    }
    const_iterator begin() const override
    {
        return (*this).queue.cbegin();
    };
    const_iterator cbegin() const override
    {
        return (*this).queue.cbegin();
    };
    const_iterator end() const override
    {
        return (*this).queue.cend();
    };
    const_iterator cend() const override
    {
        return (*this).queue.cend();
    };


    PackageQueueType get_queue_type() const override { return queue_type;};
    Package pop();

private:
    PackageQueueType queue_type;
    std::list<Package> queue;

};

#endif //MOJEZ_NET_SIM_STORAGE_TYPES_H
