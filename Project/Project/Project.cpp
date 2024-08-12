/*
FILE       : project
PROJECT : project
PROGRAMMER : ANCHITA KAKRIA(8961585) , ARYANKUMAT MALAVIYA(8963030) 
FIRSTVERSION : 11 AUG 2024
DESCRIPTION :This C program manages courier parcel data using Hash Tables and Binary Search Trees.
It will read parcel details from the txt file and enables users to get info based on country, weight and value.
*/


#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
HashTable* create_hash_table();
void display_menu();
int get_user_choice();
int read_file_and_populate_hash_table(HashTable* table, const char* filename);
TreeNode* create_tree_node(Parcel* parcel);
void insert_tree_node(TreeNode** root, Parcel* parcel);
void insert_parcel(HashTable* table, Parcel* parcel);
static ParseResult parse_line(const char* line, int line_number, Parcel** out_parcel);
void clean_up_hash_table(HashTable* table);
void handle_display_parcels(HashTable* table)

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

                // all functions with `handle_` manages the input by user and calls the respective functions,
                // if any unusual things happen, it shows respective message for user's understanding.
            case 1:
                printf("1");
                break;
            case 2:
                printf("2");
                break;
            case 3:
                printf("3");
                break;
            case 4:
                printf("4");
                break;
            case 5:
                printf("5");
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

    return 0;
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
