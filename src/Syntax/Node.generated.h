// Automatically generated by script/generate_ast.py.
// Do not hand-edit.

class AndNode;
class BinaryOpNode;
class BoolNode;
class CallNode;
class DefMethodNode;
class DoNode;
class IfNode;
class NameNode;
class NotNode;
class NothingNode;
class NumberNode;
class OrNode;
class RecordNode;
class ReturnNode;
class SequenceNode;
class StringNode;
class VariableNode;
class NothingPattern;
class VariablePattern;

class NodeVisitor
{
public:
  virtual ~NodeVisitor() {}

  virtual void visit(const AndNode& node, int dest) = 0;
  virtual void visit(const BinaryOpNode& node, int dest) = 0;
  virtual void visit(const BoolNode& node, int dest) = 0;
  virtual void visit(const CallNode& node, int dest) = 0;
  virtual void visit(const DefMethodNode& node, int dest) = 0;
  virtual void visit(const DoNode& node, int dest) = 0;
  virtual void visit(const IfNode& node, int dest) = 0;
  virtual void visit(const NameNode& node, int dest) = 0;
  virtual void visit(const NotNode& node, int dest) = 0;
  virtual void visit(const NothingNode& node, int dest) = 0;
  virtual void visit(const NumberNode& node, int dest) = 0;
  virtual void visit(const OrNode& node, int dest) = 0;
  virtual void visit(const RecordNode& node, int dest) = 0;
  virtual void visit(const ReturnNode& node, int dest) = 0;
  virtual void visit(const SequenceNode& node, int dest) = 0;
  virtual void visit(const StringNode& node, int dest) = 0;
  virtual void visit(const VariableNode& node, int dest) = 0;

protected:
  NodeVisitor() {}

private:
  NO_COPY(NodeVisitor);
};

// Base class for all AST node classes.
class Node : public Managed
{
public:
  Node(const SourcePos& pos)
  : pos_(pos)
  {}

  virtual ~Node() {}

  // The visitor pattern.
  virtual void accept(NodeVisitor& visitor, int arg) const = 0;

  // Dynamic casts.
    virtual const AndNode* asAndNode() const { return NULL; }
  virtual const BinaryOpNode* asBinaryOpNode() const { return NULL; }
  virtual const BoolNode* asBoolNode() const { return NULL; }
  virtual const CallNode* asCallNode() const { return NULL; }
  virtual const DefMethodNode* asDefMethodNode() const { return NULL; }
  virtual const DoNode* asDoNode() const { return NULL; }
  virtual const IfNode* asIfNode() const { return NULL; }
  virtual const NameNode* asNameNode() const { return NULL; }
  virtual const NotNode* asNotNode() const { return NULL; }
  virtual const NothingNode* asNothingNode() const { return NULL; }
  virtual const NumberNode* asNumberNode() const { return NULL; }
  virtual const OrNode* asOrNode() const { return NULL; }
  virtual const RecordNode* asRecordNode() const { return NULL; }
  virtual const ReturnNode* asReturnNode() const { return NULL; }
  virtual const SequenceNode* asSequenceNode() const { return NULL; }
  virtual const StringNode* asStringNode() const { return NULL; }
  virtual const VariableNode* asVariableNode() const { return NULL; }

  const SourcePos& pos() const { return pos_; }

private:
  SourcePos pos_;
};

class AndNode : public Node
{
public:
  AndNode(const SourcePos& pos, gc<Node> left, gc<Node> right)
  : Node(pos),
    left_(left),
    right_(right)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const AndNode* asAndNode() const { return this; }

  gc<Node> left() const { return left_; }
  gc<Node> right() const { return right_; }

