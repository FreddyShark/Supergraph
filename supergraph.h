#ifndef SUPERGRAPH_H
#define SUPERGRAPH_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define TRUE 1
#define FALSE 0

typedef struct query_helper query_helper;
typedef struct result result;
typedef struct criteria criteria;
typedef struct user user;
typedef struct post post;

struct query_helper {
	int dummy_field;
	// my additions
	uint32_t routine;
	result* result_ptr;
};

struct result {
	void** elements;
	size_t n_elements;
};

struct criteria {
	float oc_threshold;
	float acc_rep_threshold;
	float bot_net_threshold;
};

struct user {
	uint64_t user_id;
	size_t* follower_idxs;
	size_t n_followers;
	size_t* following_idxs;
	size_t n_following;
	size_t* post_idxs;
	size_t n_posts;
};

struct post {
	uint64_t pst_id;
	uint64_t timestamp;
	size_t* reposted_idxs;
	size_t n_reposted;
};

//===============================================================================
// the following defintions are part of the queue implementation on gist.github.com
// taken from https://gist.github.com/ArnonEilat/4471278
// created by https://github.com/ArnonEilat
// =============================================================================
/* a link in the queue, holds the info and point to the next Node*/
typedef struct {
    int info;
} DATA;

typedef struct Node_t {
    DATA data;
    struct Node_t *prev;
} NODE;

/* the HEAD of the Queue, hold the amount of node's that are in the queue*/
typedef struct Queue {
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;
//==============================================================================
// the following functions were taken from the queue implementation on gist.github.com
// https://gist.github.com/ArnonEilat/4471278
// function definitions in supergraph.c
//================================================================================
Queue *ConstructQueue(int limit);
void DestructQueue(Queue *queue);
int Enqueue(Queue *pQueue, NODE *item);
NODE *Dequeue(Queue *pQueue);
int isEmpty(Queue* pQueue);
//================================================================================

query_helper* engine_setup(size_t n_processors);

result* find_all_reposts(post* posts, size_t count, uint64_t post_id, query_helper* helper);

result* find_original(post* posts, size_t count, uint64_t post_id, query_helper* helper);

result* shortest_user_link(user* users, size_t count, uint64_t userA, uint64_t userB, query_helper* helper);

result* find_bots(user* users, size_t user_count, post* posts, size_t post_count, criteria* crit, query_helper* helper);


void engine_cleanup(query_helper* helpers);

// added helper fucntions


#endif

