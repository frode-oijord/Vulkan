
#include <any>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <unordered_map>

// Nodes can modify the state when they're visited, e.g. change the current transform,
// or set the current index buffer, texture image etc...
struct State 
{
	std::string indent;
};


// The context we're visiting the nodes in, e.g. Vulkan instance, Vulkan device. 
struct Context 
{
	State state;
};


// The visitor base. Contains a map that map node types to callbacks. 
struct Visitor 
{
	Visitor(std::shared_ptr<Context> context)
		: context(std::move(context))
	{}

	// a node calls this method when visited. If there's a matching callback for the given node 
	// type, it is called.
	template <typename NodeType>
	void apply(NodeType* node)
	{
		std::type_index type = typeid(NodeType);
		auto it = callbacks.find(type);
		if (it != this->callbacks.end()) {
			auto callback = std::any_cast<std::function<void(NodeType*)>>(it->second);
			callback(node);
		}
	}

	// register a callback for a node type
	template <typename NodeType>
	void register_callback(std::function<void(NodeType*)> operation)
	{
		this->callbacks[typeid(NodeType)] = operation;
	}

	std::shared_ptr<Context> context;
	std::unordered_map<std::type_index, std::any> callbacks;
};


struct Node 
{
	Node(std::string name) : 
		name(name)	
	{}

	virtual void visit(Visitor* visitor) = 0;

	std::string name;
};

#define VISITABLE											\
void visit(Visitor* visitor) override \
{																			\
	visitor->apply(this);								\
}																			\


struct Label : Node 
{
	VISITABLE
	Label() :
		Node("Label") 
	{}
};

struct Group : public Node 
{
	VISITABLE
	Group(std::string name) :
		Node(name)
	{}

	void visitChildren(Visitor* visitor)
	{
		for (auto child : this->children) {
			child->visit(visitor);
		}
	}

	std::vector<std::shared_ptr<Node>> children;
};


struct Separator : public Group 
{
	VISITABLE
	Separator(std::string name) :
		Group(name)
	{}
};


// print the scene graph, indent child nodes
struct SceneGraphPrinter : public Visitor
{
	SceneGraphPrinter(std::shared_ptr<Context> context)
		:	Visitor(context)
	{
		this->register_callback<Label>([this](Label* node) {
			this->visit(node);
		});
		this->register_callback<Group>([this](Group* node) {
			this->visit(node);
		});
		this->register_callback<Separator>([this](Separator* node) {
			this->visit(node);
		});
	}

	void visit(Label* node)
	{
		std::cout << this->context->state.indent << node->name << std::endl;
	}

	void visit(Group* node)
	{
		std::cout << this->context->state.indent << node->name << std::endl;
		this->context->state.indent += "  ";
		node->visitChildren(this);
	}

	void visit(Separator* node)
	{
		std::cout << this->context->state.indent << node->name << std::endl;

		State state = this->context->state;
		this->context->state.indent += "  ";
		node->visitChildren(this);
		this->context->state = state;
	}
};


// node created by client code
struct Custom : public Node 
{
	Custom() :
		Node("Custom")
	{}

	void visit(Visitor* visitor) override
	{
		visitor->apply(this);
	}
};

// visitor, created by client code
struct CustomPrinter : public Visitor
{
	CustomPrinter(std::shared_ptr<Context> context)
		:	Visitor(context)
	{
		this->register_callback<Custom>([this](Custom* node) {
			this->print(node);
		});
		this->register_callback<Label>([this](Label* node) {
			this->print(node);
		});
		this->register_callback<Group>([this](Group* node) {
			this->print(node);
		});
		this->register_callback<Separator>([this](Separator* node) {
			this->print(node);
		});
	}

	// the custom visitor simply adds a "custom: " to each line of output
	void print(Custom* node)
	{
		std::cout << this->context->state.indent << "custom: " << node->name << std::endl;
	}

	void print(Label* node)
	{
		std::cout << this->context->state.indent << "custom: " << node->name << std::endl;
	}

	void print(Group* node)
	{
		std::cout << this->context->state.indent << "custom: " << node->name << std::endl;
		this->context->state.indent += "  ";
	}

	void print(Separator* node)
	{
		std::cout << this->context->state.indent << "custom: " << node->name << std::endl;

		State state = this->context->state;
		this->context->state.indent += "  ";
		node->visitChildren(this);
		this->context->state = state;
	}
};


// create a scene graph with a separator inside a separator to show two
// indentation levels
std::shared_ptr<Node> get_scenegraph() 
{
	auto root = std::make_shared<Separator>("Separator");

	auto sep = std::make_shared<Separator>("Separator");
	sep->children = {
		std::make_shared<Label>(),
		std::make_shared<Label>(),
	};

	root->children = {
		std::make_shared<Custom>(),
		std::make_shared<Label>(),
		sep,
		std::make_shared<Label>(),
	};

	return root;
}


int main()
{
	// the context is shared between predefined and custom visitors in this case
	auto context = std::make_shared<Context>();
	SceneGraphPrinter printer(context);
	CustomPrinter custom(context);

	// client code can register callback for custom node
	printer.register_callback<Custom>([&printer](Custom* node) {
		std::cout << printer.context->state.indent << node->name << std::endl;
	});

	auto root = get_scenegraph();

	// visit nodes with predefined printer
	root->visit(&printer);

	// visit nodes with client-code defined printer
	root->visit(&custom);
}
