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
        ('leftParam',   'gc<Pattern>'),
        ('name',        'gc<String>'),
        ('rightParam',  'gc<Pattern>'),
        ('body',        'gc<Node>')],
    'Do': [
        ('body',        'gc<Node>')],
    'If': [
        ('condition',   'gc<Node>'),
        ('thenArm',     'gc<Node>'),
        ('elseArm',     'gc<Node>')],
    'Name': [
        ('name',        'gc<String>')],
    'Not': [
        ('value',       'gc<Node>')],
    'Nothing': [],
    'Number': [
        ('value',       'double')],
    'Or': [
        ('left',        'gc<Node>'),
        ('right',       'gc<Node>')],
    'Record': [
        ('fields',      'Array<Field>')],
    'Return': [
        ('value',       'gc<Node>')],
    'Sequence': [
        ('expressions', 'Array<gc<Node> >')],
    'String': [
        ('value',       'gc<String>')],
    'Variable': [
        ('isMutable',   'bool'),
        ('pattern',     'gc<Pattern>'),
        ('value',       'gc<Node>')]
}.items())

patterns = sorted({
    'Nothing': [],
    'Variable': [
        ('name', 'gc<String>')]
}.items())

HEADER = '''// Automatically generated by script/generate_ast.py.
// Do not hand-edit.

'''

REACH_METHOD = '''
  virtual void reach()
  {{
{0}  }}
'''

REACH_FIELD_ARRAY = '''
    for (int i = 0; i < {0}_.count(); i++)
    {{
        Memory::reach({0}_[i].name);
        Memory::reach({0}_[i].value);
    }}
'''

BASE_CLASS = '''
// Base class for all AST node classes.
class {0} : public Managed
{{
public:
  {0}(const SourcePos& pos)
  : pos_(pos)
  {{}}

  virtual ~{0}() {{}}

  // The visitor pattern.
  virtual void accept({0}Visitor& visitor, int arg) const = 0;

  // Dynamic casts.
  {1}
  const SourcePos& pos() const {{ return pos_; }}

private:
  SourcePos pos_;
}};
'''

SUBCLASS = '''
class {0}{6} : public {6}
{{
public:
  {0}{6}(const SourcePos& pos{1})
  : {6}(pos){2}
  {{}}

  virtual void accept({6}Visitor& visitor, int arg) const
  {{
    visitor.visit(*this, arg);
  }}

  virtual const {0}{6}* as{0}{6}() const {{ return this; }}

{3}{4}
  virtual void trace(std::ostream& out) const;

private:{5}
}};
'''

VISITOR_HEADER = '''
class {0}Visitor
{{
public:
  virtual ~{0}Visitor() {{}}

'''

VISITOR_FOOTER = '''
protected:
  {0}Visitor() {{}}

private:
  NO_COPY({0}Visitor);
}};
'''

def main():
    # Create the Node AST header.
    with open(header_path, 'w') as file:
        file.write(HEADER)

        # Write the forward declarations for the visitor.
        for className, fields in nodes:
            file.write('class {0}Node;\n'.format(className))
        for pattern, fields in patterns:
            file.write('class {0}Pattern;\n'.format(pattern))

        # Write the node visitor class.
        file.write(makeVisitor('Node', nodes))

        # Create the base Node class.
        file.write(makeBaseClass('Node', nodes))

        # Create the node subclasses.
        for node, fields in nodes:
            file.write(makeClass('Node', node, fields))

        # Write the pattern visitor class.
        file.write(makeVisitor('Pattern', patterns))

        # Create the base Pattern class.
        file.write(makeBaseClass('Pattern', patterns))

        # Create the pattern subclasses.
        for pattern, fields in patterns:
            file.write(makeClass('Pattern', pattern, fields))


def makeVisitor(name, types):
    result = VISITOR_HEADER.format(name)

    for className, fields in types:
        result += ('  virtual void visit(const {1}{0}& node, int dest) = 0;\n'
            .format(name, className))

    return result + VISITOR_FOOTER.format(name)


def makeClass(baseClass, className, fields):
    print className
    ctorParams = ''
    ctorArgs = ''
    accessors = ''
    memberVars = ''
    reachFields = ''
    for name, type in fields:
        if type.find('gc<') != -1:
            reachFields += '    Memory::reach(' + name + '_);\n'
        if type == 'Array<Field>':
            reachFields += REACH_FIELD_ARRAY.format(name)

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
    # Only generate a reach() method if the class contains a GC-ed field.
    if reachFields != '':
        reach = REACH_METHOD.format(reachFields)

    return SUBCLASS.format(className, ctorParams, ctorArgs, accessors, reach,
                           memberVars, baseClass)

def makeBaseClass(name, types):
    casts = ''
    for subclass, fields in types:
        casts += '  virtual const {1}{0}* as{1}{0}()'.format(name, subclass)
        casts += ' const { return NULL; }\n'

    return BASE_CLASS.format(name, casts)

main()