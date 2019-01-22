#pragma once

#include "nodecore.h"
#include <vector>

class node_registry
{
    std::vector<topic_info> topics;

public:
    // Get the number of open topics on the system
    u32 num_topics();

    // This function retrieves the topic info based on the index, the 
    // info parameter is output. The function returns false if there is no 
    // topic on that index
    bool get_topic_info(u32 topic_index, topic_info& info);

    // Create a new topic on the system, with the information on info
    bool create_topic(const topic_info& info);

    // Get information on a topic on the system, based on the name of the topic.
    // returns false if there is no topic with that name
    bool get_topic_info(const std::string& name, topic_info& info);
};