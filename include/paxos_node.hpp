// #include "paxos_message.hpp"
#include "message.hpp"
#include <functional>
#include <vector>
#include "rpc_client.hpp"
#include "heartbeat_message.hpp"

class PaxosNode{
    public:

        std::string node_id;
        std::string node_ip;
        int node_port;
        bool is_proposer;

        RPCClient rpc_client;

        struct PaxosNodeDescriptor{
            std::string node_id;
            std::string node_ip;
            int node_port;
            long long last_seen;
            bool is_alive;
        };

        std::vector<PaxosNodeDescriptor> cluster_nodes;

        struct PaxosNodeSharableDescriptor{
            std::string node_id;
            std::string node_ip;
            int node_port;
        };

        PaxosNode(std::string node_id,
                    std::string node_ip,
                    int node_port,
                    bool is_proposer);

        // start server
        void start(int port,
                    std::function<void(const Message&)> handler);
        
        // universal send message impl
        bool send_message(const std::string& node_addr,
                            int port,
                            const Message& msg);
        // universal receive message impl
        void receive_message();

        // join cluster helper
        void join_cluster(const std::vector<PaxosNodeSharableDescriptor>& initial_nodes);

        // handlers to send msgs
        bool send_heart_beat(std::string node_ip, int node_port, HeartBeatMessage msg);  // to be called every K seconds on a seperate thread
        void broadcast_heart_beat(HeartBeatMessage msg); // to send pulse to all nodes
        // std::string send_prepare(PrepareMessage msg);
        // std::string send_promise(PromiseMessage msg);
        // std::string send_accept(AcceptMessage msg);
        // std::string send_accepted(AcceptedMessage msg);

        
        // handler functions for received msgs
        void receive_heart_beat(HeartBeatMessage msg);
        // void receive_prepare(PrepareMessage msg);
        // void recieve_promise(PromiseMessage msg);
        // void receive_accept(AcceptMessage msg);
        // void receive_accepted(AcceptedMessage msg);


        // handler for heart beat
        bool is_node_alive(const std::string& node_id);
        bool update_last_seen(const HeartBeatMessage& msg);    // updates the node status
        void add_new_node(PaxosNodeSharableDescriptor new_node); // add new node to the cluster if new heart beat received
        void remove_node(PaxosNodeSharableDescriptor node); // remove existing node from the cluster if no heart beat received

        // // handler for prepare
        // // handler for promise
        // // handler for accept
        // void make_kv_batch();   // uses getter/setter to prepare a message body
        // // handler for accepted
        // void settle_kv_batch(); // uses getter/setter to save the latest data locally

        // // getter and setter for batch data
        // void get_key_batch();   // used to send all the keys to be synched, must use a non blocking store api
        // void get_kv_batch();    // used to send all the key-value pair to be synched, must use a non blocking store api
        // void set_kv_batch();    // used to save the received key-value pair to be synched, must use a non blocking store api

        ~PaxosNode(){};    
};