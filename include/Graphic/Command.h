#pragma once
#include "Device.h"
#include "Queue.h"

class CommandPool {
 private:
  std::shared_ptr<Device> _device;
  VkCommandPool _commandPool;

 public:
  CommandPool(std::shared_ptr<Device> device);
  VkCommandPool& getCommandPool();
  ~CommandPool();
};

class CommandBuffer {
 private:
  std::vector<VkCommandBuffer> _buffer;
  std::shared_ptr<Device> _device;
  std::shared_ptr<CommandPool> _pool;

 public:
  CommandBuffer(int number, std::shared_ptr<CommandPool> pool, std::shared_ptr<Device> device);
  void beginSingleTimeCommands(int commandNumber);
  void endSingleTimeCommands(int commandNumber, std::shared_ptr<Queue> queue);
  std::vector<VkCommandBuffer>& getCommandBuffer();
  ~CommandBuffer();
};