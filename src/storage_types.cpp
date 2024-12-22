//
// Created by Wiktor Majka on 22.12.2024.
//
#include "storage_types.h"

Package PackageQueue::pop() {
    PackageQueueType queue_type = get_queue_type();
    Package package;

    switch (queue_type) {
        case PackageQueueType::FIFO:
            package = std::move((*this).queue.front());
            queue.pop_front();
            break;

        case PackageQueueType::LIFO:
            package = std::move((*this).queue.back());
            queue.pop_back();
            break;
    }

    return package;
}

