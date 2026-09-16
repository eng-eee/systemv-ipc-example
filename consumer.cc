#include <cstdio>
#include <iostream>
#include <string>

#include "ini_parser.h"
#include "shm_lib.h"


int main(int argc, char *argv[]) {
  if (argc > 1) {
    printf("Usage: %s <Do not provide any arguments>\n", argv[0]);
    return WRONG_INPUT_ARG;
  }

  // Parse configuration
  IniParser parser("config.ini");

  int parser_result = parser.ParseIni();

  if (parser_result != INI_SUCCESS) {
    fprintf(stderr, "Failed to parse configuration file! Error: %d\n",
            parser_result);
    return FILE_PARSE_ERROR;
  }

  const std::string key_id_str = parser.GetValue("key");

  if (key_id_str.empty()) {
    fprintf(stderr, "Key not found in configuration file\n");
    return FILE_PARSE_ERROR;
  }

  int key_id;

  try {
    key_id = std::stoi(key_id_str);
  } catch (const std::exception &e) {
    fprintf(stderr, "Invalid key: %s\n", e.what());
    return FILE_PARSE_ERROR;
  }

  const std::string shm_file = parser.GetValue("shm_file");

  if (shm_file.empty()) {
    fprintf(stderr, "Shared memory file not found in configuration file\n");
    return FILE_PARSE_ERROR;
  }

  // Same keys used by producer
  key_t shm_key = create_shm_key(shm_file.c_str(), key_id);

  if (shm_key == static_cast<key_t>(SHM_ERROR)) {
    fprintf(stderr, "Failed to create shared memory key\n");
    return SHM_FAILED;
  }

  key_t msg_key = create_shm_key(shm_file.c_str(), key_id + 1);

  if (msg_key == static_cast<key_t>(SHM_ERROR)) {
    fprintf(stderr, "Failed to create message queue key\n");
    return SHM_FAILED;
  }


  // Open existing shared memory -> The consumer does not create the shared memory.
  int shmid = open_shared_memory(shm_key);

  if (shmid == SHM_ERROR) {
    fprintf(stderr, "Failed to open shared memory\n");
    return SHM_FAILED;
  }

  // Open existing message queue -> The consumer does not create the message queue.
  int msgid = open_message_queue(msg_key);

  if (msgid == SHM_ERROR) {
    fprintf(stderr, "Failed to open message queue\n");
    return MSGQ_FAILED;
  }


  char notification[32];

  std::cout << "Waiting for DATA_READY..." << std::endl;
  // Wait for producer notification --> msgrcv() blocks here until the producer sends:
  if (receive_message(msgid, MSG_DATA_READY, notification,
                      sizeof(notification)) == SHM_ERROR) {

    fprintf(stderr, "Failed to receive DATA_READY\n");
    return MSGQ_FAILED;
  }

  std::cout << "Received notification: " << notification << std::endl;

  // Attach shared memory
  char *shm_addr = attach_shared_memory(shmid);

  if (shm_addr == nullptr) {
    fprintf(stderr, "Failed to attach shared memory\n");
    return SHM_FAILED;
  }

  // Read message
  std::cout << "Reading message from shared memory: " << shm_addr << std::endl;

  // Cleanup: Detach shared memory
  if (detach_shared_memory(shm_addr) == SHM_ERROR) {
    fprintf(stderr, "Failed to detach shared memory\n");
    return SHM_FAILED;
  }



  std::cout << "Sending DATA_READ acknowledgment..." << std::endl;
  // Notify producer that the message was consumed
  if (send_message(msgid, MSG_DATA_READY_ACK, "DATA_READ") == SHM_ERROR) {

    fprintf(stderr, "Failed to send DATA_READ acknowledgment\n");
    return MSGQ_FAILED;
  }

  std::cout << "Consumer finished successfully." << std::endl;

  return SUCCESS;
}