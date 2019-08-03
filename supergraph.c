#include "supergraph.h"

static int find_post_index(post* posts, size_t count, uint64_t post_id);
static int find_user_index(user* users, size_t count, uint64_t user_id);
static result* BFS(user* users, size_t count, uint64_t userA_idx, uint64_t userB_idx);
static result* get_null_result();

query_helper* engine_setup(size_t n_processors)
{
	query_helper* ptr = malloc(sizeof(*ptr));
	return ptr;
}

result* find_all_reposts(post* posts, size_t count, uint64_t post_id, query_helper* helper)
 {
     int idx;
     result* thisResult;
	 int found_cnt;
	 int child_idx;
	 Queue* pnt_Q;           // queue
	 NODE* pnt_N;            // queue nodes

     idx = find_post_index(posts, count, post_id);
     // if post not found
     if (idx == -1)
     	return get_null_result();

	 thisResult = malloc(sizeof(*thisResult));
	 thisResult->elements = malloc(sizeof(post*)*count);
	 // insert this post into result
	 thisResult->elements[0] = posts + idx;
	 thisResult->n_elements = 1;
	 found_cnt = 1;

	 pnt_Q = ConstructQueue(count);
	 pnt_N = (NODE*) malloc(sizeof(*pnt_N));
	 pnt_N->data.info = idx;
     Enqueue(pnt_Q, pnt_N);
	 NODE* visiting;

	 while (!isEmpty(pnt_Q))
	 {
		 visiting = Dequeue(pnt_Q);
		 idx = visiting->data.info;
		 free(visiting);
		 for (int i = 0; i < posts[idx].n_reposted; i++)
		 {
			 child_idx = posts[idx].reposted_idxs[i];
			 thisResult->elements[found_cnt] = posts + child_idx;
			 found_cnt++;
			 pnt_N = (NODE*)malloc(sizeof(*pnt_N));
			 pnt_N->data.info = child_idx;
			 Enqueue(pnt_Q, pnt_N);
		 }
	 }
	 DestructQueue(pnt_Q);

	 thisResult->n_elements = found_cnt;
	 return thisResult;
}

result* find_original(post* posts, size_t count, uint64_t post_id, query_helper* helper)
{
    int idx;
    int orig_pst_idx;
    result* thisResult;
	int orig_found;			// flag

    idx = find_post_index(posts, count, post_id);
	// if no such post exists
	if (idx == -1)
        return get_null_result();

	thisResult = malloc(sizeof(*thisResult));
 	thisResult->elements = malloc(sizeof(post*)*1);

	orig_pst_idx = idx;
	orig_found = FALSE;
	while (!orig_found)
	{
		orig_found = TRUE;
		for (int i = 0; i < count; i++)
		{
			// move through each post's reposted_idxs list
			for (int j = 0; j < posts[i].n_reposted; j++)
			{
				// if index of parent post in question is found as one of the children
				//  of post[i] we have not reached the original post
				if (posts[i].reposted_idxs[j] == orig_pst_idx)
				{
					//set child as parent (original)
					orig_pst_idx = i;
					orig_found = FALSE;
					break;
				}
			}
			// no continuation needed. move to next parent
			if (!orig_found)
				break;
		}
	}

	thisResult->elements[0] = posts + orig_pst_idx;
	thisResult->n_elements = 1;
	return thisResult;
}

result* shortest_user_link(user* users, size_t count, uint64_t userA, uint64_t userB, query_helper* helper)
{
    int userA_idx;
    int userB_idx;
    result* path_AtoB;
    result* path_BtoA;

    userA_idx = find_user_index(users, count, userA);
    userB_idx = find_user_index(users, count, userB);

    if (userA_idx == -1 || userB_idx == -1 || userA_idx == userB_idx)
	   return get_null_result();

	path_AtoB = BFS(users, count, userA_idx, userB_idx);
    path_BtoA = BFS(users, count, userB_idx, userA_idx);

	// if no path was found both paths will be set to null. return either
	if ((path_AtoB->elements[0] == NULL) && (path_BtoA->elements[0] == NULL))
	{
		free(path_AtoB->elements);
        free(path_AtoB);
		return path_BtoA;
	}
	// if path_AtoB does not exist
	else if (path_AtoB->elements[0] == NULL)
	{
		free(path_AtoB->elements);
        free(path_AtoB);
		return path_BtoA;
	}
	// if path_BtoA does not exist
	else if (path_BtoA->elements[0] == NULL)
	{
		free(path_BtoA->elements);
		free(path_BtoA);
		return path_AtoB;
	}
	// if path_BtoA is shorter
	else if (path_AtoB->n_elements > path_BtoA->n_elements)
	{
		free(path_AtoB->elements);
        free(path_AtoB);
		return path_BtoA;
	}
	// if path_AtoB is shorter or both paths are equal
	else if (path_AtoB->n_elements <= path_BtoA->n_elements)
	{
		free(path_BtoA->elements);
		free(path_BtoA);
		return path_AtoB;
	}
	else
	{
		printf("\nerror in path finding");
		return NULL;
	}
}

