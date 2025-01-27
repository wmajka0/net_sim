
#include "factory.hxx"
#include <istream>
#include <string>
#include <sstream>

void Factory::do_deliveries(Time t) {
    for (auto & ramp_from_collection : collection_ramps_){
        ramp_from_collection.deliver_goods(t);
    }
}

void Factory::do_work(Time t) {
    for (auto & worker_from_collection : collection_workers_){
        worker_from_collection.do_work(t);
    }
}

void Factory::do_package_passing() {
    for(auto & ramp_from_collection: collection_ramps_){
        ramp_from_collection.send_package();
    }
    for(auto & worker_from_collection : collection_workers_){
        worker_from_collection.send_package();
    }
}




bool Factory::is_consistent()
{
    std::map<PackageSender*,State> nodes_state_map;
    for (auto & ramp : collection_ramps_)
    {
        nodes_state_map.insert(std::pair<PackageSender*,State>(&ramp, State::UNVISITED));
    }
    for (auto & worker : collection_workers_)
    {
        nodes_state_map.insert(std::pair<PackageSender*,State>(&worker, State::UNVISITED));
    }
    bool consistent = false;
    for (auto & ramp : collection_ramps_)
    {
        try
        {
            consistent = is_receiver_connected_to_storehouse(&ramp, nodes_state_map);
        }
        catch (std::logic_error &)
        {
            return false;
        }
    }
    return consistent;
}


bool Factory::is_receiver_connected_to_storehouse(PackageSender* sender, std::map<PackageSender*,State> map)
{
    if (map.at(sender) == State::VERYFIED)
        return true;
    map.at(sender) = State::VISITED;

    if (sender->receiver_preferences_.preferences_.empty())
        throw std::logic_error("Error");

    bool sender_has_at_least_one_reciever = false;
    for (auto pair : sender->receiver_preferences_.preferences_)
    {
        IPackageReceiver* reciver = pair.first;
        if (reciver->get_receiver_type() == ReceiverType::STOREHOUSE)
            sender_has_at_least_one_reciever = true;
        else
        {
            IPackageReceiver* receiver_ptr = reciver;
            auto worker_ptr = dynamic_cast<Worker*>(receiver_ptr);
            auto sendrecv_ptr = dynamic_cast<PackageSender*>(worker_ptr);

            if (sendrecv_ptr == sender)
                break;
            sender_has_at_least_one_reciever = true;
            if (map.at(sendrecv_ptr) == State::UNVISITED)
                is_receiver_connected_to_storehouse(sendrecv_ptr, map);
        }
    }
    map.at(sender) = State::VERYFIED;

    if (sender_has_at_least_one_reciever)
        return true;
    else
        throw std::logic_error("Error");
}

ParsedLineData parse_line(const std::string& line) {
    ParsedLineData new_parsed_line_data;
    std::vector<std::string> tokens;
    std::string token;
    char delimiter = ' ';
    std::istringstream token_stream(line);
    while (std::getline(token_stream, token, delimiter))
        tokens.push_back(token);

    new_parsed_line_data.element_type = ElementTypeMap.at(tokens.front());
    tokens.erase(tokens.begin()); //Pozostawia same pary klucz-wartosc w wektorze tokens

    for (std::string& single_token : tokens) {
        std::vector<std::string> all_two_values_from_token;
        std::string single_value_from_token;
        char delimiter2 = '=';
        std::istringstream token_stream2(single_token);
        while (std::getline(token_stream2,single_value_from_token, delimiter2))
            all_two_values_from_token.push_back(single_value_from_token);

        auto pair_from_token = std::make_pair(all_two_values_from_token[0],all_two_values_from_token[1]);
        new_parsed_line_data.parameters.insert(pair_from_token);
    }
    return new_parsed_line_data;
}


