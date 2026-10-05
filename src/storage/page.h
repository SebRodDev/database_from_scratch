#pragma once
#include <cstdint>
#include <type_traits>

constexpr uint32_t PAGE_SIZE  = 4096;
constexpr uint32_t VALUE_SIZE = 56;

using pageId = uint32_t;
using KeyType   = int64_t;
constexpr pageId INVALID_PAGE_ID = 0xFFFFFFFF;

// want to limit the amount of space that this is taking up
enum class PageType : uint8_t {
    INVALID,
    LEAF,
    INTERNAL
};

// padding so that we can get it exactly to the amount of bytes that we want and the compiler does not add padding that we are not explicitly setting
struct PageHeader {
    PageType  type;
    uint8_t   _pad0[3];
    uint16_t  num_keys;
    uint16_t  _pad1;
    pageId page_id;
    pageId parent_id;
};
static_assert(sizeof(PageHeader) == 16);

struct Value {
    char bytes[VALUE_SIZE]; 
};

// each leaf has a key and a value associated with it
struct LeafSlot {
    KeyType key;
    Value   value;
};
static_assert(sizeof(LeafSlot) == 64);

constexpr uint32_t LEAF_HEADER_SIZE = 64;
constexpr uint32_t LEAF_MAX_SLOTS   = (PAGE_SIZE - LEAF_HEADER_SIZE) / sizeof(LeafSlot);

struct LeafPage {
    PageHeader header;
    pageId  next_leaf;
    uint8_t    _reserved[LEAF_HEADER_SIZE - sizeof(PageHeader) - sizeof(pageId)];
    LeafSlot   slots[LEAF_MAX_SLOTS];
};

static_assert(sizeof(LeafPage) <= PAGE_SIZE);

// n keys + (n+1) children must fit: n*8 + (n+1)*4 <= PAGE_SIZE - header
constexpr uint32_t INTERNAL_MAX_KEYS =
    (PAGE_SIZE - sizeof(PageHeader) - sizeof(pageId)) /
    (sizeof(KeyType) + sizeof(pageId));

constexpr uint32_t INTERNAL_MAX_CHILDREN = INTERNAL_MAX_KEYS + 1;

// ensuring this can cleanly map to a 4096 (4 KB) buffer
struct InternalPage {
    PageHeader header;
    pageId children[INTERNAL_MAX_CHILDREN];
    KeyType    keys[INTERNAL_MAX_KEYS];
    uint8_t    _reserved[PAGE_SIZE - sizeof(PageHeader) 
                          - INTERNAL_MAX_CHILDREN * sizeof(pageId)
                          - INTERNAL_MAX_KEYS * sizeof(KeyType)];
};

static_assert(sizeof(InternalPage) <= PAGE_SIZE);

