
typedef struct Node {
  void *key;
  void *value;
  struct Node *next;
} Node;

typedef struct {
  int size;
  Node **buckets;
  unsigned int (*hash_func)(void *);
  int (*compare_func)(void *, void *);
  void (*print_func)(void *, void *);
} HashTable;

HashTable *create_table(int size, unsigned int (*h)(void *),
                        int (*c)(void *, void *), void (*p)(void *, void *));

void insert(HashTable *table, void *key, void *value);

void *lookup(HashTable *table, void *key);

void print_table(HashTable *table);

void free_table(HashTable *table);
void free_table_with_cb(HashTable *table, void (*free_fn)(void *, void *));
