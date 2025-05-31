#include "kivileaf/kivileaf_rest_controller.hpp"
#include <iostream>
#include "kivitree_utils/json.hpp"

using json = nlohmann::json;

KiviLeafRestController::KiviLeafRestController(KiviLeaf* leaf_node)
    : leaf(leaf_node) {}

void KiviLeafRestController::start(int rest_port) {
    std::thread([this, rest_port]() {
        httplib::Server svr;

        register_routes(svr);

        std::cout << "[REST] KiviLeaf REST API at http://localhost:" << rest_port << "\n";
        svr.listen("0.0.0.0", rest_port);
    }).detach();
}

static auto log_request = [](const std::string& endpoint, const httplib::Request& req) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::cout << "[REST] [" << std::put_time(std::localtime(&now_time), "%F %T") << "] "
              << req.method << " " << endpoint;

    if (!req.params.empty()) {
        std::cout << "?";
        bool first = true;
        for (const auto& [key, val] : req.params) {
            if (!first) std::cout << "&";
            std::cout << key << "=" << val;
            first = false;
        }
    }

    std::cout << std::endl;
};

void KiviLeafRestController::register_routes(httplib::Server& svr) {

    svr.Get("/get", [this](const httplib::Request& req, httplib::Response& res) {
        log_request("/get", req);
        if (!req.has_param("key")) {
            json error = {{"error", "Missing 'key' parameter"}};
            res.status = 400;
            res.set_content(error.dump(), "application/json");
            return;
        }

        std::string key = req.get_param_value("key");
        auto val_opt = leaf->kivi.get(key);

        if (val_opt.has_value()) {
            json result = {{"key", key}, {"value", val_opt.value()}};
            res.set_content(result.dump(), "application/json");
        } else {
            json error = {{"error", "Key not found"}};
            res.status = 404;
            res.set_content(error.dump(), "application/json");
        }
    });

    svr.Put("/put", [this](const httplib::Request& req, httplib::Response& res) {
        
        log_request("/put", req);

        std::string key = req.get_param_value("key");
        std::string val = req.get_param_value("value");

        if (key.empty() || val.empty()) {
            json error = {{"error", "Missing 'key' or 'value' parameter"}};
            res.status = 400;
            res.set_content(error.dump(), "application/json");
            return;
        }

        leaf->kivi.put(key, val);
        json result = {{"status", "OK"}, {"key", key}};
        res.set_content(result.dump(), "application/json");
    });

    svr.Get("/print", [this](const httplib::Request& req, httplib::Response& res) {
        log_request("/print", req);

        auto data = leaf->kivi.get_kv_batch();
        json result = data;  // implicit conversion to json object
        res.set_content(result.dump(4), "application/json");  // pretty-printed with 4-space indent
    });
}
