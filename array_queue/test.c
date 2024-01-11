#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct array_queue {
	void          **data;
	int             head, tail, capacity, size;
	pthread_mutex_t lock;
};

#define pr printf

/**
 * @brief 初始化队列
 * @param  q
 * @param  capacity
 */
int arrque_init(struct array_queue *q, int capacity)
{
	q->data = (void **)malloc(sizeof(void *) * capacity);
	if (!q->data)
		return -1;
	q->head = 0;
	q->tail = -1;
	q->size = 0;
	q->capacity = capacity;
	pthread_mutex_init(&(q->lock), NULL);
	return 0;
}

/**
 * @brief 销毁队列
 * @param  q
 */
void arrque_deinit(struct array_queue *q)
{
	free(q->data);
	q->data = NULL;
	q->head = 0;
	q->tail = -1;
	q->size = 0;
	q->capacity = 0;
	pthread_mutex_destroy(&q->lock);
}

// 判断队列是否为空
int arrque_is_empty(struct array_queue *q)
{
	int ret = 0;
	pthread_mutex_lock(&q->lock);
	ret = !q->size;
	pthread_mutex_unlock(&q->lock);
	return ret;
}

// 判断队列是否已满
int arrque_is_full(struct array_queue *q)
{
	int ret = 0;
	pthread_mutex_lock(&q->lock);
	ret = q->size == q->capacity;
	pthread_mutex_unlock(&q->lock);
	return ret;
}

// 查有效元素个数
int arrque_size(struct array_queue *q)
{
	int ret = 0;
	pthread_mutex_lock(&q->lock);
	ret = q->size;
	pthread_mutex_unlock(&q->lock);
	return ret;
}


void arrque_traverse_clear(struct array_queue *q, void (*process)(void *))
{
	int   i = 0, current_size = 0;
	void *element = NULL;
	pthread_mutex_lock(&q->lock);
	current_size = q->size;
	for (i = 0; i < current_size; i++) {
		element = q->data[q->head];
		q->head = (q->head + 1) % q->capacity;
		q->size--;
		process(element);
	}
	q->head = 0;
	q->tail = -1;
	pr("q-clear status %d %d %d %d %d %d\n\n", q->capacity, q->size, arrque_is_empty(q),
	   arrque_is_full(q), q->head, q->tail);
	pthread_mutex_unlock(&q->lock);
}

// 遍历队列并使用参数函数处理元素
void arrque_traverse(struct array_queue *q, void (*process)(void *))
{
	int i = 0, current_size = 0, index = 0;
	pthread_mutex_lock(&q->lock);
	current_size = q->size;
	for (i = 0; i < current_size; i++) {
		index = (q->head + i) % q->capacity;
		process(q->data[index]);
	}
	pthread_mutex_unlock(&q->lock);
	pr("==>>> %d %d %d %d %d %d\n\n", q->capacity, q->size, arrque_is_empty(q), arrque_is_full(q),
	   q->head, q->tail);
}

// 打印元素值的处理函数
void print_element(void *element)
{
	pr("%d ", *(int *)element);
}

// 对头入队
int arrque_en_head(struct array_queue *q, void *element)
{
	int ret = 0;
	pthread_mutex_lock(&q->lock);
	if (!(q->size == q->capacity)) {
		q->head = (q->head + q->capacity - 1) % q->capacity;
		q->size++;
		q->data[q->head] = element;
	} else {
		ret = -1;
	}
	pthread_mutex_unlock(&q->lock);
	pr("q-in-h status %d %d %d %d %d %d\n\n", q->capacity, q->size, arrque_is_empty(q), arrque_is_full(q),
	   q->head, q->tail);
	return ret;
}

// 对尾入队
int arrque_en_tail(struct array_queue *q, void *element)
{
	int ret = 0;
	pthread_mutex_lock(&q->lock);
	if (!(q->size == q->capacity)) {
		q->tail = (q->tail + 1) % q->capacity;
		q->data[q->tail] = element;
		q->size++;
	} else {
		ret = -1;
	}
	pthread_mutex_unlock(&q->lock);
	pr("q-in-t status %d %d %d %d %d %d\n\n", q->capacity, q->size, arrque_is_empty(q), arrque_is_full(q),
	   q->head, q->tail);
	return ret;
}

// 对头出队
void *arrque_de_head(struct array_queue *q)
{
	void *element = NULL;
	pthread_mutex_lock(&q->lock);
	if (!!q->size) {
		element = q->data[q->head];
		q->head = (q->head + 1) % q->capacity;
		q->size--;
	}
	pthread_mutex_unlock(&q->lock);
	pr("q-out-h status %d %d %d %d %d %d, %d\n", q->capacity, q->size, arrque_is_empty(q),
	   arrque_is_full(q), q->head, q->tail, element ? *(int *)element : -1);
	return element;
}

// 队尾出队
void *arrque_de_tail(struct array_queue *q)
{
	void *element = NULL;
	pthread_mutex_lock(&q->lock);
	if (!!q->size) {
		element = q->data[q->tail];
		q->tail = (q->tail - 1 + q->capacity) % q->capacity;
		q->size--;
	}
	pthread_mutex_unlock(&q->lock);
	pr("q-out-t status %d %d %d %d %d %d, %d\n", q->capacity, q->size, arrque_is_empty(q),
	   arrque_is_full(q), q->head, q->tail, element ? *(int *)element : -1);
	return element;
}

// 队头peek
void *arrque_peek_head(struct array_queue *q)
{
	void *result = NULL;
	pthread_mutex_lock(&q->lock);
	if (!!q->size) {
		result = q->data[q->head];
	}
	pthread_mutex_unlock(&q->lock);
	return result;
}

