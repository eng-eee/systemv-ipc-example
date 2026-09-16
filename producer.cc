#include <cstdio>
#include <iostream>
#include <string>

#include "ini_parser.h"
#include "shm_lib.h"

// Expected number of command-line arguments [one for the program name and one for the message]
constexpr int EXPECTED_INPUT_NUM_ARG = 2;

int main(int argc, char *argv[]) {
  if (argc != EXPECTED_INPUT_NUM_ARG) {
    printf("Usage: %s <message>\n", argv[0]);
    return WRONG_INPUT_ARG;
  }

  const char *message = argv[1];

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

  int key_id = 0;

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

  // key_id      -> shared memory
  // key_id + 1  -> message queue
  key_t shm_key = create_shm_key(shm_file.c_str(), key_id);

  if (shm_key == static_cast<key_t>(SHM_ERROR)) {
    fprintf(stderr, "Failed to create shared memory key\n");
    return SHM_FAILED;
  }

  key_t msg_key = create_shm_key(shm_file.c_str(), key_id + 1);

  if (msg_key == static_cast<key_t>(SHM_ERROR)) {
    fprintf(stderr, "Failed to create message queue key\n");
    return MSGQ_FAILED;
  }

  // Create shared memory
  int shmid = create_shared_memory(shm_key);

  if (shmid == SHM_ERROR) {
    fprintf(stderr, "Failed to create shared memory\n");
    return SHM_FAILED;
  }

  // Create message queue
  int msgid = create_message_queue(msg_key);

  if (msgid == SHM_ERROR) {
    fprintf(stderr, "Failed to create message queue\n");
    destroy_shared_memory(shmid);
    return MSGQ_FAILED;
  }

  // Attach shared memory
  char *shm_addr = attach_shared_memory(shmid);

  if (shm_addr == nullptr) {
    fprintf(stderr, "Failed to attach shared memory\n");

    destroy_message_queue(msgid);
    destroy_shared_memory(shmid);

    return SHM_FAILED;
  }

  // Write message into shared memory
  std::cout << "Writing message to shared memory: " << message << std::endl;

  int written = snprintf(shm_addr, BLOCK_SIZE, "%s", message);

  if (written < 0) {
    fprintf(stderr, "Failed to write message\n");

    detach_shared_memory(shm_addr);
    destroy_message_queue(msgid);
    destroy_shared_memory(shmid);

    return SHM_FAILED;
  }

  if (written >= BLOCK_SIZE) {
    fprintf(stderr, "Message is too long. Maximum size is %d bytes\n",
            BLOCK_SIZE - 1);

    detach_shared_memory(shm_addr);
    destroy_message_queue(msgid);
    destroy_shared_memory(shmid);

    return SHM_FAILED;
  }

  /*
   * We are finished accessing the shared memory for now.
   */
  if (detach_shared_memory(shm_addr) == SHM_ERROR) {
    fprintf(stderr, "Failed to detach shared memory\n");

    destroy_message_queue(msgid);
    destroy_shared_memory(shmid);

    return SHM_FAILED;
  }

  // Consumer is blocked in msgrcv() until we write the message into shared
  // memory and send the notification.
  std::cout << "Sending DATA_READY notification..." << std::endl;

  if (send_message(msgid, MSG_DATA_READY, "DATA_READY") == SHM_ERROR) {

    fprintf(stderr, "Failed to send DATA_READY\n");

    destroy_message_queue(msgid);
    destroy_shared_memory(shmid);

    return MSGQ_FAILED;
  }

  char ack[32]{};
  std::cout << "Waiting for consumer acknowledgment..." << std::endl;

  // Wait for consumer acknowledgment
  if (receive_message(msgid, MSG_DATA_READY_ACK, ack, sizeof(ack)) ==
      SHM_ERROR) {

    fprintf(stderr, "Failed to receive consumer acknowledgment\n");

    destroy_message_queue(msgid);
    destroy_shared_memory(shmid);

    return MSGQ_FAILED;
  }

  std::cout << "Consumer acknowledged: " << ack << std::endl;

  // Cleanup
  if (destroy_message_queue(msgid) == SHM_ERROR) {
    fprintf(stderr, "Failed to destroy message queue\n");
  }

  if (destroy_shared_memory(shmid) == SHM_ERROR) {
    fprintf(stderr, "Failed to destroy shared memory\n");
    return SHM_FAILED;
  }

  std::cout << "Producer finished successfully." << std::endl;

  return SUCCESS;
}