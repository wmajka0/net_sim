//
// Created by Karol on 26.12.2024.
//

#include "nodes.h"

using ReceiverWeightPair = std::pair<IPackageReceiver* const, double>;

void ReceiverPrefences::add_receiver(IPackageReceiver* receiver) {
    double total_weight = 0.0;
    for (const auto& entry : preferences_) {
        total_weight += entry.second;
    }
    preferences_[receiver] = 1.0 - total_weight;
}

void ReceiverPrefences::remove_receiver(IPackageReceiver* receiver) {
    auto it = preferences_.find(receiver);
    if (it == preferences_.end()) return;

    preferences_.erase(it);

    size_t receiver_count = preferences_.size();
    if (receiver_count == 0) return;

    double updated_weight = 1.0 / static_cast<double>(receiver_count);
    for (auto& entry : preferences_) {
        entry.second = updated_weight;
    }
}

void Storehouse::receive_package(Package&& package) {
    queue_->push(std::move(package));
}

IPackageReceiver* ReceiverPrefences::choose_receiver() {
    double random_value = default_probability_generator();
    if (random_value < 0.0 || random_value > 1.0) return nullptr;

    double cumulative_weight = 0.0;
    for (const auto& entry : preferences_) {
        cumulative_weight += entry.second;
        if (random_value <= cumulative_weight) {
            return entry.first;
        }
    }
    return nullptr;
}

void PackageSender::send_package() {
    if (!bufor_) return;

    IPackageReceiver* chosen_receiver = receiver_preferences_.choose_receiver();
    if (chosen_receiver) {
        chosen_receiver->receive_package(std::move(*bufor_));
        bufor_.reset();
    }
}

void Worker::receive_package(Package&& package) {
    queue_->push(std::move(package));
}

void Worker::do_work(Time current_time) {
    if (!bufor_ && !queue_->empty()) {
        time_this_package_ = current_time;
        bufor_ = queue_->pop();
    }

    if (current_time - time_this_package_ + 1 == time_) {
        push_package(Package(bufor_->get_id()));
        bufor_.reset();
    }
}

void Ramp::deliver_goods(Time current_time) {
    if (!buffor_) {
        push_package(Package());
        buffor_ = id_;
        t_ = current_time;
    } else if (current_time - t_ == time_) {
        push_package(Package());
    }
}
