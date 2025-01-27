#ifndef NETSIM_FACTORY_HPP
#define NETSIM_FACTORY_HPP
#include "types.hxx"
#include "nodes.hxx"

enum class State{
    VISITED,
    UNVISITED,
    VERYFIED
};

template<class Node>
class NodeCollection
{
public:
    using container_t = typename std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;
    iterator begin() { return nodes_.begin(); }
    iterator end() { return nodes_.end(); }
    const_iterator cbegin() const { return nodes_.cbegin(); }
    const_iterator cend() const { return nodes_.cend(); }

    NodeCollection<Node>::iterator find_by_id( ElementID id){
        return std::find_if(nodes_.begin(),--nodes_.end(),[id](const auto& node){return node.get_id() == id;});
    }
    NodeCollection<Node>::const_iterator find_by_id( ElementID id) const{
        return std::find_if(nodes_.begin(),--nodes_.end(),[id](const auto& node){return node.get_id() == id;});
    }

    void add(Node&& node){
        nodes_.emplace_back(std::move(node));
        nodes_.sort(); 
    }
    bool remove_by_id(ElementID id) {
        bool if_removed_flag = false;
        const_iterator it = find_by_id(id);
        if (it->get_id() == id) {
            nodes_.erase(it);
            if_removed_flag = true;
        }

        return if_removed_flag;
    }

private:
    container_t nodes_;
};


class Factory{
public:
    void add_ramp(Ramp&& ramp_to_add) {collection_ramps_.add(std::move(ramp_to_add));};
    void add_worker(Worker&& worker_to_add) {collection_workers_.add(std::move(worker_to_add));};
    void add_storehouse(Storehouse&& storehouse_to_add){collection_storehouses_.add(std::move(storehouse_to_add));};
    void remove_ramp(ElementID id) {collection_ramps_.remove_by_id(id);};
    void remove_worker(ElementID id) {remove_receiver(collection_workers_,id);};
    void remove_storehouse(ElementID id) {remove_receiver(collection_storehouses_,id);};
    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) {return collection_ramps_.find_by_id(id);};
    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) {return collection_workers_.find_by_id(id);};
    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) {return collection_storehouses_.find_by_id(id);};
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const {return collection_ramps_.find_by_id(id);};
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const {return collection_workers_.find_by_id(id);};
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const {return collection_storehouses_.find_by_id(id);};
    NodeCollection<Ramp>::const_iterator ramp_cbegin() const {return collection_ramps_.cbegin();};
    NodeCollection<Worker>::const_iterator worker_cbegin() const {return collection_workers_.cbegin();};
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin()const {return collection_storehouses_.cbegin();};
    NodeCollection<Ramp>::const_iterator ramp_cend() const {return collection_ramps_.cend();};
    NodeCollection<Worker>::const_iterator worker_cend() const {return collection_workers_.cend();};
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const {return collection_storehouses_.cend();};
    bool is_consistent();
    void do_deliveries(Time);
    void do_package_passing();
    void do_work(Time);
private:
    NodeCollection<Worker> collection_workers_;
    NodeCollection<Ramp> collection_ramps_;
    NodeCollection<Storehouse> collection_storehouses_;

    template<class Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementID id);
    bool is_receiver_connected_to_storehouse(PackageSender* sender, std::map<PackageSender*,State> map);
};

template <class Node>
void Factory::remove_receiver(NodeCollection<Node> &collection, ElementID id) {
    Node* removing_receiver_pointer = &(*collection.find_by_id(id));
    bool if_removed_flag = collection.remove_by_id(id);
    if(if_removed_flag) {
        for (auto &worker_from_collection : collection_workers_) {
            worker_from_collection.receiver_preferences_.remove_receiver(removing_receiver_pointer);
        }
        for (auto &ramp_from_collection : collection_ramps_) {
            ramp_from_collection.receiver_preferences_.remove_receiver(removing_receiver_pointer);
        }
    }
}

// IO IO IO IO IO IO IO IO IO IO IO
enum class ElementType{
    LOADING_RAMP,
    WORKER,
    STOREHOUSE,
    LINK
};
enum class NodeType{
    RAMP,
    WORKER,
    STORE
};
static std::map<std::string, PackageQueueType> QueueTypeMap = {
        {"LIFO", PackageQueueType::LIFO},
        {"FIFO", PackageQueueType::FIFO}
};
static std::map<std::string, NodeType> NodeTypeMap = {
        {"ramp",NodeType::RAMP},
        {"worker",NodeType::WORKER},
        {"store",NodeType::STORE}
};
static std::map<std::string, ElementType> ElementTypeMap = {
        {"LOADING_RAMP",ElementType::LOADING_RAMP},
        {"WORKER",ElementType::WORKER},
        {"STOREHOUSE",ElementType::STOREHOUSE},
        {"LINK",ElementType::LINK}
};

typedef struct {
    ElementType element_type;
    std::map<std::string,std::string> parameters;
}ParsedLineData;

ParsedLineData parse_line(const std::string& line);
Factory load_factory_structure(std::istream& is);
void save_factory_structure(Factory& factory, std::ostream& os);


#endif

