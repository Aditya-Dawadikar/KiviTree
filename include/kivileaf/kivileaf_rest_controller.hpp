#pragma once
#include "kivitree_utils/httplib.h"
#include "kivileaf.hpp"

class KiviLeafRestController {
public:
    KiviLeafRestController(KiviLeaf* leaf_node);
    void start(int rest_port);

private:
    KiviLeaf* leaf;
    void register_routes(httplib::Server& svr);
};
