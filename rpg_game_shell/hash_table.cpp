/*
 Student Name:
 Date:

=======================
ECE 2035 Project 2-1:
=======================
This file provides definition for the structs and functions declared in the
header file. It also contains helper functions that are not accessible from
outside of the file.

FOR FULL CREDIT, BE SURE TO TRY MULTIPLE TEST CASES and DOCUMENT YOUR CODE.

===================================
Naming conventions in this file:
===================================
1. All struct names use camel case where the first letter is capitalized.
  e.g. "HashTable", or "HashTableEntry"

2. Variable names with a preceding underscore "_" will not be called directly.
  e.g. "_HashTable", "_HashTableEntry"

  Recall that in C, we have to type "struct" together with the name of the struct
  in order to initialize a new variable. To avoid this, in hash_table.h
  we use typedef to provide new "nicknames" for "struct _HashTable" and
  "struct _HashTableEntry". As a result, we can create new struct variables
  by just using:
    - "HashTable myNewTable;"
     or
    - "HashTableEntry myNewHashTableEntry;"

  The preceding underscore "_" simply provides a distinction between the names
  of the actual struct defition and the "nicknames" that we use to initialize
  new structs.
  [See Hidden Definitions section for more information.]

3. Functions, their local variables and arguments are named with camel case, where
  the first letter is lower-case.
  e.g. "createHashTable" is a function. One of its arguments is "numBuckets".
       It also has a local variable called "newTable".

4. The name of a struct member is divided by using underscores "_". This serves
  as a distinction between function local variables and struct members.
  e.g. "num_buckets" is a member of "HashTable".

*/

/****************************************************************************
* Include the Public Interface
*
* By including the public interface at the top of the file, the compiler can
* enforce that the function declarations in the the header are not in
* conflict with the definitions in the file. This is not a guarantee of
* correctness, but it is better than nothing!
***************************************************************************/
#include "hash_table.h"
#include "globals.h"


/****************************************************************************
* Include other private dependencies
*
* These other modules are used in the implementation of the hash table module,
* but are not required by users of the hash table.
***************************************************************************/
#include <stdlib.h>   // For malloc and free
#include <stdio.h>    // For printf


/****************************************************************************
* Hidden Definitions
*
* These definitions are not available outside of this file. However, because
* the are forward declared in hash_table.h, the type names are
* available everywhere and user code can hold pointers to these structs.
***************************************************************************/
/**
 * This structure represents an a hash table.
 * Use "HashTable" instead when you are creating a new variable. [See top comments]
 */
struct _HashTable {
  /** The array of pointers to the head of a singly linked list, whose nodes
      are HashTableEntry objects */
  HashTableEntry** buckets;

  /** The hash function pointer */
  HashFunction hash;

  /** The number of buckets in the hash table */
  unsigned int num_buckets;
};

/**
 * This structure represents a hash table entry.
 * Use "HashTableEntry" instead when you are creating a new variable. [See top comments]
 */
struct _HashTableEntry {
  /** The key for the hash table entry */
  unsigned int key;

  /** The value associated with this hash table entry */
  void* value;

  /**
  * A pointer pointing to the next hash table entry
  * NULL means there is no next entry (i.e. this is the tail)
  */
  HashTableEntry* next;
};



/****************************************************************************
* Private Functions
*
* These functions are not available outside of this file, since they are not
* declared in hash_table.h.
***************************************************************************/
/**
* createHashTableEntry
*
* Helper function that creates a hash table entry by allocating memory for it on
* the heap. It initializes the entry with key and value, initialize pointer to
* the next entry as NULL, and return the pointer to this hash table entry.
*
* @param key The key corresponds to the hash table entry
* @param value The value stored in the hash table entry
* @return The pointer to the hash table entry
*/
static HashTableEntry* createHashTableEntry(unsigned int key, void* value) {
    HashTableEntry* newEntry = (HashTableEntry*)malloc(sizeof(HashTableEntry));//Make room for new entry
    newEntry->key = key;
    newEntry->value = value;
    newEntry->next = NULL;
    return newEntry;
}

/**
* findItem
*
* Helper function that checks whether there exists the hash table entry that
* contains a specific key.
*
* @param hashTable The pointer to the hash table.
* @param key The key corresponds to the hash table entry
* @return The pointer to the hash table entry, or NULL if key does not exist
*/
static HashTableEntry* findItem(HashTable* hashTable, unsigned int key) {
    unsigned int index = hashTable->hash(key);
    HashTableEntry *i = hashTable->buckets[index];
    if(!i) { //This means the bucket is empty and won't contain the item
        return NULL;
    } 
    while(i) { //While I is not null
        if (i->key == key) {
            return i; //Return the match if one is found
        }
        i = i->next;
    }
    return NULL; //If we got here, there was no match
}

