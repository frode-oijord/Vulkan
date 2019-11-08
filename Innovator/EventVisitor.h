#pragma once

#include <Innovator/Visitor.h>

class EventVisitor : public Visitor {
public:
	EventVisitor(std::shared_ptr<Context> context);
	void visit(class ViewMatrix* node);

private:
	std::shared_ptr<class MousePressEvent> press{ nullptr };
};