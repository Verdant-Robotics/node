#pragma once
#include "mytypes.h"
#include <string>
#include "nodelib.h"
#include "circular_buffer.h"

void* helper_open_channel(const channel_info& info, int& mem_fd);
void  helper_clean(void *addr, int mem_fd, u32 mem_length);

template< class T>
class topic_producer
{
    circular_buffer *indices = nullptr; // Indices should be allocated inside of data... 
    u8* data = nullptr;
    int mem_fd = 0;
    u32 mem_length = 0;
    T*  elems = nullptr;

public:
        
    // this function will open the channel, allocate memory, set the indices, and
    // do any other needed initialization
    bool open_channel(const std::string &topic_name, int num_elems = 4)
    {
        bool bret = false;
        // Find the registry, inquire about this channel
        nodelib node_lib;
        topic_info info;

        bret = node_lib.open();
        if (!bret) {
            return false;
        }

        u32 sz = (sizeof(circular_buffer) + num_elems*sizeof(T));
        int reminder = sz / 1024;
        if (reminder != 0) {
            sz = sz + 1024 - reminder;
        }

        info.name = topic_name;
        info.message_name = T::TYPE_STRING;
        info.message_hash = T::TYPE_HASH;
        info.cn_info.channel_path = "/node_";
        info.cn_info.channel_path += info.message_hash;
        info.cn_info.channel_size = sz;
        bret = node_lib.create_topic(info);
        if (!bret) {
            return false;
        }
        printf("New topic created successfully\n");
        
        // Now we have the topic info on info
        data = (u8 *)helper_open_channel(info.cn_info, mem_fd);
        if (!data) {
            printf("Could not open the shared memory\n");
            return false;
        }

        mem_length = info.cn_info.channel_size;

        // do setup of stuff in data now!
        indices = (circular_buffer *)data;
        elems = (T *)( (u8 *)data + sizeof(circular_buffer));
        indices->initialize(num_elems);

        bret = node_lib.make_topic_visible(topic_name);
        if (!bret) {
            return false;
        }
        return true;
    }
        
    // Producer: get a pointer to a struct to fill
    T* get_slot()
    {
        // This call might block
        unsigned int elem_index = indices->get_next_empty();
        
        return &elems[elem_index];
    }
    
    // Producer: This function assumes that the image* previously returned will no longer be used
    void publish()
    {
        indices->publish();
    }
            
    // This function will do a resize .. TO BE DONE
    bool resize()
    {
        return true;
    }

    ~topic_producer()
    {
        helper_clean(data, mem_fd, mem_length);
        data = nullptr;
        mem_length = 0;
        mem_fd = 0;
        elems = nullptr;
        indices = nullptr;
    }
};


template< class T>
class topic_consumer
{
    circular_buffer *indices = nullptr; // Indices are allocated inside of data
    u32 cons_index = 0; // Indicate which consumer this is, out of multiple in a topic
    u8* data = nullptr;
    int mem_fd = 0;
    u32 mem_length = 0;
    T*  elems = nullptr;

public:
        
    // this function will open the channel, allocate memory, set the indices, and
    // do any other needed initialization
    bool open_channel(const std::string &topic_name)
    {
        bool bret = false;
        // Find the registry, inquire about this channel
        nodelib node_lib;
        topic_info info;

        bret = node_lib.open();
        if (!bret) {
            fprintf(stderr, "Failure to open the node registry\n");
            return false;
        }

        bret = node_lib.get_topic_info(topic_name, info);
        if (!bret) {
            // Consumer cannot create new topics
            fprintf(stderr, "Failure to find the topic in the node registry\n");
            return false;
        } 
        
        // Now we have the topic info on info
        data = (u8 *)helper_open_channel(info.cn_info, mem_fd);
        if (!data) {
            fprintf(stderr, "Failure to open the shared memory\n");
            return false;
        }

        mem_length = info.cn_info.channel_size;

        printf("Opened channel with %d length, %s path", mem_length, 
            info.cn_info.channel_path.c_str());

        // do setup of stuff in data now!
        indices = (circular_buffer *)data;
        elems = (T *)( (u8 *)data + sizeof(circular_buffer));        

        cons_index = indices->get_cons_number();

        indices->initialize_consumer(cons_index);
        return true;
    }
            
    // Consumer: get a pointer to the next struct from the publisher
    T* get_slot()
    {
        // This call might block
        // TODO: how to handle multiple consumers?
        unsigned int elem_index = indices->get_next_full(cons_index);
        
        return &elems[elem_index];
    }
    
    // Consumer: This function assumes that the image* previously returned will no longer be used
    void release()
    {
        indices->release(cons_index);
    }
    
    // This function will do a resize .. TO BE DONE
    bool resize()
    {
        return true;
    }

    ~topic_consumer()
    {
        helper_clean(data, mem_fd, mem_length);
        data = nullptr;
        mem_length = 0;
        mem_fd = 0;
        elems = nullptr;
        indices = nullptr;
    }

};
