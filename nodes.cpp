#include <optional>
#include "nodes.hxx"
#include "storage_types.hxx"
#include "package.hxx"
#include "helpers.hxx"



void PackageSender::send_package(){
    if(PackageBuffer_!=std::nullopt){
        if (receiver_preferences_.get_preferences().empty())
            std::cout<<"BRAK ODBIORCY WYSLANEGO PRODUKTU!"<<std::endl; 
        else{
            Package Package_to_send = Package(std::move(PackageBuffer_.value()));
            PackageBuffer_ = std::nullopt;
            receiver_preferences_.choose_receiver()->receive_package(std::move(Package_to_send));
        }
    }
}

IPackageReceiver* ReceiverPreferences::choose_receiver() {
    double pb = generator_();
    auto iterator = preferences_.begin();
    auto end_it = preferences_.end();
    end_it--;
    double lower_scope=0;
    double higher_scope=iterator->second;
    while(iterator!=end_it){
        if (pb>=lower_scope and pb<higher_scope) {
            break;
        }
        lower_scope=higher_scope;
        iterator++;
        higher_scope+=iterator->second;
    }
    return iterator->first;
}



void ReceiverPreferences::add_receiver(IPackageReceiver *r) {
    if (preferences_.find(r) == preferences_.end()) { 
        preferences_.insert(std::pair<IPackageReceiver *, double>(r, 0));
        double each_receiver_p = 1.0 / double(preferences_.size());
        for (auto &preference : preferences_)
            preference.second = each_receiver_p;
    }
}

void ReceiverPreferences::remove_receiver(IPackageReceiver *r) {
    if (preferences_.find(r) != preferences_.end()) { 
        preferences_.erase(r);
        if (not(preferences_.empty())) {
            double each_receiver_p = 1.0 / double(preferences_.size());
            for (auto &preference : preferences_)
                preference.second = each_receiver_p;
        }
    }
}


void Ramp::deliver_goods(Time t) { 
    if ((double(t-1)/double(di_) == floor(double(t-1)/double(di_))) or t==1) 
        push_package(Package());
}

void Worker::do_work(Time t) {
    if (WorkingBuffer_ == std::nullopt and not(q_->empty())){
            WorkingBuffer_ = q_->pop();
            processing_start_time_ = t;
        }
    if (WorkingBuffer_.has_value() and (processing_start_time_ + pd_ -1 == t)) { 
            push_package(std::move(WorkingBuffer_.value()));
            WorkingBuffer_ = std::nullopt;
        }
    }

Worker::Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> q) {
    id_=id;
    rt_=ReceiverType::WORKER;
    pd_=pd;
    q_=std::move(q);
}

Storehouse::Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d) {
    id_ = id;
    rt_=ReceiverType::STOREHOUSE;
    d_ = std::move(d);

}
