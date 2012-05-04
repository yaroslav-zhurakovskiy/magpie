#!/usr/bin/python

# Generates the C++ code for defining the AST classes. The classes for defining
# the Nodes are pretty much complete boilerplate. They are basically dumb
# data structures. This generates that boilerplate.
from os.path import dirname, join, realpath

magpie_dir = dirname(dirname(realpath(__file__)))
header_path = join(magpie_dir, 'src', 'Syntax', 'Node.generated.h')

# Define the AST node classes.
nodes = sorted({
  'And': [
    ('left',        'gc<Node>'),
    ('right',       'gc<Node>')],
  'BinaryOp': [
    ('left',        'gc<Node>'),
    ('type',        'TokenType'),
    ('right',       'gc<Node>')],
  'Bool': [
    ('value',       'bool')],
  'Call': [
    ('leftArg',     'gc<Node>'),
    ('name',        'gc<String>'),
    ('rightArg',    'gc<Node>')],
  'DefMethod': [
    ('name',        'gc<String>'),
    ('parameter',   'gc<Pattern>'),
    ('body',        'gc<Node>')],
  'Do': [
    ('body',        'gc<Node>')],
  'If': [
    ('condition',   'gc<Node>'),
    ('thenArm',     'gc<Node>'),
    ('elseArm',     'gc<Node>')],
  'Name': [
    ('name',        'gc<String>')],
  'Nothing': [],
  'Number': [
    ('value',       'double')],
  'Or': [
    ('left',        'gc<Node>'),
    ('right',       'gc<Node>')],
  'Sequence': [
    ('expressions', 'Array<gc<Node> >')],
  'String': [
    ('value',       'gc<String>')],
  'Variable': [
    ('isMutable',   'bool'),
    ('pattern',     'gc<Pattern>'),
    ('value',       'gc<Node>')]
}.items())

# Create the Node AST header.
with open(header_path, 'w') as file:
  file.write('''// Automatically generated by script/generate_ast.py.
// Do not hand-edit.

''')

  # Write the forward declarations for the visitor.
  for className, fields in nodes:
    file.write('class {0}Node;\n'.format(className))

  # Write the node visitor class.
  file.write('''
// Visitor pattern for dispatching on AST nodes. Implemented by the compiler.
class NodeVisitor
{
public:
  virtual ~NodeVisitor() {}

''')

  for className, fields in nodes:
    file.write('  virtual void visit(const {0}Node& node, int dest) = 0;\n'
        .format(className))

  file.write('''
protected:
  NodeVisitor() {}

private:
  NO_COPY(NodeVisitor);
};
''')

  # Create the base node class.
  casts = ''
  for className, fields in nodes:
    casts += '  virtual const {0}Node* as{0}Node()'.format(className)
    casts += ' const { return NULL; }\n'

  file.write('''
// Base class for all AST node classes.
class Node : public Managed
{
public:
  Node(const SourcePos& pos)
  : pos_(pos)
  {}

  // The visitor pattern.
  virtual void accept(NodeVisitor& visitor, int arg) const = 0;

  // Dynamic casts.
''')
  file.write(casts)
  file.write('''
  const SourcePos& pos() const { return pos_; }

private:
  SourcePos pos_;
};
''')

  # Create the node subclasses.
  for className, fields in nodes:
    print className
    ctorParams = ''
    ctorArgs = ''
    accessors = ''
    memberVars = ''
    reachFields = ''
    for name, type in fields:
      # Only generate a reach() method if the class contains a GC-ed field.
      if type.find('gc<') != -1:
        reachFields += '    Memory::reach(' + name + '_);\n'

      ctorParams += ', '
      if type.startswith('Array'):
        # Array fields are passed to the constructor by reference.
        ctorParams += 'const ' + type + '&'
      else:
        ctorParams += type
      ctorParams += ' ' + name

      ctorArgs += ',\n    {0}_({0})'.format(name)
      accessors += '  {1} {0}() const {{ return {0}_; }}\n'.format(name, type)
      memberVars += '\n  {1} {0}_;'.format(name, type)

    reach = ''
    if reachFields != '':
      reach = '''
  virtual void reach()
  {{
{0}  }}
'''.format(reachFields)

    file.write('''
class {0}Node : public Node
{{
public:
  {0}Node(const SourcePos& pos{1})
  : Node(pos){2}
  {{}}

  virtual void accept(NodeVisitor& visitor, int arg) const
  {{
    visitor.visit(*this, arg);
  }}

  virtual const {0}Node* as{0}Node() const {{ return this; }}

{3}{4}
  virtual void trace(std::ostream& out) const;

private:{5}
}};
'''.format(className, ctorParams, ctorArgs, accessors, reach, memberVars))
