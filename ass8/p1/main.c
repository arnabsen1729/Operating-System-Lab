#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// struct for each node of the BST
struct bstnode {
  void *data;
  struct bstnode *lchild, *rchild;
};

// strct for the BST
struct gbst {
  int dsize;
  void (*printNode)(void *);
  int (*compareFn)(void *, void *);
  struct bstnode *root;
};

typedef struct gbst *genericBST;

// to create a Generic Binary Search Tree
genericBST createGBST(int dsize, int (*cfunction)(void *, void *),
                      void (*printNode)(void *)) {
  genericBST myBST = (genericBST)(malloc(sizeof(struct gbst)));
  myBST->dsize = dsize;
  myBST->printNode = printNode;
  myBST->compareFn = cfunction;
  myBST->root = NULL;

  return myBST;
}

// to search a node in the BST
struct bstnode *searchGBST(genericBST gt, void *d) {
  if (gt->root == NULL) {
    printf("EMPTY TREE!! \n");
    return NULL;
  }
  struct bstnode *ptr, *par;
  ptr = gt->root;
  while (ptr != NULL) {
    if (gt->compareFn(d, ptr->data) == 0) {
      return ptr;
    } else if (gt->compareFn(d, ptr->data) == 1) {
      par = ptr;
      ptr = ptr->rchild;
    } else {
      par = ptr;
      ptr = ptr->lchild;
    }
  }
  return par;
}

// to insert a node in the BST
void insertNodeGBST(genericBST gt, void *d) {
  struct bstnode *newNode = (struct bstnode *)malloc(sizeof(struct bstnode));
  newNode->data = malloc(gt->dsize);
  memcpy((void *)(newNode->data), (void *)d, gt->dsize);
  // newNode->data = d;

  if (gt->root == NULL) {
    printf("NODE INSERTED!!\n");
    gt->root = newNode;
    return;
  }

  struct bstnode *par = searchGBST(gt, d);
  if (gt->compareFn(par->data, d) == -1) {
    par->rchild = newNode;
  } else {
    par->lchild = newNode;
  }

  return;
}

// in order traversal of the BST
void inOrderTraverse(genericBST gt, struct bstnode *node) {
  if (node == NULL) return;
  inOrderTraverse(gt, node->lchild);
  gt->printNode(node);
  inOrderTraverse(gt, node->rchild);
}

// pre order traversal of the BST
void preOrderTraverse(genericBST gt, struct bstnode *node) {
  if (node == NULL) return;
  gt->printNode(node);
  preOrderTraverse(gt, node->lchild);
  preOrderTraverse(gt, node->rchild);
}

// post order traversal of the BST
void postOrderTraverse(genericBST gt, struct bstnode *node) {
  if (node == NULL) return;
  postOrderTraverse(gt, node->lchild);
  postOrderTraverse(gt, node->rchild);
  gt->printNode(node);
}

// traversal of the BST
int traverseGBST(genericBST gt, int order) {
  if (gt->root == NULL) {
    return 0;
  }

  switch (order) {
    case 0:
      inOrderTraverse(gt, gt->root);
      break;
    case 1:
      preOrderTraverse(gt, gt->root);
      break;
    case 2:
      postOrderTraverse(gt, gt->root);
      break;
    default:
      return 0;
  }

  return 1;
}

// compare function for the BST
int myComp(void *a, void *b) {
  if ((*((int *)a)) == (*((int *)b))) return 0;
  return (((*((int *)a)) < (*((int *)b))) ? -1 : 1);
}

// display function for the BST
void myPrint(void *a) {
  struct bstnode temp = (*(struct bstnode *)a);
  printf("VALUE: %d\n", *((int *)temp.data));
}

// inorder successor of a node
struct bstnode *inorderSuccessor(struct bstnode *root) {
  struct bstnode *current = root;
  while (current && current->lchild != NULL) current = current->lchild;
  return current;
}

// delete a node from the BST
struct bstnode *deleteNode(genericBST gt, struct bstnode *root, void *d) {
  if (root == NULL) return root;

  if (gt->compareFn(root->data, d) < 0) {
    root->rchild = deleteNode(gt, root->rchild, d);
  } else if (gt->compareFn(root->data, d) > 0) {
    root->lchild = deleteNode(gt, root->lchild, d);
  } else {
    if (root->lchild == NULL) {
      struct bstnode *temp = root->rchild;
      free(root);
      return temp;
    } else if (root->rchild == NULL) {
      struct bstnode *temp = root->lchild;
      free(root);
      return temp;
    }

    struct bstnode *temp = inorderSuccessor(root->rchild);
    root->data = temp->data;
    root->rchild = deleteNode(gt, root->rchild, temp->data);
  }
  return root;
}

// delete a node from the BST
int deleteNodeGBST(genericBST gt, void *d) {
  if (gt->root == NULL) {
    printf("Tree is empty!\n");
    return 0;
  }

  gt->root = deleteNode(gt, gt->root, d);
  return 1;
}

// thread function
void *operation(void *ptr) {
  genericBST gt = (genericBST)ptr;
  pthread_mutex_lock(&mutex);

  /* CRITICAL SECTION STARTS */
  int c;
  printf(
      "\nEnter an option to continue: \n1.INSERT \n2.SEARCH  \n3.DELETE "
      "\n4.TRAVERSAL \n5.EXIT \n:");
  scanf("%d", &c);
  if (c == 1) {
    int a;
    printf("\nEnter a number: ");
    scanf("%d", &a);
    void *a_ptr = &a;
    insertNodeGBST(gt, a_ptr);
    printf("Data inserted successfully!!\n");
  } else if (c == 2) {
    int a;
    printf("\nEnter a number to search: ");
    scanf("%d", &a);
    void *a_ptr = &a;
    struct bstnode *temp;
    temp = searchGBST(gt, a_ptr);
    if (gt->compareFn(a_ptr, temp->data) == 0) {
      printf("Data is present!!\n");
    } else {
      printf("Data not present!!\n");
    }
  } else if (c == 3) {
    int a;
    printf("\nEnter a number to delete: ");
    scanf("%d", &a);
    void *a_ptr = &a;
    if (deleteNodeGBST(gt, a_ptr) == 0) {
      printf("Deletion was unsuccessful!\n");
    } else {
      printf("Deletion was successful!\n");
    }
  } else if (c == 4) {
    int c2;
    printf(
        "\nEnter traversal order: \n1.IN-ORDER \n2.PRE-ORDER "
        "\n3.POST-ORDER \n:");
    scanf("%d", &c2);
    printf("\nThe elements are: \n");
    int res;
    res = traverseGBST(gt, c2 - 1);
    if (!res) {
      printf("\nOOps!! there has been some error!");
    }
    printf("\n");
  } else {
    printf("\nInvalid option!\n");
  }
  /* CRITICAL SECTION ENDS */
  pthread_mutex_unlock(&mutex);
}

int main() {
  int (*cfunction)(void *, void *) = &myComp;
  void (*printNode)(void *) = &myPrint;
  genericBST gt = createGBST((sizeof(int)), cfunction, myPrint);

  int thread_count = 5;

  pthread_t threads[thread_count];
  int iret[thread_count];

  // create the threads
  for (int i = 0; i < thread_count; i++) {
    iret[i] = pthread_create(&threads[i], NULL, operation, (void *)gt);
  }

  for (int i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
  }

  for (int i = 0; i < thread_count; i++) {
    if (iret[i] != 0) {
      printf("\nError - pthread_create() return code: %d\n", iret[i]);
    } else {
      printf("\nThread %d created successfully\n", i);
    }
  }

  return 0;
}
