#pragma once
#include <string>
#include <vector>
#include <tuple>
#include "kivitree_utils/json.hpp"
#include "kivitree_utils/timestamp.hpp"
using json = nlohmann::json;

class LocalSyncPayload{
    std::vector<std::tuple<std::string, std::string, long long int>> updates;

    public:
        LocalSyncPayload()=default;
        std::string dump() const{
            json j;
            for (const auto& [k, v, ts] : updates) {
                j.push_back({{"key", k}, {"value", v}, {"timestamp", ts}});
            }
            return j.dump();
        }

        std::unordered_map<std::string, std::string> get_payload() const{
            std::unordered_map<std::string, std::string> kv_pair;

            for(const auto& [k,v,ts]: updates){
                kv_pair[k]=v;
            }

            return kv_pair;
        }

        void load(const std::string& serialized) {
            updates.clear();
            json j = json::parse(serialized);
            for (const auto& item : j) {
                updates.emplace_back(
                    item.at("key").get<std::string>(),
                    item.at("value").get<std::string>(),
                    item.at("timestamp").get<long long>()
                );
            }
        }

        void load_from_map(const std::unordered_map<std::string, std::string> data){
            updates.clear();

            long long int ts = Timestamp::now_ms();

            for(const auto& item: data){
                updates.emplace_back(
                    item.first,
                    item.second,
                    ts
                );
            }
        }

        const std::vector<std::tuple<std::string, std::string, long long>>& get_updates() const {
            return updates;
        }

        void add_update(const std::string& key, const std::string& value, long long ts) {
            updates.emplace_back(key, value, ts);
        }
};