// BFS adapted from HackerRank video "Algorithms: Solve 'Shortest Reach' Using BFS"
// https://www.youtube.com/watch?v=0XgVhsMOcQM&index=1&list=RD0XgVhsMOcQM
static result* BFS(user* users, size_t count, uint64_t userA_idx, uint64_t userB_idx)
{
	int path_found;			// flag to indicate path found
    int dist_AtoB;
	int neighbor;			// hold index position of neighbor
    Queue* pnt_Q;           // queue for visited users
    NODE* pnt_N;            // queue nodes
    int* distances;     	// array holding distances of users from userA
	int* pred;				/* holds index of user prior to current user
							 advised by Marco Lam & Luke Tuthill on Ed */
    result* thisResult;

    pnt_Q = ConstructQueue(count);
    pnt_N = (NODE*) malloc(sizeof(*pnt_N));
	distances = malloc(sizeof(int)*count);
	pred = malloc(sizeof(int)*count);
    // fill array with -1 indicating unvisited
    for (int i = 0; i < count; i++)
    {
        distances[i] = -1;
		pred[i] = -1;
    }
    // insert starting user into queue
    pnt_N->data.info = userA_idx;
    Enqueue(pnt_Q, pnt_N);
    // insert distance 0 to self (starting user_idx)
    distances[userA_idx] = 0;

    NODE* visiting;
    neighbor = -1;
	path_found = FALSE;
    while (!isEmpty(pnt_Q) && neighbor != userB_idx)
    {
        visiting = Dequeue(pnt_Q);
        int this_user_idx = visiting->data.info;
        free(visiting);
        // for each following
        for (int i = 0; i < users[this_user_idx].n_following; i++)
        {
            neighbor = users[this_user_idx].following_idxs[i];
            // if unvisited
            if (distances[neighbor] == -1)
            {
                distances[neighbor] = distances[this_user_idx] + 1;
				// store predessesor
				pred[neighbor] = this_user_idx;
                pnt_N = (NODE*) malloc(sizeof(*pnt_N));
                pnt_N->data.info = neighbor;
                Enqueue(pnt_Q, pnt_N);
            }
            // if target user is reached
            if (neighbor == userB_idx)
            {
                dist_AtoB = distances[neighbor];
				path_found = TRUE;
                break;
            }
        }
    }
    DestructQueue(pnt_Q);
	free(distances);

	if (path_found == FALSE)
	{
		free(pred);
		return get_null_result();
	}

    thisResult = malloc(sizeof(*thisResult));
    thisResult->elements = malloc(sizeof(user*)*count);

	int idx = userB_idx;
	// store last user in path
	thisResult->elements[dist_AtoB] = &users[idx];
	// store predecessors. dist_AtoB = (# of users -1)
    for (int i = (dist_AtoB-1); i >= 0; i--)
    {
        thisResult->elements[i] = &users[pred[idx]];
		idx = pred[idx];
    }
    thisResult->n_elements = dist_AtoB + 1;

	free(pred);
    return thisResult;
}