  virtual void reach()
  {
    Memory::reach(left_);
    Memory::reach(right_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> left_;
  gc<Node> right_;
};

class BinaryOpNode : public Node
{
public:
  BinaryOpNode(const SourcePos& pos, gc<Node> left, TokenType type, gc<Node> right)
  : Node(pos),
    left_(left),
    type_(type),
    right_(right)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const BinaryOpNode* asBinaryOpNode() const { return this; }

  gc<Node> left() const { return left_; }
  TokenType type() const { return type_; }
  gc<Node> right() const { return right_; }

  virtual void reach()
  {
    Memory::reach(left_);
    Memory::reach(right_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> left_;
  TokenType type_;
  gc<Node> right_;
};

class BoolNode : public Node
{
public:
  BoolNode(const SourcePos& pos, bool value)
  : Node(pos),
    value_(value)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const BoolNode* asBoolNode() const { return this; }

  bool value() const { return value_; }

  virtual void trace(std::ostream& out) const;

private:
  bool value_;
};

class CallNode : public Node
{
public:
  CallNode(const SourcePos& pos, gc<Node> leftArg, gc<String> name, gc<Node> rightArg)
  : Node(pos),
    leftArg_(leftArg),
    name_(name),
    rightArg_(rightArg)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const CallNode* asCallNode() const { return this; }

  gc<Node> leftArg() const { return leftArg_; }
  gc<String> name() const { return name_; }
  gc<Node> rightArg() const { return rightArg_; }

  virtual void reach()
  {
    Memory::reach(leftArg_);
    Memory::reach(name_);
    Memory::reach(rightArg_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> leftArg_;
  gc<String> name_;
  gc<Node> rightArg_;
};

class DefMethodNode : public Node
{
public:
  DefMethodNode(const SourcePos& pos, gc<Pattern> leftParam, gc<String> name, gc<Pattern> rightParam, gc<Node> body)
  : Node(pos),
    leftParam_(leftParam),
    name_(name),
    rightParam_(rightParam),
    body_(body)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const DefMethodNode* asDefMethodNode() const { return this; }

  gc<Pattern> leftParam() const { return leftParam_; }
  gc<String> name() const { return name_; }
  gc<Pattern> rightParam() const { return rightParam_; }
  gc<Node> body() const { return body_; }

  virtual void reach()
  {
    Memory::reach(leftParam_);
    Memory::reach(name_);
    Memory::reach(rightParam_);
    Memory::reach(body_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Pattern> leftParam_;
  gc<String> name_;
  gc<Pattern> rightParam_;
  gc<Node> body_;
};

class DoNode : public Node
{
public:
  DoNode(const SourcePos& pos, gc<Node> body)
  : Node(pos),
    body_(body)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const DoNode* asDoNode() const { return this; }

  gc<Node> body() const { return body_; }

  virtual void reach()
  {
    Memory::reach(body_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> body_;
};

class IfNode : public Node
{
public:
  IfNode(const SourcePos& pos, gc<Node> condition, gc<Node> thenArm, gc<Node> elseArm)
  : Node(pos),
    condition_(condition),
    thenArm_(thenArm),
    elseArm_(elseArm)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const IfNode* asIfNode() const { return this; }

  gc<Node> condition() const { return condition_; }
  gc<Node> thenArm() const { return thenArm_; }
  gc<Node> elseArm() const { return elseArm_; }

  virtual void reach()
  {
    Memory::reach(condition_);
    Memory::reach(thenArm_);
    Memory::reach(elseArm_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> condition_;
  gc<Node> thenArm_;
  gc<Node> elseArm_;
};

class NameNode : public Node
{
public:
  NameNode(const SourcePos& pos, gc<String> name)
  : Node(pos),
    name_(name)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const NameNode* asNameNode() const { return this; }

  gc<String> name() const { return name_; }

  virtual void reach()
  {
    Memory::reach(name_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<String> name_;
};

class NotNode : public Node
{
public:
  NotNode(const SourcePos& pos, gc<Node> value)
  : Node(pos),
    value_(value)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const NotNode* asNotNode() const { return this; }

  gc<Node> value() const { return value_; }

  virtual void reach()
  {
    Memory::reach(value_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> value_;
};

class NothingNode : public Node
{
public:
  NothingNode(const SourcePos& pos)
  : Node(pos)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const NothingNode* asNothingNode() const { return this; }


  virtual void trace(std::ostream& out) const;

private:
};

class NumberNode : public Node
{
public:
  NumberNode(const SourcePos& pos, double value)
  : Node(pos),
    value_(value)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const NumberNode* asNumberNode() const { return this; }

  double value() const { return value_; }

  virtual void trace(std::ostream& out) const;

private:
  double value_;
};

class OrNode : public Node
{
public:
  OrNode(const SourcePos& pos, gc<Node> left, gc<Node> right)
  : Node(pos),
    left_(left),
    right_(right)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const OrNode* asOrNode() const { return this; }

  gc<Node> left() const { return left_; }
  gc<Node> right() const { return right_; }

  virtual void reach()
  {
    Memory::reach(left_);
    Memory::reach(right_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> left_;
  gc<Node> right_;
};

class RecordNode : public Node
{
public:
  RecordNode(const SourcePos& pos, const Array<Field>& fields)
  : Node(pos),
    fields_(fields)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const RecordNode* asRecordNode() const { return this; }

  Array<Field> fields() const { return fields_; }

  virtual void reach()
  {

    for (int i = 0; i < fields_.count(); i++)
    {
        Memory::reach(fields_[i].name);
        Memory::reach(fields_[i].value);
    }
  }

  virtual void trace(std::ostream& out) const;

private:
  Array<Field> fields_;
};

class ReturnNode : public Node
{
public:
  ReturnNode(const SourcePos& pos, gc<Node> value)
  : Node(pos),
    value_(value)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const ReturnNode* asReturnNode() const { return this; }

  gc<Node> value() const { return value_; }

  virtual void reach()
  {
    Memory::reach(value_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<Node> value_;
};

class SequenceNode : public Node
{
public:
  SequenceNode(const SourcePos& pos, const Array<gc<Node> >& expressions)
  : Node(pos),
    expressions_(expressions)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const SequenceNode* asSequenceNode() const { return this; }

  Array<gc<Node> > expressions() const { return expressions_; }

  virtual void reach()
  {
    Memory::reach(expressions_);
  }

  virtual void trace(std::ostream& out) const;

private:
  Array<gc<Node> > expressions_;
};

class StringNode : public Node
{
public:
  StringNode(const SourcePos& pos, gc<String> value)
  : Node(pos),
    value_(value)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const StringNode* asStringNode() const { return this; }

  gc<String> value() const { return value_; }

  virtual void reach()
  {
    Memory::reach(value_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<String> value_;
};

class VariableNode : public Node
{
public:
  VariableNode(const SourcePos& pos, bool isMutable, gc<Pattern> pattern, gc<Node> value)
  : Node(pos),
    isMutable_(isMutable),
    pattern_(pattern),
    value_(value)
  {}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const VariableNode* asVariableNode() const { return this; }

  bool isMutable() const { return isMutable_; }
  gc<Pattern> pattern() const { return pattern_; }
  gc<Node> value() const { return value_; }

  virtual void reach()
  {
    Memory::reach(pattern_);
    Memory::reach(value_);
  }

  virtual void trace(std::ostream& out) const;

private:
  bool isMutable_;
  gc<Pattern> pattern_;
  gc<Node> value_;
};

class PatternVisitor
{
public:
  virtual ~PatternVisitor() {}

  virtual void visit(const NothingPattern& node, int dest) = 0;
  virtual void visit(const VariablePattern& node, int dest) = 0;

protected:
  PatternVisitor() {}

private:
  NO_COPY(PatternVisitor);
};

// Base class for all AST node classes.
class Pattern : public Managed
{
public:
  Pattern(const SourcePos& pos)
  : pos_(pos)
  {}

  virtual ~Pattern() {}

  // The visitor pattern.
  virtual void accept(PatternVisitor& visitor, int arg) const = 0;

  // Dynamic casts.
    virtual const NothingPattern* asNothingPattern() const { return NULL; }
  virtual const VariablePattern* asVariablePattern() const { return NULL; }

  const SourcePos& pos() const { return pos_; }

private:
  SourcePos pos_;
};

class NothingPattern : public Pattern
{
public:
  NothingPattern(const SourcePos& pos)
  : Pattern(pos)
  {}

  virtual void accept(PatternVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const NothingPattern* asNothingPattern() const { return this; }


  virtual void trace(std::ostream& out) const;

private:
};

class VariablePattern : public Pattern
{
public:
  VariablePattern(const SourcePos& pos, gc<String> name)
  : Pattern(pos),
    name_(name)
  {}

  virtual void accept(PatternVisitor& visitor, int arg) const
  {
    visitor.visit(*this, arg);
  }

  virtual const VariablePattern* asVariablePattern() const { return this; }

  gc<String> name() const { return name_; }

  virtual void reach()
  {
    Memory::reach(name_);
  }

  virtual void trace(std::ostream& out) const;

private:
  gc<String> name_;
};
