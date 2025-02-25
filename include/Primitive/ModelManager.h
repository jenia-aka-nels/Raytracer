#pragma once
#include "Model.h"

class Model3DManager {
 private:
  std::shared_ptr<DescriptorSetLayout> _descriptorSetLayout;
  std::shared_ptr<Pipeline> _pipeline;

  int _descriptorPoolSize = 100;
  int _modelsCreated = 0;
  std::vector<std::shared_ptr<DescriptorPool>> _descriptorPool;
  std::shared_ptr<CommandPool> _commandPool;
  std::shared_ptr<CommandBuffer> _commandBuffer;
  std::shared_ptr<Queue> _queue;
  std::shared_ptr<Device> _device;
  std::shared_ptr<Settings> _settings;

  std::vector<std::shared_ptr<Model3D>> _models;

 public:
  Model3DManager(std::shared_ptr<CommandPool> commandPool,
                 std::shared_ptr<CommandBuffer> commandBuffer,
                 std::shared_ptr<Queue> queue,
                 std::shared_ptr<RenderPass> render,
                 std::shared_ptr<Device> device,
                 std::shared_ptr<Settings> settings);

  std::shared_ptr<Model3D> createModel(std::string path, std::shared_ptr<Texture> texture);
  void registerModel(std::shared_ptr<Model3D> model);
  void unregisterModel(std::shared_ptr<Model3D> model);
  void draw(int currentFrame);
};