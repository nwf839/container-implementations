#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
   assert(elemSize > 0);
   assert(numBuckets > 0);
   assert(hashfn != NULL);
   assert(comparefn != NULL);
   h->elemSize = elemSize;
   h->loglength = 0;
   h->numBuckets = numBuckets;
   h->buckets = malloc(sizeof(vector) * numBuckets);
   h->HashSetHashFunction = hashfn;
   h->HashSetCompareFunction = comparefn;
   h->HashSetFreeFunction = freefn;
   for (int i = 0; i < numBuckets; i++) VectorNew(h->buckets + i, h->elemSize, h->HashSetFreeFunction, 0);
}

void HashSetDispose(hashset *h)
{ 
    if (h->HashSetFreeFunction != NULL) {
        for (int i = 0; i < h->numBuckets; i++) {
            h->HashSetFreeFunction(h->buckets + i);
        }
    }
    free(h->buckets);
}



int HashSetCount(const hashset *h)
{ 
    return h->loglength;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
    assert(mapfn != NULL);
    for (int i = 0; i < h->numBuckets; i++) {
        VectorMap(h->buckets + i, mapfn, auxData);
    }
}

// Returns a vector pointer to the address of the bucket that an element pointed to
// by elemAddr hashes to
static vector* HashToBucket(const hashset *h, const void *elemAddr)
{
    assert(elemAddr != NULL);
    int position = h->HashSetHashFunction(elemAddr, h->numBuckets);
    assert(position >= 0);
    assert(position < h->numBuckets);
    return h->buckets + position;
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
    void *search = HashSetLookup(h, elemAddr);
    if (search != NULL) {
        if (h->HashSetFreeFunction != NULL) h->HashSetFreeFunction(search);
        memcpy(search, elemAddr, h->elemSize);
    }
    else {
        vector* bucket = HashToBucket(h, elemAddr);
        VectorAppend(bucket, elemAddr);
        h->loglength++;
    }
    /*vector* bucket = (vector*) HashToBucket(h, elemAddr);
    int position = VectorSearch(bucket, elemAddr, h->HashSetCompareFunction, 0, false);
    if (position == -1) {
        VectorAppend(bucket, elemAddr);
        h->loglength++;
    }
    else VectorReplace(bucket, elemAddr, position);*/
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
    vector* bucket = HashToBucket(h, elemAddr);
    int position = VectorSearch(bucket, elemAddr, h->HashSetCompareFunction, 0, false);
    return (position == -1) ? NULL : VectorNth(bucket, position);
}
