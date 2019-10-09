#pragma once

#include <Innovator/Defines.h>
#include <vector>
#include <memory>

class Node {
public:
  NO_COPY_OR_ASSIGNMENT(Node)

  Node() = default;
  virtual ~Node() = default;

  void alloc(class TraversalContext* context)
  {
    this->doAlloc(context);
  }

  void resize(class TraversalContext* context)
  {
    this->doResize(context);
  }

  void stage(class TraversalContext* context)
  {
    this->doStage(context);
  }

  void pipeline(class TraversalContext* creator)
  {
    this->doPipeline(creator);
  }

  void record(class TraversalContext* recorder)
  {
    this->doRecord(recorder);
  }

  void render(class SceneRenderer * renderer)
  {
    this->doRender(renderer);
  }

  void present(class TraversalContext* context)
  {
    this->doPresent(context);
  }

private:
  virtual void doAlloc(class TraversalContext*) {}
  virtual void doResize(class TraversalContext*) {}
  virtual void doStage(class TraversalContext*) {}
  virtual void doPipeline(class TraversalContext*) {}
  virtual void doRecord(class TraversalContext*) {}
  virtual void doRender(class SceneRenderer*) {}
  virtual void doPresent(class TraversalContext*) {}
};

class Group : public Node {
public:
  NO_COPY_OR_ASSIGNMENT(Group)
  Group() = default;
  virtual ~Group() = default;

  explicit Group(std::vector<std::shared_ptr<Node>> children)
    : children(std::move(children)) {}

  std::vector<std::shared_ptr<Node>> children;

protected:
  void doAlloc(TraversalContext* context) override
  {
    for (const auto& node : this->children) {
      node->alloc(context);
    }
  }

  void doResize(TraversalContext* context) override
  {
    for (const auto& node : this->children) {
      node->resize(context);
    }
  }

  void doStage(TraversalContext* context) override
  {
    for (const auto& node : this->children) {
      node->stage(context);
    }
  }

  void doPipeline(TraversalContext* creator) override
  {
    for (const auto& node : this->children) {
      node->pipeline(creator);
    }
  }

  void doRecord(TraversalContext* recorder) override
  {
    for (const auto& node : this->children) {
      node->record(recorder);
    }
  }

  void doRender(class SceneRenderer * renderer) override
  {
    for (const auto& node : this->children) {
      node->render(renderer);
    }
  }

  void doPresent(class TraversalContext* context) override
  {
    for (const auto& node : this->children) {
      node->present(context);
    }
  }
};
