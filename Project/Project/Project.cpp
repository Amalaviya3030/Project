/*
FILE       : project
PROJECT : project
PROGRAMMER : ANCHITA KAKRIA(8961585) , ARYANKUMAT MALAVIYA(8963030) 
FIRSTVERSION : 11 AUG 2024
DESCRIPTION :This C program manages courier parcel data using Hash Tables and Binary Search Trees.
It will read parcel details from the txt file and enables users to get info based on country, weight and value.
*/


#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning(disable : 4996)


#define HASH_SIZE 127
#define MAX_COUNTRY_LENGTH 21
#define MAX_LINE_LENGTH 256
#define HANDLE_ERROR(msg) do { perror(msg); longjmp(error_buf, 1); } while (0)

static jmp_buf error_buf;

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

// Function prototypes

unsigned long hash_djb2(const char* str);
TreeNode* create_tree_node(Parcel* parcel);
void insert_tree_node(TreeNode** root, Parcel* parcel);
HashTable* create_hash_table();
void insert_parcel(HashTable* table, Parcel* parcel);
void trim(char* str);
static ParseResult parse_line(const char* line, int line_number, Parcel** out_parcel);
int read_file_and_populate_hash_table(HashTable* table, const char* filename);
int count_tree_nodes(TreeNode* root);
void display_bucket_counts(HashTable* table);
void clean_up_tree(TreeNode* root);
void clean_up_hash_table(HashTable* table);
void display_menu();
int get_user_choice();
bool get_country_input(char* country);
TreeNode* find_bucket_root(HashTable* table, const char* country);
void handle_display_parcels(HashTable* table);
void handle_filter_by_weight(HashTable* table);
void handle_total_load_and_value(HashTable* table);
void handle_cheapest_and_most_expensive(HashTable* table);
void handle_lightest_and_heaviest(HashTable* table);

// All handler functions
void printAllParcels(TreeNode* root);
void printParcelsByWeight(TreeNode* root, int weight, bool higher);
void calculateTotalLoadAndValue(TreeNode* root, int* totalLoad, float* totalValue);
void findCheapestAndMostExpensive(TreeNode* root, TreeNode** cheapest, TreeNode** mostExpensive);
void findLightestAndHeaviest(TreeNode* root, TreeNode** lightest, TreeNode** heaviest);

// Function definations

