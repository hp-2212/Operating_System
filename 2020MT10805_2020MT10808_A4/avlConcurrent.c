#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

// Define the Node struct
typedef struct Node {
    int val ;
    int height ;
    bool isLeaf ;
    struct Node* left ;    // Use struct Node* instead of Node*
    struct Node* right ;   // Use struct Node* instead of Node*
    struct Node* parent ;  // Use struct Node* instead of Node*
} Node;

typedef struct {
    Node* root ;
} Tree;

Tree* MyTree ;
pthread_mutex_t insertLock ;
pthread_mutex_t deleteLock ;
pthread_mutex_t searchLock ;
pthread_mutex_t inorderLock;
pthread_mutex_t readlock;
pthread_mutex_t writelock;

int max(int a, int b) {
    if ( a>=b ) return a ;
    return b ;
}

void swap(int *a, int *b) {
    *a = *a ^ *b;
    *b = *a ^ *b; 
    *a = *a ^ *b;
}

void updateHeight(Node* currNode) {
    currNode->height = 1 + max( currNode->left->height, currNode->right->height ) ;
}

int calcBalanceFactor(Node* currNode) {
    return currNode->left->height - currNode->right->height;
}

Node* rightRotate(Node* currNode){
    Node* leftChild = currNode->left;
    Node* rightChild = currNode->right;
    Node* currParent = currNode->parent;

    bool isLeftChild = false ;
    if ( currParent != NULL ) {
        if ( !currParent->left->isLeaf && currParent->left->val == currNode->val )
            isLeftChild = true ;
    }
    
    currNode->left = leftChild->right;
    leftChild->right->parent = currNode;
    leftChild->right = currNode;
    currNode->parent = leftChild;
    leftChild->parent = currParent;
    if ( currParent!=NULL ) {
        if(isLeftChild) currParent->left = leftChild;
        else currParent->right = leftChild;
    }
    else MyTree->root = leftChild ;
    
    updateHeight(currNode);
    updateHeight(leftChild);
    return leftChild;
}

Node* leftRotate(Node* currNode){
    Node* leftChild = currNode->left;
    Node* rightChild = currNode->right;
    Node* currParent = currNode->parent;

    bool isLeftChild = false ;
    if ( currParent != NULL ) {
        if ( !currParent->left->isLeaf && currParent->left->val == currNode->val )
            isLeftChild = true ;
    }

    currNode->right = rightChild->left;
    rightChild->left->parent = currNode;
    rightChild->left = currNode;
    currNode->parent = rightChild;
    rightChild->parent = currParent;
    if ( currParent!=NULL ) {
        if(isLeftChild) currParent->left = rightChild;
        else currParent->right = rightChild;
    }
    else MyTree->root = rightChild ;

    updateHeight(currNode);
    updateHeight(rightChild);
    return rightChild;
}

Node* searchNode(int value, Node* currNode) {
    if ( currNode->isLeaf  ) return currNode ;
    if ( currNode->val == value ) return currNode ;
    else if ( value < currNode->val ) return searchNode( value, currNode->left) ;
    else return searchNode( value, currNode->right) ;
}

void insertNode(int value) {
    Node* currNode = searchNode(value, MyTree->root);

    if ( !currNode->isLeaf ) return ;
    currNode->val = value ;
    currNode->isLeaf = false ;
    Node* l_child = (Node*)malloc(sizeof(Node)) ;
    Node* r_child = (Node*)malloc(sizeof(Node)) ;
    currNode->left = l_child ;
    currNode->right = r_child ;
    
    l_child->isLeaf = true ;
    r_child->isLeaf = true ;
    l_child->height = -1;
    r_child->height = -1;
    l_child->parent = currNode ;
    r_child->parent = currNode ;

    //Updating heights of ancestors
    Node* temp = currNode;
    while(temp!=NULL){
        temp->height = 1+max(temp->left->height, temp->right->height);
        int bf = calcBalanceFactor(temp);
        if(bf>1 && calcBalanceFactor(temp->left)>=0){
            temp = rightRotate(temp);
        }
        else if(bf<-1 && calcBalanceFactor(temp->right)<=0){
            temp = leftRotate(temp);
        }
        else if(bf>1 && calcBalanceFactor(temp->left)<0){
            temp->left = leftRotate(temp->left);
            temp = rightRotate(temp);
        }
        else if(bf<-1 && calcBalanceFactor(temp->right)>0){
            temp->right = rightRotate(temp->right);
            temp = leftRotate(temp);
        }
        temp = temp->parent;
    }

}

void deleteNode(Node* currNode) {
    if(currNode->isLeaf) return ;

    Node* leftChild = currNode->left;
    Node* rightChild = currNode->right;
    //Case of No child
    if(leftChild->isLeaf && rightChild->isLeaf) {
        free(leftChild);
        free(rightChild);
        currNode->isLeaf = true;
        currNode->height = -1;
    }
    //Case of only left child
    else if(rightChild->isLeaf) {
        Node* nextNode = leftChild;
        //Find immediate inorder predecessor
        while(!nextNode->right->isLeaf) nextNode = nextNode->right;
        swap(&currNode->val, &nextNode->val);
        deleteNode(nextNode);
    }
    //Case of 2 children
    else{
        Node* nextNode = rightChild;
        //Find immediate inorder succesor
        while(!nextNode->left->isLeaf) nextNode = nextNode->left;
        swap(&currNode->val, &nextNode->val);
        deleteNode(nextNode);
    }

    //Updating heights of ancestors
    Node* temp = currNode->parent ;
    while(temp!=NULL){
        temp->height = 1+max(temp->left->height, temp->right->height);
        int bf = calcBalanceFactor(temp);
        if(bf>1 && calcBalanceFactor(temp->left)>=0){
            temp = rightRotate(temp);
        }
        else if(bf<-1 && calcBalanceFactor(temp->right)<=0){
            temp = leftRotate(temp);
        }
        else if(bf>1 && calcBalanceFactor(temp->left)<0){
            temp->left = leftRotate(temp->left);
            temp = rightRotate(temp);
        }
        else if(bf<-1 && calcBalanceFactor(temp->right)>0){
            temp->right = rightRotate(temp->right);
            temp = leftRotate(temp);
        }
        temp = temp->parent;
    }
}

