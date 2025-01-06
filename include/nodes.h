#ifndef NODES_H
#define NODES_H

#include <mutex>
#include <optional>
#include <memory>
#include <map>
#include <utility>

#include "package.h"
#include "storage_types.h"
#include "types.h"
#include "helpers.h"

enum class ReceiverType {
    WORKER, STOREHOUSE
};

enum class NodeColor {
    UNVISITED, VISITED, VERIFIED
};

class IPackageReceiver
{
public:
    virtual void  receive_package(Package&& p) = 0;
    virtual ElementID get_id() const = 0;
    virtual IPackageStockpile::const_iterator begin() const = 0;
    virtual IPackageStockpile::const_iterator cbegin() const = 0;
    virtual IPackageStockpile::const_iterator end() const = 0;
    virtual IPackageStockpile::const_iterator cend() const = 0;
    //virtual ReceiverType get_receiver_type() const = 0;
    virtual ~IPackageReceiver() = default;
};

class ReceiverPreferences
{

public:
    using preferences_t = std::map<IPackageReceiver*, double>;
    using const_iterator = preferences_t::const_iterator;

    ReceiverPreferences(ProbabilityGenerator pg = probability_generator): generated_prob_{std::move(pg)} {}
    void add_receiver(IPackageReceiver* r);
    void remove_receiver(IPackageReceiver* r);
    IPackageReceiver* choose_receiver();
    preferences_t&  get_preferences()  {return (*this).preferences_;}

    const_iterator begin() const {return preferences_.cbegin();}
    const_iterator cbegin() const {return preferences_.cbegin();}
    const_iterator end() const {return preferences_.cend();}
    const_iterator cend() const {return preferences_.cend();}

private:
    preferences_t  preferences_;
    ProbabilityGenerator generated_prob_;
};

// poniższa klasa nie jest wirtualna dla ułatwienia zadania(napisane na mvg)
class PackageSender // to ma obsługiwać wysyłkę paczek dla robotnika i rampy
{
public:
    PackageSender(PackageSender&& pase)= default;
    PackageSender() = default;
    PackageSender& operator=(PackageSender&&) = default;

    void send_package();
    const std::optional<Package>& get_sending_buffer() const {return bufor_;}
    ReceiverPreferences receiver_preferences_;
protected:
    void push_package(Package&& package_to_emplace){bufor_.emplace(package_to_emplace.get_id());}
    std::optional<Package> bufor_ = std::nullopt; // bufor do przechowywania półproduktów gotywch do wysłania od robotnika/ od rampy
};


class Worker: public PackageSender, public IPackageReceiver // przetwarzają półprodukty od ramp
{
public:
    Worker(ElementID id, TimeOffset pd, std::unique_ptr<PackageQueue> q) : PackageSender(), id_{id}, time_{pd},queue_{std::move(q)} {} // pracownicy przyjmują preferencje stałą na typ FIFO
    void do_work(Time t); //argument to biezący czas symulacji robotnik ma go zapamiętać by wiedzieć kiedy rozpoczął przetwarzanie i kiedy skończyć na podstawie "pd" reprezentującego czas przetwarzania produktu
    TimeOffset get_processing_duration() const {return time_;}
    Time get_package_processing_start_time() const {return time_this_package_;}

    void receive_package(Package&& p) override;
    ElementID get_id() const override { return id_; }
    //ReceiverType get_receiver_type() const override { return ReceiverType::WORKER; };
    const std::optional<Package>& get_processing_buffer() const { return bufor_; }


    IPackageStockpile::const_iterator begin() const override {return queue_->begin();}
    IPackageStockpile::const_iterator cbegin() const override {return queue_->cbegin();}
    IPackageStockpile::const_iterator end() const override {return  queue_->end();}
    IPackageStockpile::const_iterator cend() const override {return queue_->cend();}
    // get_reciver_type()
    IPackageQueue* get_queue() const { return queue_.get(); }


private:
    ElementID id_;
    TimeOffset time_;
    Time time_this_package_; // czas robienia aktualniej paczki
    std::unique_ptr<IPackageQueue> queue_; // kolejka z której są pobierane produkty
    std::optional<Package> bufor_ = std::nullopt; // przedmiot który atualnie jest przetwarzany

};

class Ramp: public PackageSender // Dostarczanie do fabryki półproduktów do obróbki
{
public:
    Ramp(ElementID id, TimeOffset di): PackageSender(), id_{id}, time_{di}, t_() {
    }
    void deliver_goods(Time t);
    TimeOffset get_delivery_interval() const { return time_; } // tu i w niektórych miejscach jest const bo metoda nie zmienia stanu obiektu  na diagramach UML oznaczone jako {query}
    ElementID get_id() const { return id_; }
private:
    ElementID id_;
    TimeOffset time_; // czas od ostatniej dostawy
    Time t_;  // bierzący czas symulacji
    // na ich podstawie będzie wiadomo kiedy na rampie pojawi się kolejny produkt
    std::optional<Package> buffor_ = std::nullopt; // specjalny kontener który może być pusty
};


class Storehouse: public IPackageReceiver // przechowują półprodukty wykonane przez robotników
{
public:
    Storehouse(ElementID id, std::unique_ptr<IPackageStockpile> d = std::make_unique<PackageQueue>(PackageQueueType::FIFO)): id{id}, queue_{std::move(d)} {}

    void receive_package(Package&& p) override;

    IPackageStockpile::const_iterator begin() const override {return queue_->begin();}
    IPackageStockpile::const_iterator cbegin() const override {return queue_->cbegin();}
    IPackageStockpile::const_iterator end() const override {return  queue_->end();}
    IPackageStockpile::const_iterator cend() const override {return queue_->cend();}
    ElementID get_id() const override { return id; }

    //ReceiverType get_receiver_type() const override { return ReceiverType::STOREHOUSE; };
    IPackageStockpile* get_queue() const { return queue_.get(); }
private:
    ElementID id;
    std::unique_ptr<IPackageStockpile> queue_; //  referencja do magazynu z półproduktami
};

#endif //NODES_H