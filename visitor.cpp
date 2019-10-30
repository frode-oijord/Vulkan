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

private:
	std::map<std::type_index, std::any> operations;
};


struct Node {
	virtual void visit(Visitor* visitor) = 0;
};


struct Group : public Node {
	void visit(Visitor* visitor) override
	{
		visitor->apply(this);
	}

	void visitChildren(Visitor* visitor)
	{
		for (auto child : this->children) {
			child->visit(visitor);
		}
	}

	std::vector<std::shared_ptr<Node>> children;
};


struct Text : public Node {
	void visit(Visitor* visitor) override
	{
		visitor->apply(this);
	}

	std::string data;
};


struct TextVisitor {

	TextVisitor()
	{
		//this->visitor.register_operation<Group>([this](Group* group) {
		//	this->apply_group(group);
		//	});
		this->visitor.register_operation<Group>(std::bind(&TextVisitor::apply_group, this, std::placeholders::_1));

		this->visitor.register_operation<Text>(apply_text);
	}

	void apply(Node* node)
	{
		node->visit(&this->visitor);
	}

	void apply_group(Group* group) 
	{
		std::cout << "Group node visited" << std::endl;
		group->visitChildren(&this->visitor);
	}

	static void apply_text(Text* node)
	{
		std::cout << "Text node visited" << std::endl;
	}

	Visitor visitor;
};


struct CustomNode : public Node {
	void visit(Visitor* visitor) override 
	{
		visitor->apply(this);
	}
};


void apply_text(Text* node)
{
	std::cout << "Text node visited by custom visitor" << std::endl;
}


void apply_custom(CustomNode* node)
{
	std::cout << "Custom node visited by custom visitor" << std::endl;
}


int main()
{
	auto root = std::make_shared<Group>();
	root->children = {
		std::make_shared<Text>(),
		std::make_shared<CustomNode>(),
	};

	TextVisitor text_visitor;
	text_visitor.visitor.register_operation<CustomNode>([](CustomNode* node) {
		std::cout << "Custom node visited" << std::endl;
	});
	text_visitor.apply(root.get());

	Visitor custom_visitor;
	custom_visitor.register_operation<Text>(apply_text);
	custom_visitor.register_operation<Group>([&](Group* group) {
		std::cout << "Group node visited by custom visitor" << std::endl;
		group->visitChildren(&custom_visitor);
	});
	custom_visitor.register_operation<CustomNode>(apply_custom);

	root->visit(&custom_visitor);
}