void inorder(Node* currNode){
    if(currNode->isLeaf) return ;
    inorder(currNode->left);
    printf("%d ", currNode->val);
    inorder(currNode->right);
}

void preorder(Node* currNode){
    if(currNode->isLeaf) return;
    printf("%d ", currNode->val);
    preorder(currNode->left);
    preorder(currNode->right);
}

Node* search(int value) {
    return searchNode(value, MyTree->root) ;
}

void insert(int value) {
    if ( MyTree->root == NULL ) {
        Node* rootNode = (Node*) malloc(sizeof(Node));
        rootNode->isLeaf = true;
        rootNode->height = -1 ;
        MyTree->root = rootNode;
    }
    insertNode(value) ;
}

void delete(int value) {
    if ( MyTree->root == NULL ) return ;
    Node* currNode = searchNode(value, MyTree->root);
    deleteNode(currNode) ;
}

bool isBalanced(Node* node) {
    if ( node->isLeaf ) return true;

    int balance = calcBalanceFactor(node);
    if (balance < -1 || balance > 1) {
        return false;
    }

    return isBalanced(node->left) && isBalanced(node->right);
}

void *insertThread(void *arg) {
    int *value = (int *)arg;
    //printf("Thread for insert = %d\n", *value);
    pthread_mutex_lock(&writelock);
    insert(*value);
    pthread_mutex_unlock(&writelock);
    free(value); 
    return NULL;
}

void *deleteThread(void *arg) {
    int *value = (int *)arg;
    //printf("Thread for delete = %d\n", *value);
    pthread_mutex_lock(&writelock);
    delete(*value);
    pthread_mutex_unlock(&writelock);
    free(value); 
    return NULL;
}

void* searchThread(void *arg) {
    int *value = (int *)arg;
    //printf("Thread for search = %d\n", *value);
    pthread_mutex_lock(&searchLock);
    bool contains = !(search(*value)->isLeaf);
    if(contains) printf("yes\n");
    else printf("no\n");
    pthread_mutex_unlock(&searchLock);
    free(value); 
    bool* result = (bool *)malloc(sizeof(bool));
    if(result) *result = contains;
    return (void*)result;
}

void *inorderThread(void *arg) {
    //int *value = (int *)arg;
    //printf("Thread for insert = %d\n", *value);
    pthread_mutex_lock(&inorderLock);
    inorder(MyTree->root);
    printf("\n");
    pthread_mutex_unlock(&inorderLock);
    //free(value); 
    return NULL;
}

int main() {

    MyTree = (Tree*) malloc(sizeof(Tree));

    char command[20];
    int commandList[1000], intParamsList[1000], commandCount = 0, value;
    pthread_t threads[1000];

    pthread_mutex_init(&searchLock, NULL);
    pthread_mutex_init(&inorderLock, NULL);
    pthread_mutex_init(&writelock, NULL);
    //pthread_mutex_init(&readlock, NULL);
    while (1) {
        //printf("Enter command: ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command)-1] = '\0';  
 
        if (strcmp(command, "exit") == 0) {
            printf("Exiting the program.\n");
            break;
        }
        else if (sscanf(command, "insert %d", &value) == 1) {
            commandList[commandCount] = 1 ;
            intParamsList[commandCount] = value ;
        } 
        else if (sscanf(command, "delete %d", &value) == 1) {
            commandList[commandCount] = 2 ;
            intParamsList[commandCount] = value ;
        } 
        else if (sscanf(command, "contains %d", &value) == 1) {
            commandList[commandCount] = 0 ;
            intParamsList[commandCount] = value ;
        } else if (strcmp(command, "in order") == 0) {
            commandList[commandCount] = -1 ;
            intParamsList[commandCount] = -1 ;
        } else {
            printf("Invalid command: %s\n", command);
            --commandCount ;
        }
        ++commandCount ;
    }

    for (int i = 0; i < commandCount; ++i) {
        if (commandList[i] == 1) {
            int *value = (int *)malloc(sizeof(int));
            *value = intParamsList[i];
            //printf("(in main) Thread for insert = %d\n", *value);
            
            pthread_create(&threads[i], NULL, insertThread, value);
        }
        else if (commandList[i] == 2) {
            int *value = (int *)malloc(sizeof(int));
            *value = intParamsList[i];
            //printf("(in main) Thread for Delete = %d\n", *value);
            pthread_create(&threads[i], NULL, deleteThread, value);
        }
        else if (commandList[i] == 0) {
            int *value = (int *)malloc(sizeof(int));
            *value = intParamsList[i];
            //printf("(in main) Thread for Search = %d\n", *value);
            pthread_create(&threads[i], NULL, searchThread, value);
        }
        else if ( commandList[i] == -1 ) {
            //int *value = (int *)malloc(sizeof(int));
            //*value = intParamsList[i];
            //printf("(in main) Thread for Search = %d\n", *value);
            pthread_create(&threads[i], NULL, inorderThread, NULL);
        }
    }

    for (int i = 0; i < commandCount; ++i) {
        pthread_join(threads[i], NULL);
    }
    preorder(MyTree->root); printf("\n") ;
    return 0;
}