Factory load_factory_structure(std::istream& is){
    Factory new_factory;
    std::string line;
    std::vector<ParsedLineData> parsed_line_data_collection;
    ParsedLineData parsed_line_data;
    while (std::getline(is, line)) {
        if (line.empty() or line.front() == ';')
            continue;
        try {
            parsed_line_data = parse_line(line);
        } catch (std::out_of_range &) {
            continue;
        }
        parsed_line_data_collection.push_back(parsed_line_data);
    }


    for (ParsedLineData& single_line_data : parsed_line_data_collection){
        switch (single_line_data.element_type){
            case ElementType::LOADING_RAMP: {
                ElementID id = static_cast<ElementID>(std::stoi(single_line_data.parameters.at("id")));
                TimeOffset di = static_cast<TimeOffset>(std::stoi(single_line_data.parameters.at("delivery-interval")));
                new_factory.add_ramp(Ramp(id, di));
                break;
            }
            case ElementType::WORKER: {
                ElementID id = static_cast<ElementID>(std::stoi(single_line_data.parameters.at("id")));
                TimeOffset pt = static_cast<TimeOffset>(std::stoi(single_line_data.parameters.at("processing-time")));
                std::string string_queue_type= single_line_data.parameters.at("queue-type");
                PackageQueueType queue_type = QueueTypeMap.at(string_queue_type);
                new_factory.add_worker(Worker(id,pt,std::make_unique<PackageQueue>(queue_type)));
                break;
            }
            case ElementType::STOREHOUSE: {
                ElementID id = static_cast<ElementID>(std::stoi(single_line_data.parameters.at("id")));
                new_factory.add_storehouse(Storehouse(id));
                break;
            }
            case ElementType::LINK: {
                std::string src = single_line_data.parameters.at("src");
                std::string dest = single_line_data.parameters.at("dest");
                std::istringstream src_stream(src);
                std::istringstream dest_stream(dest);
                std::vector<std::string> src_vector;
                std::vector<std::string> dest_vector;
                char delimiter = '-';
                std::string parsing_temp;
                while (getline(src_stream, parsing_temp, delimiter))
                    src_vector.push_back(parsing_temp);
                while (getline(dest_stream, parsing_temp, delimiter))
                    dest_vector.push_back(parsing_temp);
                NodeType src_type = NodeTypeMap.at(src_vector[0]);
                NodeType dest_type = NodeTypeMap.at(dest_vector[0]);
                auto src_id = static_cast<ElementID>(std::stoi(src_vector[1]));
                auto dest_id = static_cast<ElementID>(std::stoi(dest_vector[1]));
                if (src_type == NodeType::RAMP and dest_type == NodeType::WORKER) {
                    auto src_iterator = new_factory.find_ramp_by_id(src_id);
                    auto dest_iterator = new_factory.find_worker_by_id(dest_id);
                    src_iterator->receiver_preferences_.add_receiver(&(*dest_iterator));
                }
                if (src_type == NodeType::RAMP and dest_type == NodeType::STORE) {
                    auto src_iterator = new_factory.find_ramp_by_id(src_id);
                    auto dest_iterator = new_factory.find_storehouse_by_id(dest_id);
                    src_iterator->receiver_preferences_.add_receiver(&(*dest_iterator));
                }
                if (src_type == NodeType::WORKER and dest_type == NodeType::WORKER) {
                    auto src_iterator = new_factory.find_worker_by_id(src_id);
                    auto dest_iterator = new_factory.find_worker_by_id(dest_id);
                    src_iterator->receiver_preferences_.add_receiver(&(*dest_iterator));
                }
                if (src_type == NodeType::WORKER and dest_type == NodeType::STORE) {
                    auto src_iterator = new_factory.find_worker_by_id(src_id);
                    auto dest_iterator = new_factory.find_storehouse_by_id(dest_id);
                    src_iterator->receiver_preferences_.add_receiver(&(*dest_iterator));
                }
            }
        }
    }
    return new_factory;
}

void save_factory_structure(Factory& factory, std::ostream& os){
    os << "; == LOADING RAMPS ==" << std::endl << std::endl;
    std::for_each(factory.ramp_cbegin(),factory.ramp_cend(),[&os](const Ramp& ramp){
        os<<"LOADING_RAMP id="<<ramp.get_id()<<" delivery-interval="<<ramp.get_delivery_interval()<<std::endl;
    });
    os<<std::endl<<"; == WORKERS =="<<std::endl<<std::endl;
    std::for_each(factory.worker_cbegin(),factory.worker_cend(),[&os](const Worker& worker){
        std::string queue_type;
        switch (worker.get_queue()->get_queue_type()){
            case PackageQueueType::FIFO: {
                queue_type="FIFO";
                break;
            }
            case PackageQueueType::LIFO: {
                queue_type="LIFO";
                break;
            }
        }
        os<<"WORKER id="<<worker.get_id()<<" processing-time="<<worker.get_processing_duration()<<" queue-type="<<queue_type<<std::endl;
    });
    os<<std::endl<<"; == STOREHOUSES =="<<std::endl<<std::endl;
    std::for_each(factory.storehouse_cbegin(),factory.storehouse_cend(),[&os](const Storehouse& storehouse){
        os<<"STOREHOUSE id="<<storehouse.get_id()<<std::endl;
    });
    os<<std::endl<<"; == LINKS =="<<std::endl<<std::endl;
    std::for_each(factory.ramp_cbegin(),factory.ramp_cend(),[&os](const Ramp& ramp){
        for(auto& preferences : ramp.receiver_preferences_.get_preferences()){
            IPackageReceiver* receiver_ptr = preferences.first;
            switch(receiver_ptr->get_receiver_type()){
                case ReceiverType::WORKER: {
                    os<<"LINK src=ramp-"<<ramp.get_id()<<" dest=worker-"<<receiver_ptr->get_id()<<std::endl;
                    break;
                }
                case ReceiverType::STOREHOUSE: {
                    os<<"LINK src=ramp-"<<ramp.get_id()<<" dest=store-"<<receiver_ptr->get_id()<<std::endl;
                    break;
                }
            }
        }
        os<<std::endl;
    });
    std::for_each(factory.worker_cbegin(),factory.worker_cend(),[&os](const Worker& worker){
        for(auto& preferences : worker.receiver_preferences_.get_preferences()){
            IPackageReceiver* receiver_ptr = preferences.first;
            switch(receiver_ptr->get_receiver_type()){
                case ReceiverType::WORKER: {
                    os<<"LINK src=worker-"<<worker.get_id()<<" dest=worker-"<<receiver_ptr->get_id()<<std::endl;
                    break;
                }
                case ReceiverType::STOREHOUSE: {
                    os<<"LINK src=worker-"<<worker.get_id()<<" dest=store-"<<receiver_ptr->get_id()<<std::endl;
                    break;
                }
            }
        }
        os<<std::endl;
    });
    os.flush();
}
