#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_ORDERS 100
#define BUFFER_SIZE 10
#define PRODUCERS 3
#define CONSUMERS 3
#define ITEM_TYPES 3

sem_t semEmptyNormal;
sem_t semFullNormal;
sem_t semEmptyPriority;
sem_t semFullPriority;

pthread_mutex_t mutexNormalBuffer;
pthread_mutex_t mutexPriorityBuffer;
pthread_mutex_t orderLock = PTHREAD_MUTEX_INITIALIZER;

struct Buffer
{
    int buffer[BUFFER_SIZE];
    int front;
    int rear;
    int count;
};

struct Buffer bufferNormal;
struct Buffer bufferPriority;

enum ItemType
{
    CHAIR = 0,
    SOFA = 1,
    BED = 2
};
const char *itemNames[] = {"Chair", "Sofa", "Bed"};

struct Order
{
    int order_id;
    int quantity[3];
    bool isPriority;
    char creation_time[64];
    char completion_time[64];
};

struct Order orderList[MAX_ORDERS];
int totalOrders = 0, fin = 0;
int orderProgress[MAX_ORDERS][ITEM_TYPES] = {0};
bool orderCompleted[MAX_ORDERS] = {false};
bool allOrdersCompleted = false;

void timeoutHandler(int sig)
{
    printf("\nShop is closing! Finishing up remaining orders.\n");
    allOrdersCompleted = true;
    printf("\a");
    fflush(stdout);
}

void LogOrder(int order_id, int chair, int sofa, int bed, const char *creation_time, const char *completion_time)
{
    FILE *logFile = fopen("logs.txt", "a");
    if (!logFile)
    {
        perror("Error opening logs.txt");
        return;
    }
    if (!creation_time) creation_time = "NULL";  
    if  (!completion_time) completion_time = "NULL"; 
    fprintf(logFile, "OrderNumber=%d Chair=%d Sofa=%d Bed=%d CreationTime=%s CompletionTime=%s\n",
            order_id, chair, sofa, bed, creation_time, completion_time);
    fclose(logFile);
}

void ManualOrderEntry(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f)
    {
        perror("orders file");
        exit(EXIT_FAILURE);
    }

    int n;
    printf("Enter Number of orders: ");
    while (scanf("%d", &n) != 1 || n <= 0)
    {
        printf("Invalid input.Please Enter a positive integer: \n");
        printf("\a");
        while (getchar() != '\n')
            ;
    }

    for (int i = 0; i < n; ++i)
    {
        int id, c, s, b;
        while (1)
        {
            printf("\nOrder #%d id: ", i + 1);
            if (scanf("%d", &id) != 1 || id <= 0)
            {
                printf("Invalid Input. Enter a positive integer:\n");
                while (getchar() != '\n')
                    ;
                continue;
            }
            int duplicate = 0;
            for (int j = 0; j < totalOrders; j++)
            {
                if (orderList[j].order_id == id)
                {
                    duplicate = 1;
                    break;
                }
            }

            if (duplicate)
            {
                printf("Duplicate ID.Please enter a unique ID:\n");
                continue;
            }
            break;
        }

        printf("  Chairs: ");
        while (scanf("%d", &c) != 1 || c < 0)
        {
            printf("Invalid Input. Enter a positive integer:\n");
            while (getchar() != '\n')
                ;
        }

        printf("  Sofas : ");
        while (scanf("%d", &s) != 1 || s < 0)
        {
            printf("Invalid Input. Enter a positive integer:\n");
            while (getchar() != '\n')
                ;
        }

        printf("  Beds  : ");
        while (scanf("%d", &b) != 1 || b < 0)
        {
            printf("Invalid Input. Enter a positive integer:\n");
            while (getchar() != '\n')
                ;
        }

        time_t now = time(NULL);
        strftime(orderList[totalOrders].creation_time, sizeof(orderList[totalOrders].creation_time), "%Y-%m-%d %H:%M:%S", localtime(&now));

        strcpy(orderList[totalOrders].completion_time, "");

        fprintf(f, "Order %d: Chair=%d, Sofa=%d, Bed=%d\n", id, c, s, b);
        orderList[totalOrders].order_id = id;
orderList[totalOrders].quantity[0] = c;
orderList[totalOrders].quantity[1] = s;
orderList[totalOrders].quantity[2] = b;
orderList[totalOrders].isPriority = (c + s + b <= 10);
orderCompleted[totalOrders] = false;
orderProgress[totalOrders][0] = orderProgress[totalOrders][1] = orderProgress[totalOrders][2] = 0;
totalOrders++;

    }
    fclose(f);
    fin=totalOrders;
    puts("\nOrders saved.\n");
}

