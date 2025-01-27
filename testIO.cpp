#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "factory.hxx"
#include "nodes.hxx"
#include "factory.hxx"
#include "package.hxx"

#include "storage_types.hxx"
#include "types.hxx"
using ::testing::ContainerEq;
#include <set>
using ::std::cout;
using ::std::endl;





TEST(FactoryTest, IsConsistentCorrect) {
    

    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));

    Ramp& r = *(factory.find_ramp_by_id(1));
    r.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(1)));

    Worker& w = *(factory.find_worker_by_id(1));
    w.receiver_preferences_.add_receiver(&(*factory.find_storehouse_by_id(1)));

    EXPECT_TRUE(factory.is_consistent());
}

TEST(FactoryTest, IsConsistentMissingLink1) {
    
    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));

    Ramp& r = *(factory.find_ramp_by_id(1));
    r.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(1)));

    Worker& w = *(factory.find_worker_by_id(1));
    w.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(1)));

    EXPECT_FALSE(factory.is_consistent());
}

TEST(FactoryTest, IsConsistentMissingLink2) {
    

    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_storehouse(Storehouse(1));

    Ramp& r = *(factory.find_ramp_by_id(1));
    r.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(1)));

    Worker& w1 = *(factory.find_worker_by_id(1));
    w1.receiver_preferences_.add_receiver(&(*factory.find_storehouse_by_id(1)));
    w1.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(2)));

    Worker& w2 = *(factory.find_worker_by_id(2));
    w2.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(2)));

    EXPECT_FALSE(factory.is_consistent());
}

TEST(FactoryTest, RemoveWorkerNoSuchReceiver) {
    

    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));

    Ramp& r = *(factory.find_ramp_by_id(1));
    Worker& w = *(factory.find_worker_by_id(1));
    r.receiver_preferences_.add_receiver(&w);

    Worker w2(2, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO));

    factory.remove_worker(w2.get_id());

    auto prefs = r.receiver_preferences_.get_preferences();
    ASSERT_EQ(prefs.size(), 1U);

    auto it = prefs.find(&w);
    ASSERT_NE(it, prefs.end());
    EXPECT_EQ(it->second, 1.0);
}

TEST(FactoryTest, RemoveWorkerOnlyOneReceiver) {

    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));

    Ramp& r = *(factory.find_ramp_by_id(1));
    Worker& w = *(factory.find_worker_by_id(1));
    r.receiver_preferences_.add_receiver(&w);

    factory.remove_worker(w.get_id());

    auto prefs = r.receiver_preferences_.get_preferences();
    ASSERT_TRUE(prefs.empty());
}

