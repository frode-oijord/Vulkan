
#include <Innovator/Visitor.h>
#include <Innovator/Nodes.h>

Visitor::Visitor(std::shared_ptr<Context> context)
	: context(std::move(context))
{
	auto visit_group = [this](Group* node) {
		for (auto child : node->children) {
			child->visit(this);
		}
	};

	auto visit_separator = [this, visit_group](Separator* node) {
		State state = this->context->state;
		visit_group(node);
		this->context->state = state;
	};

	this->register_callback<Group>(visit_group);
	this->register_callback<Scene>(visit_group);
	this->register_callback<Renderpass>(visit_group);
	this->register_callback<Separator>(visit_separator);
}
