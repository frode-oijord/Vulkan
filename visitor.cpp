#include <memory>
#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include <any>

struct Visitor {

	template <typename NodeType>
	void apply(NodeType* node)
	{
		std::type_index type = typeid(NodeType);
		auto it = operations.find(type);
		if (it != this->operations.end()) {
			auto operation = std::any_cast<std::function<void(NodeType*)>>(it->second);
			operation(node);
		}
	}

	template <typename NodeType>
	void register_operation(std::function<void(NodeType*)> operation)
	{
		this->operations[typeid(NodeType)] = operation;
	}

	std::map<std::type_index, std::any> operations;
};


struct Node {
	virtual void visit(Visitor* visitor) = 0;
};


struct Group : public Node {
	void visit(Visitor* visitor) override
	{
		visitor->apply(this);
		for (auto child : this->children) {
			child->visit(visitor);
		}
	}

	std::vector<std::shared_ptr<Node>> children;
};


struct BuiltinNode : public Node {
	void visit(Visitor* visitor) override
	{
		visitor->apply(this);
	}
};


struct BuiltinVisitor 
{
	BuiltinVisitor()
	{
		this->visitor.register_operation<Group>([this](Group* node) {
			this->visit(node);
		});
		this->visitor.register_operation<BuiltinNode>([this](BuiltinNode* node) {
			this->visit(node);
		});
	}

	void apply(Node* root)
	{
		this->state.clear();
		root->visit(&this->visitor);
		std::cout << this->state << std::endl;
	}

	void visit(Group* group)
	{
		this->state += "Group node visited by builtin visitor\n";
	}

	void visit(BuiltinNode* node)
	{
		this->state += "BuiltinNode node visited by builtin visitor\n";
	}

	Visitor visitor;
	std::string state;
};

static BuiltinVisitor builtin_visitor;

struct CustomNode : public Node {
	void visit(Visitor* visitor) override
	{
		visitor->apply(this);
	}
};

struct CustomVisitor {

	CustomVisitor()
	{
		this->visitor.register_operation<Group>([this](Group* node) {
			this->visit(node);
		});
		this->visitor.register_operation<BuiltinNode>([this](BuiltinNode* node) {
			this->visit(node);
		});
		this->visitor.register_operation<CustomNode>([this](CustomNode* node) {
			this->visit(node);
  	});
	}

	void apply(Node* root)
	{
		this->state.clear();
		root->visit(&this->visitor);
		std::cout << this->state << std::endl;
	}

	void visit(Group* node)
	{
		this->state += "Group node visited by custom visitor\n";
	}

	void visit(BuiltinNode* node)
	{
		this->state += "BuiltinNode node visited by custom visitor\n";
	}

	void visit(CustomNode* node)
	{
		this->state += "CustomNode node visited by custom visitor\n";
	}

	Visitor visitor;
	std::string state;
};
	
static CustomVisitor custom_visitor;

int main()
{
	builtin_visitor.visitor.register_operation<CustomNode>([](CustomNode* node) {
		builtin_visitor.state += "Custom node visited by builtin visitor\n";
	});

	auto root = std::make_shared<Group>();
	root->children = {
		std::make_shared<BuiltinNode>(),
		std::make_shared<CustomNode>(),
	};

	builtin_visitor.apply(root.get());
	custom_visitor.apply(root.get());
}
