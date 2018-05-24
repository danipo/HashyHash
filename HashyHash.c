//Daniel Powley, da872859
//Assignment 4, HashyHash

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "HashyHash.h"

HashTable *makeHashTable(int capacity)
{
    int i;

    HashTable *h = malloc(sizeof(HashTable));

    //creates an array of 5, if 0 or less is passed
    if(capacity < 1)
    {
        h->capacity = DEFAULT_CAPACITY;
        h->array = malloc(sizeof(int)* DEFAULT_CAPACITY);
    }

    else
    {
        h->capacity = capacity;
        h->array = malloc(sizeof(int)* capacity);
    }

    for(i = 0; i<capacity; i++)
    {
        h->array[i] = UNUSED;
    }

    h->size = 0;

    h->hashFunction = NULL;

    h->probing = LINEAR;

    h->stats.collisions = 0;
    h->stats.opCount = 0;

    //printf("Table created, capacity: %d\n", h->capacity);

    return h;
}

//frees dynamically allocated memory and then destroys the table itself
HashTable *destroyHashTable(HashTable *h)
{
    if(h == NULL)
        return NULL;

    free(h->array);
    free(h);

    return NULL;
}

int setProbingMechanism(HashTable *h, ProbingType probing)
{
    if(h == NULL)
        return HASH_ERR;

    //printf("Probing Mechanism set\n");
    h->probing = probing;
    return HASH_OK;
}

//sets hash function or nothing if there is none
int setHashFunction(HashTable *h, unsigned int (*hashFunction)(int))
{
    if(h == NULL)
        return HASH_ERR;

    h->hashFunction = hashFunction;

    //printf("Hash function set\n");
    return HASH_OK;
}

//checks if array is half empty, to assist in expansion, if needed
int isAtLeastHalfEmpty(HashTable *h)
{
    if(h == NULL || h->capacity == 0)
        return 0;

    if(h->size > (h->capacity/2))
        return 0;

    return 1;
}

//borrowed from primes.c
int nextPrime(int n)
{
	int i, root, keepGoing = 1;

	if (n % 2 == 0)
		++n;

	while (keepGoing)
	{
		keepGoing = 0;
		root = sqrt(n);

		for (i = 3; i <= root; i++)
		{
			if (n % i == 0)
			{
				// Move on to the next candidate for primality. Since n is odd, we
				// don't want to increment it by 1. That would give us an even
				// integer greater than 2, which would necessarily be non-prime.
				n += 2;
				keepGoing = 1;

				// Leave for-loop. Move on to next iteration of while-loop.
				break;
			}
		}
	}

	return n;
}

//helper for expansion. Takes the temporary array and re-inserts into the new one.
void rehash(HashTable *h, int *array, int cap)
{
    int i;

    for(i=0;i<cap-1;i++)
    {
        insert(h,array[i]);
    }
}

int expandHashTable(HashTable *h)
{
    int i,j, temp;
    int newCapacity = (h->capacity * 2) + 1;

    //create a temporary array that will hold each value in the current h->array
    int *tempArray = malloc(sizeof(int) * h->size);

    //scans the current array for a value and passes it to the temporary array, then clears it.
    for(i=0; i<(h->size-1); i++)
    {
        temp = 0;
        for(j = 0; j <(h->capacity-1); j++)
        {
            if(h->array[j] != UNUSED || h->array[j] != DIRTY)
                temp = h->array[j];
                h->array[j] = UNUSED;
        }
        tempArray[i] = temp;
    }

    //creates a new array of the new capacity and rehashes the values
    if(h->probing == LINEAR)
    {
        h->array = realloc(h->array, sizeof(int)*newCapacity);
        rehash(h,tempArray, newCapacity);
    }

    if(h->probing == QUADRATIC)
    {
        h->array = realloc(h->array, sizeof(int) * (nextPrime(newCapacity)));
        rehash(h,tempArray, nextPrime(newCapacity));
    }

    free(tempArray);
    return HASH_OK;
}

int insert(HashTable *h, int key)
{
    int index, newSpot;
    int i = 0;

    if(h == NULL)
        return HASH_ERR;

    if(!isAtLeastHalfEmpty(h))
    {
        //printf("Time to expand!\n");
        expandHashTable(h);
    }

    //finds the index for the key and a temp value, where it is going to go if a collision occurs
    index = newSpot = h->hashFunction(key) % h->capacity;

    //printf("index is %d\n", index);

    //if the index is empty or dirty, place the key there
    if(h->array[index] == UNUSED || h->array[index] == DIRTY)
    {
        h->array[index] = key;
        ++h->size;
        ++h->stats.opCount;
        //printf("We've done %d ops\n", h->stats.opCount);
        return HASH_OK;
    }

    else
    {
        if(h->probing == LINEAR)
        {
            if (h->array[index] != UNUSED || h->array[index] != DIRTY)
            {
                ++h->stats.collisions;
                index += i % h->capacity;
                i++;
            }
        }

        if(h->probing == QUADRATIC)
        {
            //while(h->array[newSpot] != UNUSED || h->array[newSpot] != DIRTY);{
                ++h->stats.collisions;
                newSpot = (index + (i*i)) % h->capacity;
                //printf("Collision! New index is %d\n", newSpot);
                i++;
            //}
            h->array[newSpot] = key;
        }
    }

    ++h->size;
    ++h->stats.opCount;
    //printf("We've done %d ops\n", h->stats.opCount);

    return HASH_OK;
}

//searches the table for the key
int search(HashTable *h, int key)
{
    if (h == NULL || h->hashFunction == NULL)
        return -1;

    int index = 0, i;

    index = h->hashFunction(key) % h->capacity;

    for(i=0; i < h->capacity-1; i++)
    {
        if(h->probing == LINEAR)
        {
            if(h->array[index] == key)
                ++h->stats.opCount;
                return index;
            if(h->array[index] == UNUSED)
                ++h->stats.opCount;
                return -1;

            ++h->stats.collisions;
            index += (i+1) % h->capacity;
        }

        if(h->probing == QUADRATIC)
        {
            if(h->array[index] == key)
                ++h->stats.opCount;
                return index;
            if(h->array[index] == UNUSED)
                ++h->stats.opCount;
                return -1;

            ++h->stats.collisions;
            index += (i*i) % h->capacity;
        }
    }

    ++h->stats.opCount;
    return -1;
}

//uses the same search function, but also adds a deletion operation
int delete(HashTable *h, int key)
{
    if(h == NULL || h->hashFunction == NULL)
        return -1;

    int index;

    index = search(h, key);

    if(h->array[index] == key)
    {
        h->array[index] = DIRTY;
        ++h->stats.opCount;
        return index;
    }

    ++h->stats.opCount;
    return -1;
}

double difficultyRating(void)
{
    return 4.0;
}

double hoursSpent(void)
{
    return 20.0;
}