result* find_bots(user* users, size_t user_count, post* posts, size_t post_count, criteria* crit, query_helper* helper)
{
	float repost_cnt;
	//int bot_added;		// flag to indicate bot is added
	int post_idx;
	int follower_idx;
	int bot_cnt;
	float bots_follow_cnt;
	int* reposts;			// indicies represent post indicies
	int* bot_idxs;			// indicies represent bot indicies
	result* thisResult;

	if (crit->oc_threshold > 1 || crit->acc_rep_threshold > 1 || crit->bot_net_threshold > 1 ||
		crit->oc_threshold < 0 || crit->acc_rep_threshold < 0 || crit->bot_net_threshold < 0)
			return get_null_result();

	reposts = malloc(sizeof(*reposts)*post_count);
	bot_idxs = malloc(sizeof(*bot_idxs)*user_count);

	// intialize
	for (int i = 0; i < post_count; i++)
	{
		reposts[i] = FALSE;
		bot_idxs[i] = FALSE;
	}

	// flag reposts
	// for each post
	for (int i = 0; i < post_count; i++)
	{
		// for each repost of each post
		for (int j = 0; j < posts[i].n_reposted; j++)
		{
			post_idx = posts[i].reposted_idxs[j];
			reposts[post_idx] = TRUE;
		}
	}

	thisResult = malloc(sizeof(*thisResult));
	thisResult->elements = malloc(sizeof(user*)*user_count);
	bot_cnt = 0;
	// for each user
	for (int i = 0; i < user_count; i++)
	{
		repost_cnt = 0;
		// for each post by user
		for (int j = 0; j < users[i].n_posts; j++)
		{
			post_idx = users[i].post_idxs[j];

			if (reposts[post_idx] == TRUE)
				repost_cnt++;
		}
		// REPOSTS MORE THAN POSTS TEST
		if ((users[i].n_posts != 0) &&
			(repost_cnt/users[i].n_posts > crit->oc_threshold))
		{
			if (bot_idxs[i] != TRUE)
			{
				thisResult->elements[bot_cnt] = &users[i];
				bot_idxs[i] = TRUE;
				bot_cnt++;
			}
		}
		// ACCOUNT REP TEST
		else if (((users[i].n_followers + users[i].n_following) != 0) &&
		 	((float)(users[i].n_followers)/(users[i].n_followers + users[i].n_following)) <
		 		crit->acc_rep_threshold)
		{
			if (bot_idxs[i] != TRUE)
			{
				thisResult->elements[bot_cnt] = &users[i];
				bot_idxs[i] = TRUE;
				bot_cnt++;
			}
		}
	}

	// DISCRETE BOTS TEST
	//bot_added = FALSE;
	// for each user
	for (int i = 0; i < user_count; i++)
	{
		bots_follow_cnt = 0;
		// for each follower
		for (int j = 0; j < users[i].n_followers; j++)
		{
			follower_idx = users[i].follower_idxs[j];
			if (bot_idxs[follower_idx] == TRUE)
				bots_follow_cnt++;
		}

		if ((users[i].n_followers != 0) &&
			(bots_follow_cnt/users[i].n_followers > crit->bot_net_threshold))
		{
			if (bot_idxs[i] != TRUE)
			{
				thisResult->elements[bot_cnt] = &users[i];
				bot_idxs[i] = TRUE;
				bot_cnt++;
				i = -1;		// reset loop
			}
		}
	}
	thisResult->n_elements = bot_cnt;

	free(reposts);
	free(bot_idxs);
	return thisResult;
}

void engine_cleanup(query_helper* helpers) {
	//Clean up your engine
	free(helpers);
}

static int find_post_index(post* posts, size_t count, uint64_t post_id)
{
    int idx = -1;
    for (int i = 0; i < count; i++)
    {
        // find specified post
        if (posts[i].pst_id == post_id)
        {
           idx = i;
           break;
        }
    }
    return idx;
}

// returns -1 if search failed
static int find_user_index(user* users, size_t count, uint64_t user_id)
{
    int idx = -1;
    for (int i = 0; i < count; i++)
    {
        // find specified post
        if (users[i].user_id == user_id)
        {
           idx = i;
           break;
        }
    }
    return idx;
}

static result* get_null_result()
{
    result* thisResult;
    thisResult = malloc(sizeof(*thisResult));
    thisResult->elements = malloc(sizeof(post*));
    thisResult->elements[0] = NULL;
    thisResult->n_elements = 0;
    return thisResult;
}

// the following implementation of a queue was taken from gist.github.com
// https://gist.github.com/ArnonEilat/4471278
// created by https://gist.github.com/ArnonEilat
//==================================================================================

Queue *ConstructQueue(int limit) {
    Queue *queue = (Queue*) malloc(sizeof (Queue));
    if (queue == NULL) {
        return NULL;
    }
    if (limit <= 0) {
        limit = 65535;
    }
    queue->limit = limit;
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void DestructQueue(Queue *queue) {
    NODE *pN;
    while (!isEmpty(queue)) {
        pN = Dequeue(queue);
        free(pN);
    }
    free(queue);
}

int Enqueue(Queue *pQueue, NODE *item) {
    /* Bad parameter */
    if ((pQueue == NULL) || (item == NULL)) {
        return FALSE;
    }
    // if(pQueue->limit != 0)
    if (pQueue->size >= pQueue->limit) {
        return FALSE;
    }
    /*the queue is empty*/
    item->prev = NULL;
    if (pQueue->size == 0) {
        pQueue->head = item;
        pQueue->tail = item;

    } else {
        /*adding item to the end of the queue*/
        pQueue->tail->prev = item;
        pQueue->tail = item;
    }
    pQueue->size++;
    return TRUE;
}

NODE * Dequeue(Queue *pQueue) {
    /*the queue is empty or bad param*/
    NODE *item;
    if (isEmpty(pQueue))
        return NULL;
    item = pQueue->head;
    pQueue->head = (pQueue->head)->prev;
    pQueue->size--;
    return item;
}

int isEmpty(Queue* pQueue) {
    if (pQueue == NULL) {
        return FALSE;
    }
    if (pQueue->size == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}
//===================================================================