/****************************************************************************
* Public Interface Functions
*
* These functions implement the public interface as specified in the header
* file, and make use of the private functions and hidden definitions in the
* above sections.
****************************************************************************/
// The createHashTable is provided for you as a starting point.
HashTable* createHashTable(HashFunction hashFunction, unsigned int numBuckets) {
  // The hash table has to contain at least one bucket. Exit gracefully if
  // this condition is not met.
  if (numBuckets==0) {
    printf("Hash table has to contain at least 1 bucket...\n");
    exit(1);
  }

  // Allocate memory for the new HashTable struct on heap.
  HashTable* newTable = (HashTable*)malloc(sizeof(HashTable));

  // Initialize the components of the new HashTable struct.
  newTable->hash = hashFunction;
  newTable->num_buckets = numBuckets;
  newTable->buckets = (HashTableEntry**)malloc(numBuckets*sizeof(HashTableEntry*));

  // As the new buckets contain indeterminant values, init each bucket as NULL.
  unsigned int i;
  for (i=0; i<numBuckets; ++i) {
    newTable->buckets[i] = NULL;
  }

  // Return the new HashTable struct.
  return newTable;
}


void destroyHashTable(HashTable* hashTable) {
        for(unsigned int i = 0; i < hashTable->num_buckets; i++){ //Free buckets
            HashTableEntry* j = (hashTable->buckets[i]); //Get the first buckets
            if(hashTable->buckets[i]){ //If there are entries in the bucket
                HashTableEntry* tNext = j; //Temp holder
                while(j->next) { //While there is a next node
                    tNext = j->next; //Get the next data
                    free(j); //Free the current node
                    j = tNext; //move to next node
                }
                free(j); //Free the last one we stopped on              
            }
        }
        free(hashTable->buckets);
        free(hashTable); //Free the entire table
    }


void* insertItem(HashTable* hashTable, unsigned int key, void* value) {
    unsigned int index = hashTable->hash(key); //Find where the key should go
    HashTableEntry* j = hashTable->buckets[index]; //Get the pointer from the bucket
    HashTableEntry* i = j;
    if (!j) { //If the bucket is null, this is the first item in the bucket
        j = createHashTableEntry(key, value); //Create an entry
        hashTable->buckets[index] = j; //Store the pointer to the entry at the head
        return NULL;
    }
    while(j) { //While j points to another node
        i = j; //i holds current node
        if (i->key == key) { //This means that we need to replace
            void* temp = i->value; //Hold value from i
            i->value = value; //change out the values
            return temp; //Return the value
        }
        j = j->next; //Otherwise move onto the next one
    }
    i->next = createHashTableEntry(key, value); //j is null, so i is the tail and we should adjust the pointers
    return NULL;


}

void* getItem(HashTable* hashTable, unsigned int key) {
    HashTableEntry* j = findItem(hashTable, key);
    if (j == NULL){
        return NULL;
    }
    return j->value;

}

void* removeItem(HashTable* hashTable, unsigned int key) {
    HashTableEntry* j = NULL;;
    void* returnVal = NULL;
    unsigned int index = hashTable->hash(key); //Get the bucket
    HashTableEntry *i = hashTable->buckets[index]; //Get the first pointer
    while(i) {
        if (i->key == key) { //If we find a match
            if(j == NULL) { //This means the key was in the first location
                returnVal = i->value; //Get the value
                hashTable->buckets[index] = i->next; //Set the bucket to the next one
                free(i->value);
                free(i); 
                printf("Found something to remove \r\n");
                return returnVal;
            }
            if(i->next == NULL) { //If not in the first position, and next is null, this is the tail
                j->next = NULL; //Set the previous pointer to null
                returnVal = i->value; //Get the value
                free(i->value);
                free(i);
                printf("Found something to remove \r\n");
                return returnVal;
            }
            if(i->next != NULL) { //This means this is in the middle of the list
                j->next = i->next; //Previous pointer should point where i was pointing
                free(i->value);
                free(i);
                printf("Found something to remove \r\n");
                return returnVal;
            }
        }
        j = i; //Hold i
        i = i->next; //Increment i
    }
    printf("Didn't find something to remove \r\n");
    return NULL; //This means we didn't find it

}

void deleteItem(HashTable* hashTable, unsigned int key) {
    HashTableEntry* j = NULL;
    unsigned int index = hashTable->hash(key); //Get the bucket
    HashTableEntry *i = hashTable->buckets[index];
    j = NULL;
    while(i) {
        if (i->key == key) {
                        if(j == NULL) { //This means the key was in the first location
                            hashTable->buckets[index] = i->next; //Change the bucket pointer
                            free(i->value);
                            free(i);
                            i = NULL;
                        } else if(i->next == NULL) { //Tail location
                            j->next = NULL; //Set previous to null
                            free(i->value);
                            free(i);
                            i = NULL;
                        } else if(i->next != NULL) { //In the middle
                            j->next = i->next;
                            free(i->value);
                            free(i);
                            i = NULL;
                        }
                    }
                if(i) { 
                j = i;
                i = i->next;
            }
            }
}
