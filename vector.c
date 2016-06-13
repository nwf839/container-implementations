#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <assert.h>

#include "vector.h"

#define NthElemAddr(base, elemSize, index) \
    ((char*) base + index * elemSize)

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(elemSize > 0);
    v->elemSize = elemSize;
    v->loglength = 0;
    assert(initialAllocation >= 0);
    if (initialAllocation == 0) initialAllocation = 10;
    v->alloclength = initialAllocation;
    v->elems = malloc(initialAllocation * v->elemSize);
    v->VectorFreeFunction = freeFn;
}

void VectorDispose(vector *v)
{
    if (v->VectorFreeFunction != NULL) {
        for (int i = 0; i < v->loglength; i++) {
            v->VectorFreeFunction((char*) v->elems + i * v->elemSize);
        }
    }
    free(v->elems);
}

static void VectorGrow(vector *v)
{
    v->alloclength *= 2;
    v->elems = realloc(v->elems, v->alloclength * v->elemSize);
}

int VectorLength(const vector *v)
{
    return v->loglength;
}

void *VectorNth(const vector *v, int position)
{
    assert(position >= 0);
    assert(position < v->loglength);
    return NthElemAddr(v->elems, v->elemSize, position);
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0);
    assert(position < v->loglength);
    void* target = VectorNth(v, position);
    if (v->VectorFreeFunction != NULL) v->VectorFreeFunction(target);
    memcpy(target, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0);
    assert(position <= v->loglength);
    if (v->loglength == v->alloclength) VectorGrow(v);
    void* insertionPoint = NthElemAddr(v->elems, v->elemSize, position);
    void* target = (char*) insertionPoint + v->elemSize;
    if (position != v->loglength)
        memmove(target, insertionPoint, v->elemSize * (v->loglength - position));
    memcpy(insertionPoint, elemAddr, v->elemSize);
    v->loglength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    if (v->loglength == v->alloclength) VectorGrow(v);
    VectorInsert(v, v->elems, v->loglength);
    v->loglength++;
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0);
    assert(position < v->loglength);
    void* target = VectorNth(v, position);
    if (v->VectorFreeFunction != NULL) v->VectorFreeFunction(target);
    void* source = (char*) target + v->elemSize;
    v->loglength--;
    memcpy(target, source, v->elemSize * (v->loglength - position));
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->loglength, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);
    for (int i = 0; i < v->loglength; i++) {
        mapFn((char*) v->elems + v->elemSize * i, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    assert(startIndex >= 0);
    assert(startIndex <= v->loglength);
    int position = kNotFound;
    void* cursor = NULL;
    void* base = (char*) v->elems + startIndex * v->elemSize;
    size_t n = (size_t) v->loglength;
    if (isSorted) cursor = bsearch(key, base, n, v->elemSize, searchFn);
    else cursor = lfind(key, base, &n, v->elemSize, searchFn);
    if (cursor != NULL) position = ((char*) cursor - (char*) v->elems) * v->elemSize;
    return position;
}
