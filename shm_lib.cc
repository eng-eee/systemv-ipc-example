#include "shm_lib.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>


key_t create_shm_key(const char *path, int id)
{
    key_t key = ftok(path, id);

    if (key == static_cast<key_t>(-1)) {
        perror("ftok failed");
        return static_cast<key_t>(SHM_ERROR);
    }

    std::cout << "Generated key: " << key << std::endl;

    return key;
}


int create_shared_memory(key_t key)
{
    int shmid = shmget(
        key,
        BLOCK_SIZE,
        0600 | IPC_CREAT | IPC_EXCL
    );

    if (shmid == SHM_ERROR) {
        perror("shmget(create) failed");
        return SHM_ERROR;
    }

    std::cout << "Created shared memory segment with ID: "
              << shmid << std::endl;

    return shmid;
}


int open_shared_memory(key_t key)
{
    int shmid = shmget(
        key,
        BLOCK_SIZE,
        0600
    );

    if (shmid == SHM_ERROR) {
        perror("shmget(open) failed");
        return SHM_ERROR;
    }

    std::cout << "Opened shared memory segment with ID: "
              << shmid << std::endl;

    return shmid;
}


char *attach_shared_memory(int shmid)
{
    void *addr = shmat(shmid, nullptr, 0);

    if (addr == reinterpret_cast<void *>(-1)) {
        perror("shmat failed");
        return nullptr;
    }

    return static_cast<char *>(addr);
}


int detach_shared_memory(const void *addr)
{
    if (shmdt(addr) == SHM_ERROR) {
        perror("shmdt failed");
        return SHM_ERROR;
    }

    return SUCCESS;
}


int destroy_shared_memory(int shmid)
{
    if (shmctl(shmid, IPC_RMID, nullptr) == SHM_ERROR) {
        perror("shmctl(IPC_RMID) failed");
        return SHM_ERROR;
    }

    return SUCCESS;
}


// ============================================================
// Message Queue
// ============================================================

int create_message_queue(key_t key)
{

    // IPC_CREAT -> create if it doesn't exist
    // IPC_EXCL  -> fail if it already exists
    int msgid = msgget(key,0600 | IPC_CREAT | IPC_EXCL);

    if (msgid == SHM_ERROR) {
        perror("msgget(create) failed");
        return SHM_ERROR;
    }

    std::cout << "Created message queue with ID: " << msgid << std::endl;

    return msgid;
}


int open_message_queue(key_t key)
{
     //Consumer must not create the queue.
    int msgid = msgget(key,0600);

    if (msgid == SHM_ERROR) {
        perror("msgget(open) failed");
        return SHM_ERROR;
    }

    std::cout << "Opened message queue with ID: " << msgid << std::endl;

    return msgid;
}


int send_message(int msgid, long message_type, const char *message)
{
    IpcMessage msg{};

    msg.mtype = message_type;

    std::strncpy(msg.mtext, message, sizeof(msg.mtext) - 1);

    msg.mtext[sizeof(msg.mtext) - 1] = '\0';

    if (msgsnd(msgid,&msg,sizeof(msg.mtext),0) == SHM_ERROR) {
        perror("msgsnd failed");
        return SHM_ERROR;
    }

    return SUCCESS;
}


int receive_message(int msgid, long message_type, char *buffer, size_t buffer_size)
{
    if (buffer == nullptr || buffer_size == 0) {
        errno = EINVAL;
        return SHM_ERROR;
    }

    IpcMessage msg{};

    ssize_t received = msgrcv(msgid,&msg,sizeof(msg.mtext),message_type,0);

    if (received == -1) {
        perror("msgrcv failed");
        return SHM_ERROR;
    }

    size_t copy_size = static_cast<size_t>(received);

    if (copy_size >= buffer_size) {
        copy_size = buffer_size - 1;
    }

    std::memcpy(buffer,msg.mtext,copy_size);

    buffer[copy_size] = '\0';
    return SUCCESS;
}


int destroy_message_queue(int msgid)
{
    if (msgctl(msgid, IPC_RMID, nullptr) == SHM_ERROR) {
        perror("msgctl(IPC_RMID) failed");
        return SHM_ERROR;
    }

    return SUCCESS;
}