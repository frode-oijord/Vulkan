#pragma once

#include <Innovator/Defines.h>
#include <vector>
#include <memory>

class Node {
public:
  NO_COPY_OR_ASSIGNMENT(Node)

  Node() = default;
  virtual ~Node() = default;

  void alloc(class Context* context)
  {
    this->doAlloc(context);
  }

  void resize(class Context* context)
  {
    this->doResize(context);
  }

  void stage(class Context* context)
  {
    this->doStage(context);
  }

  void pipeline(class Context* context)
  {
    this->doPipeline(context);
  }

  void record(class Context* context)
  {
    this->doRecord(context);
  }

  void event(class Context* context)
  {
    this->doEvent(context);
  }

  void render(class Context* context)
  {
    this->doRender(context);
  }

  void present(class Context* context)
  {
    this->doPresent(context);
  }

private:
  virtual void doAlloc(class Context*) {}
  virtual void doResize(class Context*) {}
  virtual void doStage(class Context*) {}
  virtual void doPipeline(class Context*) {}
  virtual void doRecord(class Context*) {}
  virtual void doEvent(class Context*) {}
  virtual void doRender(class Context*) {}
  virtual void doPresent(class Context*) {}
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
  void doAlloc(Context* context) override
  {
    for (const auto& node : this->children) {
      node->alloc(context);
    }
  }

  void doResize(Context* context) override
  {
    for (const auto& node : this->children) {
      node->resize(context);
    }
  }

  void doStage(Context* context) override
  {
    for (const auto& node : this->children) {
      node->stage(context);
    }
  }

  void doPipeline(Context* creator) override
  {
    for (const auto& node : this->children) {
      node->pipeline(creator);
    }
  }

  void doRecord(Context* context) override
  {
    for (const auto& node : this->children) {
      node->record(context);
    }
  }

  void doEvent(class Context* context) override
  {
    for (const auto& node : this->children) {
      node->event(context);
    }
  }

  void doRender(class Context* context) override
  {
    for (const auto& node : this->children) {
      node->render(context);
    }
  }

  void doPresent(class Context* context) override
  {
    for (const auto& node : this->children) {
      node->present(context);
    }
  }
};