void InitializeBuffers()
{
    bufferNormal.front = bufferNormal.rear = -1;
    bufferNormal.count = 0;
    bufferPriority.front = bufferPriority.rear = -1;
    bufferPriority.count = 0;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        bufferNormal.buffer[i] = -1;
        bufferPriority.buffer[i] = -1;
    }
}

void printQueue(struct Buffer q, const char *label)
{
    printf("%s: [", label);
    int idx = (q.front + 1) % BUFFER_SIZE;
    for (int c = 0; c < q.count; c++)
    {
        printf("%d", q.buffer[idx]);
        idx = (idx + 1) % BUFFER_SIZE;
        if (c < q.count - 1)
            printf(", ");
    }
    printf("]\n\n");
}

void LoadOrdersFromFile(const char *filename)
{
    totalOrders = 0; /* reset before reload */
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        return;
    }
    while (fscanf(file,
                  "Order %d: Chair=%d, Sofa=%d, Bed=%d\n",
                  &orderList[totalOrders].order_id,
                  &orderList[totalOrders].quantity[0],
                  &orderList[totalOrders].quantity[1],
                  &orderList[totalOrders].quantity[2]) == 4)
    {
        int total_items = orderList[totalOrders].quantity[0] +
                          orderList[totalOrders].quantity[1] +
                          orderList[totalOrders].quantity[2];
        orderList[totalOrders].isPriority = (total_items <= 10);
        orderCompleted[totalOrders] = false;
        orderProgress[totalOrders][0] = orderProgress[totalOrders][1] =
            orderProgress[totalOrders][2] = 0;
        totalOrders++;
    }
    fin = totalOrders;
    fclose(file);
    printf("%d orders successfully loaded from file.\n", totalOrders);
}

void printOrders(void)
{
    if (totalOrders == 0)
    {
        printf("---------------------\n");
        puts("Alert No orders loaded.");
        printf("---------------------\n");
        printf("\a");
        return;
    }
    for (int i = 0; i < totalOrders; ++i)
        printf("Order %d -> Chair:%d Sofa:%d Bed:%d%s\n",
               orderList[i].order_id,
               orderList[i].quantity[0],
               orderList[i].quantity[1],
               orderList[i].quantity[2],
               orderCompleted[i] ? " [DONE]" : "");
}

void *Producer(void *arg)
{
    int type = *((int *)arg);

    for (int i = 0; i < totalOrders && !allOrdersCompleted; i++)
    {
        if (allOrdersCompleted)
            break;

        int itemEncoded = i * 10 + type;
        struct Order ord = orderList[i];
        int qty = ord.quantity[type];

        for (int j = 0; j < qty; j++)
        {
            if (ord.isPriority)
            {
                int sem_val;
                sem_getvalue(&semEmptyPriority, &sem_val);
                if (sem_val == 0)
                {
                    printf("--------------------------------------------------\n");
                    printf("Alert Priority buffer is full. Producer is waiting.\n");
                    printf("--------------------------------------------------\n");
                    printf("\a");
                }

                if (sem_wait(&semEmptyPriority) != 0)
                {
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("ERROR: sem_wait failed! Priority production paused.\n");
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("\a\a\a");
                    pthread_exit(NULL);
                }

                if (pthread_mutex_lock(&mutexPriorityBuffer) != 0)
                {
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("ERROR: Mutex lock failed! Priority production paused.\n");
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("\a\a\a");
                    pthread_exit(NULL);
                }

                bufferPriority.rear = (bufferPriority.rear + 1) % BUFFER_SIZE;
                bufferPriority.buffer[bufferPriority.rear] = itemEncoded;
                bufferPriority.count++;
                printf("[Producer-%s] Pushed PRIORITY %s (Order %d)\n",
                       itemNames[type], itemNames[type], ord.order_id);
                printQueue(bufferPriority, "Priority Queue");

                pthread_mutex_unlock(&mutexPriorityBuffer);
                sem_post(&semFullPriority);
            }
            else
            {
                int sem_val;
                sem_getvalue(&semEmptyNormal, &sem_val);
                if (sem_val == 0)
                {
                    printf("------------------------------------------------\n");
                    printf("Alert Normal buffer is full. Producer is waiting.\n");
                    printf("-------------------------------------------------\n");
                    printf("\a");
                }

                if (sem_wait(&semEmptyNormal) != 0)
                {
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("ERROR: sem_wait failed! Normal production paused.\n");
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("\a\a\a");
                    pthread_exit(NULL);
                }

                if (pthread_mutex_lock(&mutexNormalBuffer) != 0)
                {
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("ERROR: Mutex lock failed! Normal production paused.\n");
                    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    printf("\a\a\a");
                    pthread_exit(NULL);
                }

                bufferNormal.rear = (bufferNormal.rear + 1) % BUFFER_SIZE;
                bufferNormal.buffer[bufferNormal.rear] = itemEncoded;
                bufferNormal.count++;
                printf("[Producer-%s] Pushed NORMAL %s (Order %d)\n",
                       itemNames[type], itemNames[type], ord.order_id);
                printQueue(bufferNormal, "Normal Queue");

                pthread_mutex_unlock(&mutexNormalBuffer);
                sem_post(&semFullNormal);
            }

            sleep(1);
        }
    }

    return NULL;
}


