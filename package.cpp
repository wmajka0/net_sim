#include <algorithm>
#include "package.hxx"
#include "types.hxx"
#include "storage_types.hxx"

std::vector<ElementID> Package::usedlist_ = {0};
std::vector<ElementID> Package::freelist_ = {};

Package::Package(){
    if(freelist_.empty()){
        auto maxiter = std::max_element(usedlist_.begin(), usedlist_.end());
        ID_=*maxiter+1;
        usedlist_.push_back(ID_);
    }
    else{
        auto miniter = std::min_element(freelist_.begin(), freelist_.end());
        ID_=*miniter;
        freelist_.erase(miniter);
        usedlist_.push_back(ID_);
    }
}

Package::Package(const ElementID id){
    auto id_usediter = std::find(usedlist_.begin(), usedlist_.end(), id);
    if (id_usediter == usedlist_.end()) {
        ID_ = id;
        usedlist_.push_back(ID_);
        auto id_freeiter = std::find(freelist_.begin(), freelist_.end(), id);
        if (id_freeiter != freelist_.end())
            freelist_.erase(id_freeiter);
    }
    else {
        ID_ = 0;
    }
}

Package::Package(Package&& old) noexcept{
    ID_= old.ID_;
    old.ID_ = 0;
}

Package& Package::operator = (Package&& rvalue) noexcept{
    if (ID_!=0 && ID_!=rvalue.ID_) {
        auto id_usediter = std::find(usedlist_.begin(), usedlist_.end(), ID_);
        usedlist_.erase(id_usediter);
        freelist_.push_back(ID_);
    }
    ID_ = rvalue.ID_;
    rvalue.ID_ = 0;
    return *this;
}

Package::~Package(){
    if (ID_!=0) {
        auto id_usediter = std::find(usedlist_.begin(), usedlist_.end(), ID_);
        usedlist_.erase(id_usediter);
        freelist_.push_back(ID_);
    }
}
