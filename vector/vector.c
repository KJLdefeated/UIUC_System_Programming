/**
 * vector
 * CS 341 - Fall 2024
 */
#include "vector.h"
#include <assert.h>

struct vector {
    /* The function callback for the user to define the way they want to copy
     * elements */
    copy_constructor_type copy_constructor;

    /* The function callback for the user to define the way they want to destroy
     * elements */
    destructor_type destructor;

    /* The function callback for the user to define the way they a default
     * element to be constructed */
    default_constructor_type default_constructor;

    /* Void pointer to the beginning of an array of void pointers to arbitrary
     * data. */
    void **array;

    /**
     * The number of elements in the vector.
     * This is the number of actual objects held in the vector,
     * which is not necessarily equal to its capacity.
     */
    size_t size;

    /**
     * The size of the storage space currently allocated for the vector,
     * expressed in terms of elements.
     */
    size_t capacity;
};

/**
 * IMPLEMENTATION DETAILS
 *
 * The following is documented only in the .c file of vector,
 * since it is implementation specfic and does not concern the user:
 *
 * This vector is defined by the struct above.
 * The struct is complete as is and does not need any modifications.
 *
 * The only conditions of automatic reallocation is that
 * they should happen logarithmically compared to the growth of the size of the
 * vector inorder to achieve amortized constant time complexity for appending to
 * the vector.
 *
 * For our implementation automatic reallocation happens when -and only when-
 * adding to the vector makes its new  size surpass its current vector capacity
 * OR when the user calls on vector_reserve().
 * When this happens the new capacity will be whatever power of the
 * 'GROWTH_FACTOR' greater than or equal to the target capacity.
 * In the case when the new size exceeds the current capacity the target
 * capacity is the new size.
 * In the case when the user calls vector_reserve(n) the target capacity is 'n'
 * itself.
 * We have provided get_new_capacity() to help make this less ambigious.
 */

static size_t get_new_capacity(size_t target) {
    /**
     * This function works according to 'automatic reallocation'.
     * Start at 1 and keep multiplying by the GROWTH_FACTOR untl
     * you have exceeded or met your target capacity.
     */
    size_t new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}

vector *vector_create(copy_constructor_type copy_constructor,
                      destructor_type destructor,
                      default_constructor_type default_constructor) {
    // your code here
    // Casting to void to remove complier error. Remove this line when you are
    // ready.
    vector* v = (vector*)malloc(sizeof(vector));
    if (v == NULL) {
        return NULL;
    }
    v->copy_constructor = copy_constructor;
    v->destructor = destructor;
    v->default_constructor = default_constructor;
    v->size = 0;
    v->capacity = INITIAL_CAPACITY;
    v->array = (void**)malloc(sizeof(void*) * INITIAL_CAPACITY);
    if (v->array == NULL){
        free(v);
        return NULL;
    }
    for (size_t i = 0; i < INITIAL_CAPACITY; i++) {
        v->array[i] = default_constructor();
    }
    return v;
}

void vector_destroy(vector *this) {
    assert(this);
    // your code here
    for (size_t i = 0; i < this->size; i++) {
        this->destructor(this->array[i]);
        this->array[i] = NULL;
    }
    free(this->array);
    free(this);
}

void **vector_begin(vector *this) {
    if (this == NULL || this->array == NULL) {
        return NULL;
    }
    return this->array + 0;
}

void **vector_end(vector *this) {
    if (this == NULL || this->array == NULL) {
        return NULL;
    }
    return this->array + this->size;
}

size_t vector_size(vector *this) {
    assert(this);
    // your code here
    return this->size;
}

void vector_resize(vector *this, size_t n) {
    assert(this);
    // your code here
    if (n < this->size) {
        for (size_t i = n; i < this->size; i++) {
            this->destructor(this->array[i]);
            this->array[i] = this->default_constructor();
        }
        this->size = n;
    }
    else if (n > this->size) {
        if (n > this->capacity) {
            vector_reserve(this, n);
        }
        for (size_t i = this->size; i < n; i++) {
            this->array[i] = this->default_constructor();
        }
        this->size = n;
    }
}

size_t vector_capacity(vector *this) {
    assert(this);
    // your code here
    return this->capacity;
}

bool vector_empty(vector *this) {
    assert(this);
    // your code here
    return this->size == 0;
}

void vector_reserve(vector *this, size_t n) {
    assert(this);
    // your code here
    if (n <= this->capacity) return;
    size_t new_capacity = get_new_capacity(n);
    void** new_array = (void**)malloc(sizeof(void*) * new_capacity);
    if (new_array == NULL) return;
    for (size_t i = 0; i < this->size; i++) new_array[i] = this->array[i];
    for (size_t i = this->size; i < new_capacity; i++) new_array[i] = this->default_constructor();
    free(this->array);
    this->array = new_array;
    this->capacity = new_capacity;
}

void **vector_at(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < this->size);
    return this->array + position;
}

void vector_set(vector *this, size_t position, void *element) {
    assert(this);
    // your code here
    this->destructor(this->array[position]);
    this->array[position] = this->copy_constructor(element);
}

void *vector_get(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < this->capacity && position >= 0);
    return this->array[position];
}

void **vector_front(vector *this) {
    assert(this);
    // your code here
    assert(this->size > 0);
    return this->array[0];
}

void **vector_back(vector *this) {
    // your code here
    assert(this);
    assert(this->size > 0);
    return this->array[this->size - 1];
}

void vector_push_back(vector *this, void *element) {
    assert(this);
    // your code here
    if (this->size == this->capacity) {
        vector_reserve(this, this->size + 1);
    }
    this->array[this->size] = this->copy_constructor(element);
    this->size++;
}

void vector_pop_back(vector *this) {
    assert(this);
    // your code here
    this->destructor(this->array[this->size - 1]);
    this->array[this->size - 1] = NULL;
    this->size--;
}

void vector_insert(vector *this, size_t position, void *element) {
    assert(this);
    // your code here
    if (this->size == this->capacity) {
        vector_reserve(this, this->size + 1);
    }
    for (size_t i = this->size; i > position; i--) {
        this->destructor(this->array[i]);
        this->array[i] = this->copy_constructor(this->array[i - 1]);
    }
    this->array[position] = this->copy_constructor(element);
    this->size++;
}

void vector_erase(vector *this, size_t position) {
    assert(this);
    assert(position < vector_size(this));
    // your code here
    for (size_t i = position; i < this->size - 1; i++) {
        this->destructor(this->array[i]);
        this->array[i] = this->copy_constructor(this->array[i + 1]);
    }
    this->destructor(this->array[this->size - 1]);
    this->size--;
}

void vector_clear(vector *this) {
    // your code here
    for (size_t i = 0; i < this->size; i++){
        this->destructor(this->array[i]);
        this->array[i] = NULL;
    }
    this->size = 0;
}