void *Consumer(void *args)
{
    while (1)
    {
        int item;
        if (sem_trywait(&semFullPriority) == 0)
        {
            pthread_mutex_lock(&mutexPriorityBuffer);
            bufferPriority.front = (bufferPriority.front + 1) % BUFFER_SIZE;
            item = bufferPriority.buffer[bufferPriority.front];
            bufferPriority.count--;
            printf("Consumed PRIORITY item %d\n", item);
            printQueue(bufferPriority, "Priority Queue");
            pthread_mutex_unlock(&mutexPriorityBuffer);
            sem_post(&semEmptyPriority);
        }
        else if (sem_trywait(&semFullNormal) == 0)
        {
            pthread_mutex_lock(&mutexNormalBuffer);
            bufferNormal.front = (bufferNormal.front + 1) % BUFFER_SIZE;
            item = bufferNormal.buffer[bufferNormal.front];
            bufferNormal.count--;
            printf("Consumed NORMAL item %d\n", item);
            printQueue(bufferNormal, "Normal Queue");
            pthread_mutex_unlock(&mutexNormalBuffer);
            sem_post(&semEmptyNormal);
        }
        else
        {
            if (allOrdersCompleted)
                break;
            printf("----------------------------------------------------\n");
            printf("Alert No item available to consume.Consumer waiting.\n");
            printf("----------------------------------------------------\n");
            printf("\a");
            sleep(1);
            continue;
        }

        int orderIndex = item / 10;
        int itemType = item % 10;

        pthread_mutex_lock(&orderLock);

        if (orderIndex < totalOrders && !orderCompleted[orderIndex])
        {
            orderProgress[orderIndex][itemType]++;
            if (orderProgress[orderIndex][0] >= orderList[orderIndex].quantity[0] &&
                orderProgress[orderIndex][1] >= orderList[orderIndex].quantity[1] &&
                orderProgress[orderIndex][2] >= orderList[orderIndex].quantity[2])
            {
                orderCompleted[orderIndex] = true;

                printf("-----------------------------------------\n");
                printf("\nOrder %d is COMPLETE (Chair, Sofa, Bed)\n\n", orderList[orderIndex].order_id);
                printf("-----------------------------------------\n");
                time_t now = time(NULL);
                strftime(orderList[orderIndex].completion_time, sizeof(orderList[orderIndex].completion_time), "%Y-%m-%d %H:%M:%S", localtime(&now));

                
                LogOrder(orderList[orderIndex].order_id,
                         orderList[orderIndex].quantity[0],
                         orderList[orderIndex].quantity[1],
                         orderList[orderIndex].quantity[2],
                         orderList[orderIndex].creation_time,
                         orderList[orderIndex].completion_time);
            }
        }
        pthread_mutex_unlock(&orderLock);

        sleep(rand() % 3 + 1);
    }
    return NULL;
}


