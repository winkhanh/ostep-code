#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>       

// global ticket count, global weight
int gtickets = 0, gweight = 0;
const int prio_to_weight[40] = {
/*-20 */ 88761, 71755,  56483,  46273,  36291,
/*-15 */ 29154, 23254,  18705,  14949,  11916,
/*-10 */ 9548,  7620,   6100,   4904,   3906,
/* -5 */ 3121,  2501,   1991,   1586,   1277,
/*  0 */ 1024,  820,    655,    526,    423,
/*  5 */ 335,   272,    215,    172,    137,
/* 10 */ 110,   87,     70,     56,     45,
/* 15 */ 36,    29,     23,     18,     15,
};
struct task_t{
    int id;
    int tickets;
    int prior;
    int runtime;
    float vruntime;
};
struct node_t {
    struct task_t *task;
    struct node_t *left;
    struct node_t *right;
    struct node_t *par;
};

int cmp(struct node_t *a, struct node_t *b){
    return a->task->vruntime+0.000001 <  b->task->vruntime;
}
struct node_t* findMin(struct node_t* cur){
    struct node_t* tmp=cur;
    while (tmp!=NULL && tmp->left!=NULL)
        tmp=tmp->left;
    return tmp;
}

struct node_t *insert_to_tree(struct node_t *cur, struct node_t *node){
    if (cur==NULL) return node;
    if (cmp(node,cur)){
    
        cur->left = insert_to_tree(cur->left,node);
        cur->left->par=cur;
        
    }else{
    
        cur->right = insert_to_tree(cur->right,node);
        cur->right->par=cur;
        
    }
    return cur;
}
struct node_t *delete_from_tree(struct node_t *cur, struct node_t *node){
    if (cur==NULL) return cur;
    if (cmp(node,cur)){
        cur->left=delete_from_tree(cur->left,node);
    }else if (cmp(cur,node)){
        cur->right=delete_from_tree(cur->right,node);
    }
    if (cur->left==NULL){
        struct node_t *tmp=cur->right;
        free(cur);
        return tmp;
    }else if (cur->right==NULL){
        struct node_t *tmp=cur->left;
        free(cur);
        return tmp;
    }
    struct node_t * tmp=findMin(cur->right);
    cur->task=tmp->task;
    cur->right=delete_from_tree(cur->right,tmp);
    return cur;
}

struct node_t *head = NULL;
struct task_t *create_a_task(int maxSize, int id){
    struct task_t *tmp= malloc(sizeof(struct task_t));
    tmp->id=id;
    tmp->tickets = random()%maxSize;
    tmp->prior = random()%40;
    tmp->vruntime=0;
    tmp->runtime=0;
    return tmp;
}
struct node_t *create_a_node(struct task_t *task){
    struct node_t *tmp=malloc(sizeof(struct node_t));
    tmp->task=task;
    tmp->left=NULL;
    tmp->right=NULL;
    tmp->par=NULL;
    return tmp;
}
void insert_node(struct node_t *node) {
    assert(node != NULL);
    head = insert_to_tree(head,node);
}
void delete_node(struct node_t *node){
    assert(node != NULL);
    head = delete_from_tree(head,node);
}


struct node_t* find_min(){
    return findMin(head);
}
void print_node(struct node_t *curr){
    printf("[%d : ", curr->task->id);
    printf("%d--", curr->task->tickets);
    printf(" %d ]\n", curr->task->prior-20);
}
void print_list(struct node_t *curr) {
    print_node(curr);
    if (curr->left!=NULL) print_list(curr->left);
    if (curr->right!=NULL) print_list(curr->right);
}
void generate_task(int max_size,int n){
    struct node_t *node;
    struct task_t *task;
    for (int i = 0;i < n; i++){//create and insert tasks into the list
        task=create_a_task(max_size,i);
        node=create_a_node(task);
        gtickets    += task->tickets;
        gweight     += prio_to_weight[task->prior];
        insert_node(node);
    }
}




int main(int argc, char *argv[])
{
    int if_debug=1;
    if (argc != 6) {
	fprintf(stderr, "usage: cfs <seed> <latency> <min> <num of tasks> <max size of task>\n");
    //seed for generate tasks
	exit(1);
    }
    int seed  = atoi(argv[1]);
    int sched_latency = atoi(argv[2]);
    int min_granularity  = atoi(argv[3]);
    int n = atoi(argv[4]);
    int max_size = atoi(argv[5]);
    srandom(seed);
    
    // populate list with some number of jobs, each
    // with some number of tickets
    
    generate_task(max_size,n);
    if (if_debug)
        print_list(head);
    clock_t begin = clock();
    int run_time=sched_latency/n;
    if (run_time<min_granularity) run_time=min_granularity;
    int timeStamp=0;
    while (head && n!=0){//while list is not empty
        printf("Start");
        struct node_t* cur=find_min();
        struct task_t *task=cur->task;
        printf("found %d",task->id);
        task->runtime+=run_time;
        float vincreasing=((float)(prio_to_weight[0])/prio_to_weight[cur->task->prior]);
        task->vruntime+=vincreasing;
        printf("Medium");
        if (if_debug)
            printf("At time %d, %d runs\n",timeStamp,task->id);
        delete_node(cur);
        timeStamp+=run_time;
        
        if (task->runtime>task->tickets)//done
        {
            if (if_debug)
                printf("id %d done, %d left\n",task->id, n-1);
            gweight     -= prio_to_weight[task->prior];
            free(task);
            n--;//reduce the number of tasks
            
            if (n!=0)
            {   
                run_time=sched_latency/n;
                if (run_time<min_granularity) run_time=min_granularity;//recalculate the timeslice
            }
            if (if_debug)
                printf("id %d done, %d left %ld \n",task->id, n-1,(long)(head));
        }else{
            if (if_debug)
                printf("HUH\n");
            struct node_t *node=create_a_node(task);
            insert_node(node);
        }
        printf("ABC\n");
        
    }
    clock_t end = clock();
    printf("Running time: %f\n",(double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}

