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
    struct task_t task;
    struct node_t *left;
    struct node_t *right;
    struct node_t *par;
    int color;//0 is black
};
int cmp(struct node_t *a, struct node_t *b){
    return a->task.vruntime<b->task.vruntime;
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

void R_rotate(struct node_t *cur, struct node_t **head){
    struct node_t *l = cur->left;
    cur->left = l->right;
    if (cur->left)
        cur->left->par=cur;
    l->par=cur->par;
    if (!cur->par)
        (*head)=l;
    else if (cur==cur->par->left)
        cur->par->left=l;
    else 
        cur->par->right=l;
    l->right=cur;
    cur->par=l;
}

void L_rotate(struct node_t *cur, struct node_t **head){
    struct node_t *r = cur->right;
    cur->right = r->left;
    if (cur->right)
        cur->right->par=cur;
    r->par=cur->par;
    if (!cur->par)
        (*head)=r;
    else if (cur==cur->par->left)
        cur->par->left=r;
    else 
        cur->par->right=r;
    r->left=cur;
    cur->par=r;
}

void rebalance(struct node_t* root,struct node_t* cur){
    struct node_t* par=NULL;
    struct node_t* grand_par=NULL;
    while ((cur!=root) && (cur->color!=0) && (cur->par->color==1)){
        par=cur->par;
        grand_par=par->par;
        if (par==grand_par->left)
        {
            struct node_t* neigh_par=grand_par->right;
            if (neigh_par!=NULL && neigh_par->color==1){
                grand_par->color=1;
                par->color=0;
                neigh_par->color=0;
                cur=grand_par;
            }else{
                if (cur==par->right){
                    L_rotate(par,&root);
                    cur=par;
                    par=cur->par;
                }
                R_rotate(grand_par,&root);
                int tmp= par->color;
                par->color=grand_par->color;
                grand_par->color=tmp;
                cur=par;
            }
        }else{
            struct node_t* neigh_par=grand_par->left;
            if (neigh_par!=NULL && neigh_par->color==1){
                grand_par->color=1;
                par->color=0;
                neigh_par->color=0;
                cur=grand_par;
            }else{
                if (cur==par->left){
                    R_rotate(par,&root);
                    cur=par;
                    par=cur->par;
                }
                L_rotate(grand_par,&root);
                int tmp= par->color;
                par->color=grand_par->color;
                grand_par->color=tmp;
                cur=par;
            }
        }
    }
    root->color=0;
}

struct node_t *head = NULL;
struct node_t *create_a_task(int maxSize, int id){
    struct node_t *tmp= malloc(sizeof(struct node_t));
    tmp->task.id=id;
    tmp->task.tickets = random()%maxSize;
    tmp->task.prior = random()%40;
    tmp->task.vruntime=0;
    tmp->task.runtime=0;
    tmp->left = NULL;
    tmp->right= NULL;
    tmp->par = NULL;
    tmp->color=0;
    return tmp;
}
void insert_node(struct node_t *node) {
    assert(node != NULL);
    head = insert_to_tree(head,node);
    rebalance(head,node);
    gtickets    += node->task.tickets;
    gweight     += prio_to_weight[node->task.prior];
}
struct node_t* findMin(struct node_t* cur){
    struct node_t* tmp=cur;
    while (tmp!=NULL && tmp->left!=NULL)
        tmp=tmp->left;
    return tmp;
}
struct node_t* to_be_delete(struct node_t* cur){
    if (cur->left==NULL && cur->right==NULL)
        return cur;
    if (!(cur->left!=NULL && cur->right!=NULL))
        return cur;
    struct node_t* tmp=findMin(cur);
    cur->task=tmp->task;
    return to_be_delete(tmp);
}

void delete_node(struct node_t *node){
    assert(node != NULL);
    gtickets    -= node->task.tickets;
    gweight     -= prio_to_weight[node->task.prior];
    struct node_t* to_delete=to_be_delete(node);
    struct node_t* child;
    if (to_delete->left==NULL) child=to_delete->right;
    else child=to_delete->left;
    if (to_delete->color==1 || (child!=NULL && child->color==1)){
        if (child!=NULL && child->color==1){
            child->color=0;
        }
        *to_delete=*child;
        free(child);
    }else{
        *to_delete=*child;
        to_delete->color=-1;
        
        free(child);
    }
}
struct node_t* find_min(){
    struct node_t *tmp=head, *res=head;
    while (tmp){
            if (!res || tmp->task.vruntime < res->task.vruntime){
                res=tmp;
            }
        tmp=tmp->next;
    }
    return res;
}
void print_node(struct node_t *curr){
    printf("[%d : ", curr->task.id);
    printf("%d--", curr->task.tickets);
    printf(" %d ]\n", curr->task.prior-20);
}
void print_list() {
    struct node_t *curr = head;
    printf("List: ");
    while (curr) {
        print_node(curr);
        curr = curr->next;
    }
    printf("\n");
}
void generate_task(int max_size,int n){
    struct node_t *tmp;
    for (int i = 0;i < n; i++){//create and insert tasks into the list
        tmp=create_a_task(max_size,i);
        insert_node(tmp);
    }
}




int main(int argc, char *argv[])
{
    int if_debug=0;
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
        print_list();
    clock_t begin = clock();
    int run_time=sched_latency/n;
    if (run_time<min_granularity) run_time=min_granularity;
    int timeStamp=0;
    while (head){//while list is not empty
        struct node_t* cur=find_min();
        cur->task.runtime+=run_time;
        float vincreasing=((float)(prio_to_weight[0])/prio_to_weight[cur->task.prior]);
        cur->task.vruntime+=vincreasing;
        if (if_debug)
            printf("At time %d, %d runs\n",timeStamp,cur->task.id);
        if (cur->task.runtime>cur->task.tickets)//done
        {
            if (if_debug)
                printf("id %d done\n",cur->task.id);
            delete_node(cur);
            n--;//reduce the number of tasks
            if (n!=0)
            {   
                run_time=sched_latency/n;
                if (run_time<min_granularity) run_time=min_granularity;//recalculate the timeslice
            }

        }
        timeStamp+=run_time;
    }
    clock_t end = clock();
    printf("Running time: %f\n",(double)(end - begin) / CLOCKS_PER_SEC);
    return 0;
}