void ViewLogs()
{
    FILE *logFile = fopen("logs.txt", "r");
    if (!logFile)
    {
        perror("Error opening logs.txt");
        printf("\a");
        return;
    }

    char line[256];
    int orderId, chair, sofa, bed;
    char creationDate[11], creationTime[9], completionDate[11], completionTime[9];

    printf("\n--------------------------------------------------------------------------------------------------\n");
    printf("| %-10s | %-6s | %-6s | %-6s | %-11s | %-12s | %-15s | %-15s |\n",
           "OrderID", "Chair", "Sofa", "Bed", "Date", "CreationTime", "CompletionDate", "CompletionTime");
    printf("--------------------------------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), logFile))
    {
        if (sscanf(line,
                   "OrderNumber=%d Chair=%d Sofa=%d Bed=%d CreationTime=%10s %8s CompletionTime=%10s %8s",
                   &orderId, &chair, &sofa, &bed,
                   creationDate, creationTime,
                   completionDate, completionTime) == 8)
        {
            printf("| %-10d | %-6d | %-6d | %-6d | %-11s | %-12s | %-15s | %-15s |\n",
                   orderId, chair, sofa, bed,
                   creationDate, creationTime,
                   completionDate, completionTime);
        }
    }

    printf("--------------------------------------------------------------------------------------------------\n");
    fclose(logFile);
}

void CheckStatus()
{
    printf("\n======= ORDER STATUS =======\n");
    for (int i = 0; i < fin; i++)
    {
        if (totalOrders == 0)
        {
            printf("Order %d is Completed.\n", orderList[i].order_id);
        }
        else if (orderProgress[i][0] == 0 && orderProgress[i][1] == 0 && orderProgress[i][2] == 0)
        {
            printf("Order %d production not started yet\n", orderList[i].order_id);
        }
    }
}

int main()
{
    const char *fname = "orders.txt";

   
    InitializeBuffers();
    pthread_mutex_init(&mutexNormalBuffer, NULL);
    pthread_mutex_init(&mutexPriorityBuffer, NULL);
    sem_init(&semEmptyNormal, 0, BUFFER_SIZE);
    sem_init(&semFullNormal, 0, 0);
    sem_init(&semEmptyPriority, 0, BUFFER_SIZE);
    sem_init(&semFullPriority, 0, 0);
   
    pthread_t producers[PRODUCERS];
    pthread_t consumers[CONSUMERS];
    int types[PRODUCERS] = {CHAIR, SOFA, BED};
    bool processingStarted = false;

    while (1)
    {
        printf("\n");
        printf("***************************\n");
        printf("---------------------------\n");
        printf("WELCOME TO TUMZ WARE HOUSE \n");
        printf("---------------------------\n");
        printf("***************************\n");
        printf("\n");
        printf("\n===== MENU =====\n"
               "1. Create new orders.\n"
               "2. Start processing orders.\n"
               "3. View current orders.\n"
               "4. ViewLogs\n"
               "5. Check Order Status.\n"
               "6. Exit\n"
               "Select the option: ");
        int choice;
        if (scanf("%d", &choice) != 1 || choice <= 0)
        {
            printf("Invalid Input. Please enter number from (1 to 6).\n");
            while (getchar() != '\n')
                ;
            continue;
        }

        switch (choice)
        {
        case 1:
            ManualOrderEntry(fname);
            allOrdersCompleted = false;
   

            break;

        case 3:
   
            printOrders();
            break;

        case 2:
            if (processingStarted)
            {
                puts("Processing already started.");
                break;
            }
            if (totalOrders == 0)
            {
                puts("No orders loaded. Use option 1 first.");
                break;
            }

            if (allOrdersCompleted)
            {
                puts("All orders are completed. Use option 1 first.");
                break;
            }

            /* spawn threads */
            for (int i = 0; i < PRODUCERS; i++)
                pthread_create(&producers[i], NULL, Producer, &types[i]);
            for (int i = 0; i < CONSUMERS; i++)
                pthread_create(&consumers[i], NULL, Consumer, NULL);
            processingStarted = true;

            /* wait for producers */
            for (int i = 0; i < PRODUCERS; i++)
                pthread_join(producers[i], NULL);
            allOrdersCompleted = true;
            /* wait for consumers */
            for (int i = 0; i < CONSUMERS; i++)
                pthread_join(consumers[i], NULL);

            puts("\nAll orders processed.");
            processingStarted = false;
            allOrdersCompleted = true;

            totalOrders = 0;
            break;
        case 4:
            printf("---------------------------\n");
            printf("         LOGS \n");
            printf("---------------------------\n");
            ViewLogs();
            break;

        case 5:
            CheckStatus();
            break;

        case 6:
            puts("Exiting.");

            allOrdersCompleted = true;
            return 0;

        default:
            puts("Invalid choice.");
        }
    }
}
