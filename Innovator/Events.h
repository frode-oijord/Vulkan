#pragma once

struct Event {
  virtual void dummy() {};
};

struct MousePressEvent : public Event {
  MousePressEvent(int x, int y, int button)
    : button(button)
  {
    this->pos = {
      static_cast<double>(x),
      static_cast<double>(y)
    };
  }
  glm::dvec2 pos{};
  int button;
};

struct MouseReleaseEvent : public Event {
  MouseReleaseEvent() = default;
};

struct MouseMoveEvent : public Event {
  MouseMoveEvent(int x, int y)
  {
    this->pos = {
      static_cast<double>(x),
      static_cast<double>(y)
    };
  }
  glm::dvec2 pos{};
};