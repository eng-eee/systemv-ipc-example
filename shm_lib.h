#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

constexpr int BLOCK_SIZE = 1024;

// Error codes
constexpr int SUCCESS = 0;
constexpr int SHM_ERROR = -1;
constexpr int WRONG_INPUT_ARG = -999;
constexpr int FILE_PARSE_ERROR = -998;
constexpr int SHM_FAILED = -996;
constexpr int MSGQ_FAILED = -995;

// Message types
constexpr long MSG_DATA_READY = 1;
constexpr long MSG_DATA_READY_ACK = 2;

struct IpcMessage
{
    long mtype;
    char mtext[32];
};

// shared memory
key_t create_shm_key(const char *path, int id);

int create_shared_memory(key_t key);

int open_shared_memory(key_t key);

char *attach_shared_memory(int shmid);

int detach_shared_memory(const void *addr);

int destroy_shared_memory(int shmid);

// message queue
int create_message_queue(key_t key);

int open_message_queue(key_t key);

int send_message(int msgid, long message_type, const char *message);

int receive_message(int msgid, long message_type, char *buffer, size_t buffer_size);

int destroy_message_queue(int msgid);