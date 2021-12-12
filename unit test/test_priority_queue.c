#include <stdio.h>
#include <stdlib.h>

struct node
{
    int id;
    int priority;
    struct node *next;
};
typedef struct node node;
typedef struct node * node_ptr;

node_ptr head = NULL;

void enq(node_ptr *new_node, node_ptr *head)
{
    node_ptr temp = (*head);
    node_ptr temp_ex = NULL;
    if(temp!=NULL)
    {
        while(temp != NULL)
        {
            if(temp->priority >= (*new_node)->priority)
            {
                temp_ex = temp;
                temp = temp->next;
            }
            else
            {
                (*new_node)->next = temp;
                if(temp != (*head))
                {
                    temp_ex->next = (*new_node);
                }
                else
                {
                    (*head) = (*new_node);
                }
                break;
            }
        }
        if(temp == NULL)
            temp_ex->next = (*new_node);
    }
    else//empty queue
    {
        (*head) = (*new_node);
        (*new_node)->next = NULL;
    }
    return;
}

void deq(node_ptr *head)
{
    if((*head) == NULL)
        return;
    else
    {
        node_ptr leave = (*head);
        (*head) = (*head)->next;
        leave->next = NULL;
        free(leave);
        return;
    }
}

void printq(node_ptr *head)
{
    node_ptr temp = (*head);
    while(temp != NULL)
    {
        printf("%d %d\n", temp->id, temp->priority);
        temp = temp->next;
    }
    printf("\n");
    return;
}

void create(int id, int priority)
{
    node_ptr new = malloc(sizeof(node));
    new->id = id;
    new->priority = priority;
    enq(&new, &head);
    printq(&head);
    return;
}

int main()
{
    create(1,1);
    create(2,0);
    create(3,2);
    create(4,1);
    create(5,2);
    create(6,0);
    deq(&head);
    printq(&head);
    return 0;
}