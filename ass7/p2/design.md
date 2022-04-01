# Shared BST

Since we have multiple BSTs we will use an array of BST node ptr, which will be the array of root pointers of the BST.

```c++
struct node {
  int data;
  node *left;
  node *right;
};
```

So the BST array would be `node *trees[]`. This will be local to the producer process.

## Local Data Structure

- the array of BST ptrs.

## Shared Data Structure

- The other process will send instructions like which BST to update and some meta data for the instruction. This can be achieved with semaphores.
