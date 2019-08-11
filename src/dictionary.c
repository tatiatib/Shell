#include "dictionary.h"

/*
 * Returns the index that given character should have
 * in the children array of struct DictionaryNode.
 * An assert is raised
 * Allowed characters:
 * Uppercase english letters ([A-Z]), digits (0-9), underscore sign "_".
 */
__attribute__((unused)) static int child_index(char c);

/*Constructs a new node*/
static void node_new(DictionaryNode *node, void *value);


static void freeNode(DictionaryNode *ptr, DictionaryFreeFunction freeFn);

void dictionary_new(DictionaryNode *root, DictionaryFreeFunction freeFn) {

    node_new(root, NULL);
    root->count = 0;
    root->freeFn = freeFn;

}

void dictionary_dispose(DictionaryNode *d) {
    for (int i = 0; i < ALLOWED_CHARACTER_COUNT; ++i) {
        freeNode((DictionaryNode *) d->children[i], d->freeFn);
    }
    if (d->freeFn && d->value) d->freeFn(d->value);
}

static void freeNode(DictionaryNode *ptr, DictionaryFreeFunction freeFn) {
    if (!ptr) return;
    for (int i = 0; i < ALLOWED_CHARACTER_COUNT; ++i) {
        freeNode((DictionaryNode *) ptr->children[i], freeFn);
    }
    if (ptr->value && freeFn) freeFn(ptr->value);
    free(ptr);
}

size_t dictionary_size(const DictionaryNode *d) {

    return d->count;
}

void dictionary_put(DictionaryNode *d, const char *key, const void *value) {
    DictionaryNode *node = d;
    assert(key);
    d->count++;
    size_t l = strlen(key);
    for (int i = 0; i < l; ++i) {
        int ci = child_index(key[i]);
        if (!node->children[ci]) {

            node->children[ci] = malloc(sizeof(DictionaryNode));
            node_new((DictionaryNode *) node->children[ci], NULL);
        }
        node = (DictionaryNode *) node->children[ci];
        if (i == l - 1) {
            if (node->value && d->freeFn) {
                d->freeFn(node->value);
            }
            node->value = (void *) value;
        }
    }

}

void *dictionary_get(const DictionaryNode *d, const char *key) {
    DictionaryNode *resultNode = (DictionaryNode *) d;
    size_t l = strlen(key);
    for (int i = 0; i < l; ++i) {
        if (resultNode) resultNode = (DictionaryNode *) resultNode->children[child_index(key[i])];
    }
    if (resultNode != NULL) return resultNode->value;
    return NULL;
}

void *dictionary_remove(DictionaryNode *d, const char *key) {
    //TODO: implement
    return NULL;
}

static int child_index(char c) {
    if (in_range(c, 'A', 'Z')) return c - 'A';
    if (in_range(c, 'a', 'z')) return 26 + c - 'a';
    if (in_range(c, '0', '9')) return 52 + c - '0';
    if (c == '_') return 62;
    return -1;
}


void node_new(DictionaryNode *node, void *value) {
    node->n_children = 0;
    node->value = value;
    memset(node->children, 0, ALLOWED_CHARACTER_COUNT * sizeof(DictionaryNode *));

}
