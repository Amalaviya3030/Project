#include <stdio.h>


#define HASH_SIZE 127
#define MAX_COUNTRY_LENGTH 21
#define MAX_LINE_LENGTH 256

// Parcel body structure
typedef struct Parcel {
    char* destination;
    int weight;
    float value;
} Parcel;

// Treenode structure for tree
typedef struct TreeNode {
    Parcel* parcel;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// HashTable structure
typedef struct HashTable {
    TreeNode* buckets[HASH_SIZE];
} HashTable;

//enums for errors
typedef enum {
    PARSE_SUCCESS,
    PARSE_MEMORY_ERROR,
    PARSE_FORMAT_ERROR
} ParseResult;

// main body function
int main() {
    return 0;
}