// 队尾peek
void *arrque_peek_tail(struct array_queue *q)
{
	void *result = NULL;
	pthread_mutex_lock(&q->lock);
	if (!!q->size) {
		result = q->data[q->tail];
	}
	pthread_mutex_unlock(&q->lock);
	return result;
}

// 队尾入队, 若队列已满, 则出队头元素再入队
void *arrque_en_tail_pop(struct array_queue *q, void *element)
{
	void *popped_element = NULL;
	pthread_mutex_lock(&q->lock);
	if (q->size == q->capacity) {
		popped_element = q->data[q->head];
		q->head = (q->head + 1) % q->capacity;
	} else {
		q->size++;
	}
	q->tail = (q->tail + 1) % q->capacity;
	q->data[q->tail] = element;
	pthread_mutex_unlock(&q->lock);
	return popped_element;
}

// 队头入队, 若队列已满, 则出队尾元素再入队
void *arrque_en_head_pop(struct array_queue *q, void *element)
{
	void *removed_element = NULL;
	pthread_mutex_lock(&q->lock);
	if (q->size == q->capacity) {
		removed_element = q->data[q->tail];
		q->tail = (q->tail - 1 + q->capacity) % q->capacity;
	} else {
		q->size++;
	}
	q->head = (q->head - 1 + q->capacity) % q->capacity;
	q->data[q->head] = element;
	pthread_mutex_unlock(&q->lock);
	return removed_element;
}

// 从队列1头移到队列2尾
int arrque_move_head_to_tail(struct array_queue *q1, struct array_queue *q2)
{
	int ret = -1;
	pthread_mutex_lock(&q1->lock);
	pthread_mutex_lock(&q2->lock);
	if (!!q1->size && !(q2->size == q2->capacity)) {
		q2->tail = (q2->tail + 1) % q2->capacity;
		q2->size++;
		q2->data[q2->tail] = q1->data[q1->head];
		q1->head = (q1->head + 1) % q1->capacity;
		q1->size--;
		ret = 0;
	}
	pthread_mutex_unlock(&q2->lock);
	pthread_mutex_unlock(&q1->lock);
	return ret;
}

// 从队列1尾移到队列2头
int arrque_mov_tail_to_head(struct array_queue *q1, struct array_queue *q2)
{
	int ret = -1;
	pthread_mutex_lock(&q1->lock);
	pthread_mutex_lock(&q2->lock);
	if (!!q1->size && !(q2->size == q2->capacity)) {
		q2->head = (q2->head + q2->capacity - 1) % q2->capacity;
		q2->size++;
		q2->data[q2->head] = q1->data[q1->tail];
		q1->tail = (q1->tail - 1 + q1->capacity) % q1->capacity;
		q1->size--;
		ret = 0;
	}
	pthread_mutex_unlock(&q2->lock);
	pthread_mutex_unlock(&q1->lock);
	return ret;
}

// 查找队列中指定元素的位置，存在则返回index，不存在则返回-1
int arrque_search(struct array_queue *q, void *element)
{
	int index = -1, i = 0, count = 0;
	pthread_mutex_lock(&q->lock);
	i = q->head;
	while (count < q->size) {
		if (q->data[i] == element) {
			index = (i - q->head + q->capacity) % q->capacity;
			break;
		}
		i = (i + 1) % q->capacity;
		count++;
	}
	pthread_mutex_unlock(&q->lock);
	return index;
}

// 从队列中删除指定元素，成功返回1，失败返回0
int arrque_remove(struct array_queue *q, void *element)
{
	int i = 0, j = 0, index = 0;
	pthread_mutex_lock(&q->lock);
	for (i = 0; i < q->size; i++) {
		index = (q->head + i) % q->capacity;
		if (q->data[index] == element) {
			for (j = i; j < q->size - 1; j++) {
				q->data[(q->head + j) % q->capacity] =
				    q->data[(q->head + j + 1) % q->capacity];
			}
			q->tail = (q->tail - 1 + q->capacity) % q->capacity;
			q->size--;
			pthread_mutex_unlock(&q->lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&q->lock);
	return -1;
}


int main(int argc, char const *argv[])
{
	int  i = 0, cnt = 8, tmp = 0;
	int *data = NULL, *data2 = NULL, *data3 = NULL, *hdat = NULL, *tdat = NULL, *mdat = NULL;
	struct array_queue q1, q2;

	arrque_init(&q1, cnt);
	arrque_init(&q2, cnt);

	pr("\n入队测试-----------------------\n");
	for (i = 0; i < cnt - 4; i++) {
		data = malloc(sizeof(int));
		*data = i + 10;
		arrque_en_head(&q1, data);
	}
	arrque_traverse(&q1, print_element);
	// arrque_traverse_clear(&q1, free);

	for (i = 0; i < cnt - 4; i++) {
		data = malloc(sizeof(int));
		*data = i + 30;
		arrque_en_tail(&q2, data);
	}
	arrque_traverse(&q2, print_element);
	// arrque_traverse_clear(&q1, free);

	pr("\n队头移队尾测试-----------------------\n");
	arrque_traverse(&q1, print_element);
	arrque_traverse(&q2, print_element);
	pr("\n");
	for (i = 0; i < cnt + 20; i++) {
		if (arrque_move_head_to_tail(&q2, &q1))
			arrque_mov_tail_to_head(&q1, &q2);
		arrque_traverse(&q1, print_element);
		arrque_traverse(&q2, print_element);
		pr("\n");
	}

	arrque_traverse_clear(&q1, free);
	arrque_deinit(&q1);
	arrque_traverse_clear(&q2, free);
	arrque_deinit(&q2);
	return 0;
}