// main body function
int main() {

    // initiate HashTable
    HashTable* table = NULL;

    if (setjmp(error_buf) != 0) {
        // Error occurred, clean up and exit
        if (table) clean_up_hash_table(table);
        return 1;
    }

    // create hashtable using helper function
    table = create_hash_table();

    // read the file and load the data into hashtable, then validation occures
    if (read_file_and_populate_hash_table(table, "countries.txt") > 0) {

        // Display the bucket counts (just to confirm the data is loaded correctly)
        // display_bucket_counts(table);

        // Menu functionality
        int choice;
        do {
            // display the menu functionalities (what menu does what)
            display_menu();
            choice = get_user_choice();

            switch (choice) {

                // all functions with handle_ manages the input by user and calls the respective functions,
                // if any unusual things happen, it shows respective message for user's understanding.

            case 1:
                handle_display_parcels(table);
                break;
            case 2:
                handle_filter_by_weight(table);
                break;
            case 3:
                handle_total_load_and_value(table);
                break;
            case 4:
                handle_cheapest_and_most_expensive(table);
                break;
            case 5:
                handle_lightest_and_heaviest(table);
                break;
            case 6:
                printf("Exiting application.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
            }
        } while (choice != 6); // this will make choices to display infinitely, except user inputs 6.
    }
    else {
        fprintf(stderr, "Failed to read and populate data from file\n");
    }

    // cleans up hashtable, tree and all the occupied space on exit option entered by user.
    clean_up_hash_table(table);
    return 0;
}

// ALl handler function definations

/*
Function:       printAllParcels
Description:    This function prints all the parcel details present in tree, using tree traversal
                Traversal occures using recursion using In-Order traversal
*/
void printAllParcels(TreeNode* root) {
    if (root == NULL) return;
    printAllParcels(root->left);
    printf("Destination: %s, Weight: %d, Value: %.2f\n",
        root->parcel->destination, root->parcel->weight, root->parcel->value);
    printAllParcels(root->right);
}

/*
Function:       printParcelsByWeight
Description:    This function prints all the parcel details from the user entered destination combined with:
                higher weight then user entered weight (based on bool value)
                lower weight then user entered weight (based on bool value)
                Also as we are going from one node to another using recursion, we use In-Order traversal
*/
void printParcelsByWeight(TreeNode* root, int weight, bool higher) {
    if (root == NULL) {
        return;
    }

    printParcelsByWeight(root->left, weight, higher);

    if ((higher && root->parcel->weight > weight) || (!higher && root->parcel->weight < weight)) {
        printf("Destination: %s, Weight: %d, Value: %.2f\n",
            root->parcel->destination, root->parcel->weight, root->parcel->value);
    }

    printParcelsByWeight(root->right, weight, higher);
}

/*
Function:       calculateTotalLoadAndValue
Description:    This function is used to calculate cummulative sum of weight as well as value.
                Again it follows In-Order traversal.
*/
void calculateTotalLoadAndValue(TreeNode* root, int* totalLoad, float* totalValue) {
    if (root == NULL || totalLoad == NULL || totalValue == NULL) {
        return;
    }

    *totalLoad += root->parcel->weight;
    *totalValue += root->parcel->value;

    calculateTotalLoadAndValue(root->left, totalLoad, totalValue);
    calculateTotalLoadAndValue(root->right, totalLoad, totalValue);
}

/*
Function:       findCheapestAndMostExpensive
Description:    This function is used to filter smallest value and largest value from tree.
                This handler function is being used in other helper function and follows In-Order traversal.
*/
void findCheapestAndMostExpensive(TreeNode* root, TreeNode** cheapest, TreeNode** mostExpensive) {
    if (root == NULL || cheapest == NULL || mostExpensive == NULL) {
        return;
    }

    if (*cheapest == NULL || root->parcel->value < (*cheapest)->parcel->value) {
        *cheapest = root;
    }
    if (*mostExpensive == NULL || root->parcel->value > (*mostExpensive)->parcel->value) {
        *mostExpensive = root;
    }

    findCheapestAndMostExpensive(root->left, cheapest, mostExpensive);
    findCheapestAndMostExpensive(root->right, cheapest, mostExpensive);
}

/*
Function:       findLightestAndHeaviest
Description:    This function is used to filter smallest weight and largest weight from tree.
                This handler function is being used in other helper function and follows In-Order traversal.
*/
void findLightestAndHeaviest(TreeNode* root, TreeNode** lightest, TreeNode** heaviest) {
    if (root == NULL || lightest == NULL || heaviest == NULL) {
        return;
    }

    if (*lightest == NULL || root->parcel->weight < (*lightest)->parcel->weight) {
        *lightest = root;
    }
    if (*heaviest == NULL || root->parcel->weight > (*heaviest)->parcel->weight) {
        *heaviest = root;
    }

    findLightestAndHeaviest(root->left, lightest, heaviest);
    findLightestAndHeaviest(root->right, lightest, heaviest);
}


// Other function definations (helper functions)

/*
Function:       hash_djb2
Description:    This function computes a hash value for a given string using the djb2 algorithm.
                It is used to generate a unique hash value for the destination country names,
                which is then used to index the hash table. The computed hash value is
                ensured to fit within the bounds of the hash table size (HASH_SIZE).
*/
unsigned long hash_djb2(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

/*
Function:       count_tree_nodes
Description:    This function recursively counts the number of nodes in a binary tree.
                It traverses the tree, summing up the total number
                of nodes by adding one for the current node and recursively counting
                the nodes in the left and right subtrees.
*/
int count_tree_nodes(TreeNode* root) {
    if (root == NULL) return 0;
    return 1 + count_tree_nodes(root->left) + count_tree_nodes(root->right);
}

/*
Function:       display_bucket_counts
Description:    This function was generated to display the node values and parcel count in each hashtable bucket.
                As it is just to test the file read and data push into hashtable,
                it is not being used for any other purposes.
*/
void display_bucket_counts(HashTable* table) {
    printf("\nBucket counts:\n");
    int total_parcels = 0;
    int non_empty_buckets = 0;

    for (int i = 0; i < HASH_SIZE; i++) {
        TreeNode* bucket = table->buckets[i];
        if (bucket != NULL) {
            printf("%s(bucket-%d): %d parcels\n", bucket->parcel->destination, i, count_tree_nodes(bucket));
            total_parcels += count_tree_nodes(bucket);
            non_empty_buckets++;
        }
        else {
            printf("Empty bucket: %d\n", i);
        }
    }

    printf("\nTotal parcels: %d\n", total_parcels);
    printf("Non-empty buckets: %d\n", non_empty_buckets);
    printf("Empty buckets: %d\n", HASH_SIZE - non_empty_buckets);
}

/*
Function:       display_menu
Description:    Displays the main menu with options for viewing and querying parcel details.
*/
void display_menu() {
    printf("\n\nMAIN MENU\n");
    printf("=========\n\n");
    printf("1. Enter country name and display all the parcels details\n");
    printf("2. Enter country and weight pair\n");
    printf("3. Display the total parcel load and valuation for the country\n");
    printf("4. Enter the country name and display cheapest and most expensive parcel's details\n");
    printf("5. Enter the country name and display lightest and heaviest parcel for the country\n");
    printf("6. Exit the application\n");
}

/*
Function:       get_user_choice
Description:    Prompts the user for a menu selection, handles input validation, and returns the chosen option.
*/
int get_user_choice() {
    int choice;
    printf("\nMake a selection from the menu: ");
    if (scanf("%d", &choice) != 1) {
        while (getchar() != '\n');  // Clear input buffer
        return -1;  // Invalid input
    }
    while (getchar() != '\n');  // Clear input buffer
    return choice;
}

/*
Function:       create_hash_table
Description:    Allocates memory for a new hash table, initializes its buckets to NULL,
                and handles memory allocation failure.
*/
HashTable* create_hash_table() {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (table == NULL) {
        fprintf(stderr, "Memory allocation failed for hash table\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < HASH_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

/*
Function:       parse_line
Description:    Parses a line of text to extract parcel details and allocates memory
                for a new Parcel struct, handling errors for memory allocation and format.
*/
static ParseResult parse_line(const char* line, int line_number, Parcel** out_parcel) {
    Parcel* parcel = (Parcel*)malloc(sizeof(Parcel));
    if (!parcel) {
        return PARSE_MEMORY_ERROR;
    }

    char temp_country[MAX_COUNTRY_LENGTH];
    int weight;
    float value;

    if (sscanf(line, " %20[^,],%d,%f", temp_country, &weight, &value) != 3) {
        free(parcel);
        return PARSE_FORMAT_ERROR;
    }

    // Allocate memory for the destination string
    parcel->destination = (char*)malloc(strlen(temp_country) + 1);
    if (!parcel->destination) {
        free(parcel);
        return PARSE_MEMORY_ERROR;
    }

    // Copy the string
    strcpy(parcel->destination, temp_country);

    parcel->weight = weight;
    parcel->value = value;

    *out_parcel = parcel;
    return PARSE_SUCCESS;
}


/*
Function:       trim
Description:    Removes leading and trailing whitespace from a given string.
*/
void trim(char* str) {
    if (str == NULL) return;
    char* end;
    // Trim leading space
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;  // All spaces?
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    // Write new null terminator
    *(end + 1) = 0;
}

/*
Function:       create_tree_node
Description:    Allocates memory for a new TreeNode, initializes it with the given Parcel,
                and sets its left and right child pointers to NULL. If memory allocation fails,
                an error message is printed and the program exits.
*/
TreeNode* create_tree_node(Parcel* parcel) {
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed for tree node\n");
        exit(EXIT_FAILURE);
    }
    newNode->parcel = parcel;
    newNode->left = newNode->right = NULL;
    return newNode;
}

/*
Function:       insert_tree_node
Description:    Recursively inserts a Parcel into a binary search tree. If the tree is empty,
                it creates a new TreeNode with the given Parcel. If the tree is not empty,
                it compares the weight of the given Parcel with the weight of the Parcel in
                the current node and inserts it into the left or right subtree accordingly.
*/
void insert_tree_node(TreeNode** root, Parcel* parcel) {
    if (*root == NULL) {
        *root = create_tree_node(parcel);
    }
    else if (parcel->weight < (*root)->parcel->weight) {
        insert_tree_node(&((*root)->left), parcel);
    }
    else {
        insert_tree_node(&((*root)->right), parcel);
    }
}

/*
Function:       insert_parcel
Description:    Inserts a Parcel into a hash table. It first checks if the table or parcel is
                NULL. Then, it computes the hash index for the Parcel's destination and
                inserts the Parcel into the corresponding bucket's binary search tree.
*/
void insert_parcel(HashTable* table, Parcel* parcel) {
    if (table == NULL || parcel == NULL) return;
    unsigned long index = hash_djb2(parcel->destination);
    insert_tree_node(&(table->buckets[index]), parcel);
}

/*
Function:       read_file_and_populate_hash_table
Description:    Reads a file containing parcel data and populates a hash table with the parcels.
                It opens the file, skips the header line, and processes each subsequent line to
                create and insert Parcel objects into the hash table. Handles errors such as file
                opening issues, memory allocation failures, and format errors, and prints the
                number of lines processed and parcels successfully inserted.
*/
int read_file_and_populate_hash_table(HashTable* table, const char* filename) {
    FILE* file = NULL;
    char line[MAX_LINE_LENGTH];
    int line_count = 0;
    int successful_inserts = 0;

    if (setjmp(error_buf) != 0) {
        // Error occurred, clean up and return
        if (file) fclose(file);
        return successful_inserts;
    }

    file = fopen(filename, "r");
    if (!file) {
        HANDLE_ERROR("Error opening file");
    }

    // Skip the header line
    if (fgets(line, sizeof(line), file) == NULL) {
        HANDLE_ERROR("Error reading header line or file is empty");
    }

    while (fgets(line, sizeof(line), file)) {
        line_count++;
        trim(line);

        Parcel* parcel = NULL;
        ParseResult result = parse_line(line, line_count, &parcel);

        switch (result) {
        case PARSE_SUCCESS:
            insert_parcel(table, parcel);
            successful_inserts++;
            break;
        case PARSE_MEMORY_ERROR:
            fprintf(stderr, "Memory allocation failed on line %d\n", line_count);
            break;
        case PARSE_FORMAT_ERROR:
            fprintf(stderr, "Invalid format on line %d\n", line_count);
            break;
        }
    }

    if (ferror(file)) {
        HANDLE_ERROR("Error reading file");
    }

    fclose(file);

    printf("Processed %d lines, successfully inserted %d parcels\n", line_count, successful_inserts);
    return successful_inserts;
}

/*
Function:       get_country_input
Description:    Prompts the user for a country name, reads it from input, and removes the newline character.
                Returns true if the input was read successfully, otherwise returns false.
*/
bool get_country_input(char* country) {
    printf("Enter country name: ");
    if (fgets(country, sizeof(country), stdin) == NULL) {
        printf("Error reading input.\n");
        return false;
    }
    country[strcspn(country, "\n")] = '\0';
    return true;
}

/*
Function:       findBucketRoot
Description:    Retrieves the root node of the binary search tree in the hash table bucket corresponding
                to the hashed index of the given country. If the root node's destination matches the
                country name (case-insensitive), it returns the root node; otherwise, it returns NULL.
*/
TreeNode* find_bucket_root(HashTable* table, const char* country) {
    unsigned long index = hash_djb2(country);
    TreeNode* root = table->buckets[index];
    if (root != NULL) {
        if (_stricmp(root->parcel->destination, country) == 0) {
            return root;
        }
    }
    return NULL;
}

/*
Function:       handle_display_parcels
Description:    Prompts the user for a country name and displays all parcels associated with that
                country from the hash table. It reads the country name from the input, removes
                the newline character, and retrieves the corresponding binary search tree from the
                hash table. If parcels are found, it calls a function to print them; otherwise,
                it notifies the user that no parcels were found for the specified country.
*/
void handle_display_parcels(HashTable* table) {
    char country[MAX_COUNTRY_LENGTH];

    if (!get_country_input(country)) {
        return;
    }

    TreeNode* root = find_bucket_root(table, country);
    if (root != NULL) {
        // handler function call
        printAllParcels(root);
    }
    else {
        printf("No parcels found for %s.\n", country);
    }
}

/*
Function:       handle_filter_by_weight
Description:    Prompts the user for a country name, weight, and a filter choice to display parcels
                based on their weight. It reads the country name, weight, and filter choice from
                the input, then retrieves the corresponding binary search tree from the hash table.
                Based on the filter choice (higher or lower weight), it calls a function to print
                the relevant parcels. If no parcels are found for the specified country or if input
                is invalid, it notifies the user.
*/
void handle_filter_by_weight(HashTable* table) {
    char country[MAX_COUNTRY_LENGTH];
    int weight, higher;

    if (!get_country_input(country)) {
        return;
    }

    printf("Enter weight: ");
    if (scanf("%d", &weight) != 1) {
        printf("Invalid weight input.\n");
        while (getchar() != '\n');  // Clear input buffer
        return;
    }

    printf("Do you want parcels with weight higher (1) or lower (0)? ");
    if (scanf("%d", &higher) != 1 || (higher != 0 && higher != 1)) {
        printf("Invalid choice. Please enter 0 or 1.\n");
        while (getchar() != '\n');  // Clear input buffer
        return;
    }
    while (getchar() != '\n');  // Clear input buffer

    TreeNode* root = find_bucket_root(table, country);
    if (root != NULL) {
        // handler function
        printParcelsByWeight(root, weight, higher);
    }
    else {
        printf("No parcels found for %s.\n", country);
    }
}

/*
Function:       handle_total_load_and_value
Description:    Prompts the user for a country name and calculates the total load and value of
                parcels associated with that country from the hash table. It reads the country
                name from the input, retrieves the corresponding binary search tree from the
                hash table, and calls a function to compute the total load and value of the
                parcels. The results are then printed. If no parcels are found for the specified
                country, it notifies the user.
*/
void handle_total_load_and_value(HashTable* table) {
    char country[MAX_COUNTRY_LENGTH];
    if (!get_country_input(country)) {
        return;
    }
    TreeNode* root = find_bucket_root(table, country);
    if (root != NULL) {
        int total_load = 0;
        float total_value = 0.0;
        // handler function call
        calculateTotalLoadAndValue(root, &total_load, &total_value);
        printf("Total load: %d grams, Total valuation: $%.2f\n", total_load, total_value);
    }
    else {
        printf("No parcels found for %s.\n", country);
    }
}

/*
Function:       handle_cheapest_and_most_expensive
Description:    Prompts the user for a country name and identifies the cheapest and most expensive
                parcels associated with that country from the hash table. It reads the country name
                from the input, retrieves the corresponding binary search tree from the hash table,
                and calls a function to find the cheapest and most expensive parcels. The details of
                these parcels are then printed. If no parcels are found or if the function cannot
                determine the cheapest or most expensive parcels, it notifies the user.
*/
void handle_cheapest_and_most_expensive(HashTable* table) {
    char country[MAX_COUNTRY_LENGTH];
    if (!get_country_input(country)) {
        return;
    }
    TreeNode* root = find_bucket_root(table, country);
    if (root != NULL) {
        TreeNode* cheapest = NULL, * most_expensive = NULL;
        // handler function call
        findCheapestAndMostExpensive(root, &cheapest, &most_expensive);
        if (cheapest != NULL && most_expensive != NULL) {
            printf("Cheapest parcel - Destination: %s, Weight: %d, Value: $%.2f\n",
                cheapest->parcel->destination, cheapest->parcel->weight, cheapest->parcel->value);
            printf("Most expensive parcel - Destination: %s, Weight: %d, Value: $%.2f\n",
                most_expensive->parcel->destination, most_expensive->parcel->weight, most_expensive->parcel->value);
        }
        else {
            printf("Unable to find cheapest and most expensive parcels.\n");
        }
    }
    else {
        printf("No parcels found for %s.\n", country);
    }
}

/*
Function:       handle_lightest_and_heaviest
Description:    Prompts the user for a country name and identifies the lightest and heaviest
                parcels associated with that country from the hash table. It reads the country name
                from the input, retrieves the corresponding binary search tree from the hash table,
                and calls a function to find the lightest and heaviest parcels. The details of these
                parcels are then printed. If no parcels are found or if the function cannot determine
                the lightest or heaviest parcels, it notifies the user.
*/
void handle_lightest_and_heaviest(HashTable* table) {
    char country[MAX_COUNTRY_LENGTH];
    if (!get_country_input(country)) {
        return;
    }
    TreeNode* root = find_bucket_root(table, country);
    if (root != NULL) {
        TreeNode* lightest = NULL, * heaviest = NULL;
        // handler function call
        findLightestAndHeaviest(root, &lightest, &heaviest);
        if (lightest != NULL && heaviest != NULL) {
            printf("Lightest parcel - Destination: %s, Weight: %d, Value: $%.2f\n",
                lightest->parcel->destination, lightest->parcel->weight, lightest->parcel->value);
            printf("Heaviest parcel - Destination: %s, Weight: %d, Value: $%.2f\n",
                heaviest->parcel->destination, heaviest->parcel->weight, heaviest->parcel->value);
        }
        else {
            printf("Unable to find lightest and heaviest parcels.\n");
        }
    }
    else {
        printf("No parcels found for %s.\n", country);
    }
}

/*
Function:       clean_up_tree
Description:    Recursively frees the memory used by all nodes in a binary search tree and their
                associated parcels. It first deallocates memory for the parcel and then the tree node itself.
*/
void clean_up_tree(TreeNode* root) {
    if (root == NULL) return;
    clean_up_tree(root->left);
    clean_up_tree(root->right);
    free(root->parcel->destination);
    free(root->parcel);
    free(root);
}

/*
Function:       clean_up_hash_table
Description:    Frees the memory used by all buckets in a hash table and then deallocates the hash table itself.
*/
void clean_up_hash_table(HashTable* table) {
    for (int i = 0; i < HASH_SIZE; i++) {
        clean_up_tree(table->buckets[i]);
    }
    free(table);
}