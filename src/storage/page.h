#pragma once
#include <cstdint>

// how large we want our pages to be so that we can do paging and write them to disk so our database can persist
constexpr uint32_t PAGE_SIZE = 4096;

// defining an equivalent type just for readability
using page_id = uint32_t;

// the invalid id is when its just 32 bits of straight 1s lets us know when we have reached the end of our addressing
constexpr page_id INVALID_PAGE = 0xFFFFFFFF;

// can help us keep track what each of our pages represent like the type of node
enum class PageType {
    INVALID,
    LEAF,
    INTERNAL
};


// basic page header
struct PageHeader {
    PageType type;
    uint16_t numKeys;
    page_id pageId;
    page_id parentId;
};

struct LeafPage {
    PageHeader header;
    // since leafs always have a pointer to next leaf this just represents that but instead of a pointer its the id of the next node
    page_id nextLeaf;
    // remaining space filled up by all the key, value pairs that are normal of a datbase
};

struct InternalPage {
    PageHeader header;
    // remaining space filled with the children + keys
};