TEST(FactoryTest, RemoveWorkerTwoRemainingReceivers) {

    Factory factory;
    factory.add_ramp(Ramp(1, 1));
    factory.add_worker(Worker(1, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_worker(Worker(2, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));
    factory.add_worker(Worker(3, 1, std::make_unique<PackageQueue>(PackageQueueType::FIFO)));

    Ramp& r = *(factory.find_ramp_by_id(1));

    r.receiver_preferences_.add_receiver(&(*(factory.find_worker_by_id(1))));
    r.receiver_preferences_.add_receiver(&(*(factory.find_worker_by_id(2))));
    r.receiver_preferences_.add_receiver(&(*(factory.find_worker_by_id(3))));



    factory.remove_worker(1);

    auto prefs = r.receiver_preferences_.get_preferences();
    ASSERT_EQ(prefs.size(), 2U);

    auto it = prefs.find(&(*(factory.find_worker_by_id(2))));
    ASSERT_NE(it, prefs.end());
    EXPECT_DOUBLE_EQ(it->second, 1.0 / 2.0);

    it = prefs.find(&(*(factory.find_worker_by_id(3))));
    ASSERT_NE(it, prefs.end());
    EXPECT_DOUBLE_EQ(it->second, 1.0 / 2.0);
}
TEST(FactoryIOTest, ParseRamp) {
    std::istringstream iss("LOADING_RAMP id=1 delivery-interval=3");
    auto factory = load_factory_structure(iss);

    ASSERT_EQ(std::next(factory.ramp_cbegin(), 1), factory.ramp_cend());
    const auto& r = *(factory.ramp_cbegin());
    EXPECT_EQ(1, r.get_id());
    EXPECT_EQ(3, r.get_delivery_interval());
}

TEST(FactoryIOTest, ParseWorker) {
    std::istringstream iss("WORKER id=1 processing-time=2 queue-type=FIFO");
    auto factory = load_factory_structure(iss);

    ASSERT_EQ(std::next(factory.worker_cbegin(), 1), factory.worker_cend());
    const auto& w = *(factory.worker_cbegin());
    EXPECT_EQ(1, w.get_id());
    EXPECT_EQ(2, w.get_processing_duration());
    EXPECT_EQ(PackageQueueType::FIFO, w.get_queue()->get_queue_type());
}

TEST(FactoryIOTest, ParseStorehouse) {
    std::istringstream iss("STOREHOUSE id=1");
    auto factory = load_factory_structure(iss);

    ASSERT_EQ(std::next(factory.storehouse_cbegin(), 1), factory.storehouse_cend());
    const auto& s = *(factory.storehouse_cbegin());
    EXPECT_EQ(1, s.get_id());
}

TEST(FactoryIOTest, ParseLinkOneReceiver) {
    std::ostringstream oss;
    oss << "LOADING_RAMP id=1 delivery-interval=3" << "\n"
        << "STOREHOUSE id=1" << "\n"
        << "LINK src=ramp-1 dest=store-1" << "\n";
    std::istringstream iss(oss.str());
    auto factory = load_factory_structure(iss);

    ASSERT_EQ(std::next(factory.ramp_cbegin(), 1), factory.ramp_cend());
    const auto& r = *(factory.ramp_cbegin());

    ASSERT_EQ(std::next(factory.storehouse_cbegin(), 1), factory.storehouse_cend());
    const auto& s = *(factory.storehouse_cbegin());

    auto prefs = r.receiver_preferences_.get_preferences();
    ASSERT_EQ(1U, prefs.size());
    auto key = dynamic_cast<IPackageReceiver*>(const_cast<Storehouse*>(&s));
    ASSERT_NE(prefs.find(key), prefs.end());
    EXPECT_DOUBLE_EQ(prefs[key], 1.0);
}

   


TEST(FactoryIOTest, ParseLinkMultipleReceivers) {
    std::ostringstream oss;
    oss << "LOADING_RAMP id=1 delivery-interval=3" << "\n"
        << "STOREHOUSE id=1" << "\n"
        << "STOREHOUSE id=2" << "\n"
        << "LINK src=ramp-1 dest=store-1" << "\n"
        << "LINK src=ramp-1 dest=store-2" << "\n";
    std::istringstream iss(oss.str());
    auto factory = load_factory_structure(iss);

    ASSERT_EQ(std::next(factory.ramp_cbegin(), 1), factory.ramp_cend());
    const auto& r = *(factory.ramp_cbegin());

    ASSERT_EQ(std::next(factory.storehouse_cbegin(), 2), factory.storehouse_cend());
    const auto& s1 = *(factory.storehouse_cbegin());
    const auto& s2 = *(std::next(factory.storehouse_cbegin(), 1));

    auto prefs = r.receiver_preferences_.get_preferences();
    ASSERT_EQ(2U, prefs.size());
    auto key1 = dynamic_cast<IPackageReceiver*>(const_cast<Storehouse*>(&s1));
    auto key2 = dynamic_cast<IPackageReceiver*>(const_cast<Storehouse*>(&s2));
    ASSERT_NE(prefs.find(key1), prefs.end());
    ASSERT_NE(prefs.find(key2), prefs.end());
    EXPECT_DOUBLE_EQ(prefs[key1], 0.5);
    EXPECT_DOUBLE_EQ(prefs[key2], 0.5);
}



TEST(FactoryIOTest, LoadAndSaveTest) {
    std::string r1 = "LOADING_RAMP id=1 delivery-interval=3";
    std::string r2 = "LOADING_RAMP id=2 delivery-interval=2";
    std::string w1 = "WORKER id=1 processing-time=2 queue-type=FIFO";
    std::string w2 = "WORKER id=2 processing-time=1 queue-type=LIFO";
    std::string s1 = "STOREHOUSE id=1";
    std::string l1 = "LINK src=ramp-1 dest=worker-1";
    std::string l2 = "LINK src=ramp-2 dest=worker-1";
    std::string l3 = "LINK src=ramp-2 dest=worker-2";
    std::string l4 = "LINK src=worker-1 dest=worker-1";
    std::string l5 = "LINK src=worker-1 dest=worker-2";
    std::string l6 = "LINK src=worker-2 dest=store-1";

    std::set<std::string> input_set = {r1, r2, w1, w2, s1, l1, l2, l3, l4, l5, l6};

    std::vector<std::string> input_lines{
            "; == LOADING RAMPS ==",
            "",
            r1,
            r2,
            "",
            "; == WORKERS ==",
            "",
            w1,
            w2,
            "",
            "; == STOREHOUSES ==",
            "",
            s1,
            "",
            "; == LINKS ==",
            "",
            l1,
            "",
            l2,
            l3,
            "",
            l4,
            l5,
            "",
            l6,
    };

   

    std::string factoryStructureStr = std::accumulate(input_lines.begin(), input_lines.end(), std::string(""),
                                                      [](std::string accu, std::string line) {
                                                          return accu + line + "\n";
                                                      });

    std::istringstream iss(factoryStructureStr);
    Factory factory;
    try {
        factory = load_factory_structure(iss);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        FAIL();
    } catch (...) {
        std::cerr << "Unknown error..." << std::endl;
        FAIL();
    }

    std::ostringstream oss;
    save_factory_structure(factory, oss);

    std::vector<std::string> output_lines;
    //
    std::string structure_str = oss.str();

    iss.str(structure_str);
    iss.clear();
    std::string line;
    while (std::getline(iss, line, '\n')) {
        bool is_blank_line = line.empty() || isblank(line[0]);
        bool is_comment_line = !line.empty() && line[0] == ';';
        if (is_blank_line || is_comment_line) {
            continue;
        }

        output_lines.push_back(line);
    }

    std::set<std::string> output_set(output_lines.begin(), output_lines.end());
    ASSERT_EQ(output_set.size(), output_lines.size()) << "Duplicated lines in the output.";

    EXPECT_THAT(output_set, ContainerEq(input_set));

    
    auto first_occurence = [&output_lines](std::string label) {
        return std::find_if(output_lines.begin(), output_lines.end(),
                            [label](const std::string s) { return s.rfind(label, 0) == 0; });
    };

    auto first_ramp_it = first_occurence("LOADING_RAMP");
    auto first_worker_it = first_occurence("WORKER");
    auto first_storehouse_it = first_occurence("STOREHOUSE");
    auto first_link_it = first_occurence("LINK");

    ASSERT_LT(first_ramp_it, first_worker_it);
    ASSERT_LT(first_worker_it, first_storehouse_it);
    ASSERT_LT(first_storehouse_it, first_link_it);
}
TEST(PackageTest, IsAssignedIdLowest) {
    // przydzielanie ID o jeden większych -- utworzenie dwóch obiektów pod rząd

    Package p1;
    Package p2;

    EXPECT_EQ(p1.get_id(), 1);
    EXPECT_EQ(p2.get_id(), 2);
}

TEST(PackageTest, IsIdReused) {
    // przydzielanie ID po zwolnionym obiekcie

    {
        Package p1;
    }
    Package p2;

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsMoveConstructorCorrect) {
    Package p1;
    Package p2(std::move(p1));

    EXPECT_EQ(p2.get_id(), 1);
}

TEST(PackageTest, IsAssignmentOperatorCorrect) {
    Package p1;
    Package p2 = std::move(p1);

    EXPECT_EQ(p2.get_id(), 1);
}
TEST(PackageQueueTest, IsFifoCorrect) {
    PackageQueue q(PackageQueueType::FIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 1);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 2);
}

TEST(PackageQueueTest, IsLifoCorrect) {
    PackageQueue q(PackageQueueType::LIFO);
    q.push(Package(1));
    q.push(Package(2));

    Package p(std::move(q.pop()));
    EXPECT_EQ(p.get_id(), 2);

    p = q.pop();
    EXPECT_EQ(p.get_id(), 1);
}
