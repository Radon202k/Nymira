#pragma once

function void
stone_queue_free(StoneQueue *queue)
{
    free(queue->storage);
    memset(queue, 0, sizeof(StoneQueue));
}

function StoneQueue
stone_queue_new(u32 size)
{
    StoneQueue result = {0};
    result.storageSize = size;
    result.storage = xnalloc(size, Stone);
    result.rear = -1;
    result.front = -1;
    return result;
}

function bool
stone_queue_push(StoneQueue *queue,
                 Stone stone)
{
    // Check for overflow condition
    if (queue->rear == queue->storageSize-1)
    {
        return false;
    }
    // Check if queue is empty
    else if (queue->front == -1 && queue->rear == -1)
    {
        queue->rear = 0;
        queue->front = 0;
    }
    // 
    else
    {
        queue->rear = queue->rear + 1;
    }
    
    queue->storage[queue->rear] = stone;
    return true;
}

function bool
stone_queue_pop(StoneQueue *queue, Stone *result)
{
    // Check for underflow condition
    if (queue->front == -1 || queue->front > queue->rear)
    {
        return false;
    }
    else
    {
        *result = queue->storage[queue->front];
        queue->front = queue->front + 1;
        if (queue->front > queue->rear)
        {
            queue->front = -1;
            queue->rear = -1;
        }
        return true;
    }
}
