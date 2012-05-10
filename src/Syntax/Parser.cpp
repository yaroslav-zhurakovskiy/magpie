#include <sstream>

#include "Lexer.h"
#include "Node.h"
#include "Parser.h"

namespace magpie
{
  enum Precedence {
    PRECEDENCE_ASSIGNMENT = 1, // =
    PRECEDENCE_RECORD     = 2, // ,
    PRECEDENCE_LOGICAL    = 3, // and or
    PRECEDENCE_NOT        = 4, // not
    PRECEDENCE_EQUALITY   = 5, // == !=
    PRECEDENCE_COMPARISON = 6, // < > <= >=
    PRECEDENCE_TERM       = 7, // + -
    PRECEDENCE_PRODUCT    = 8, // * / %
    PRECEDENCE_NEGATE     = 9, // -
    PRECEDENCE_CALL       = 10
  };

  Parser::Parselet Parser::expressions_[] = {
    // Punctuators.
    { NULL,             NULL, -1 },                                 // TOKEN_LEFT_PAREN
    { NULL,             NULL, -1 },                                 // TOKEN_RIGHT_PAREN
    { NULL,             NULL, -1 },                                 // TOKEN_LEFT_BRACKET
    { NULL,             NULL, -1 },                                 // TOKEN_RIGHT_BRACKET
    { NULL,             NULL, -1 },                                 // TOKEN_LEFT_BRACE
    { NULL,             NULL, -1 },                                 // TOKEN_RIGHT_BRACE
    { NULL,             &Parser::infixRecord, PRECEDENCE_RECORD },  // TOKEN_COMMA
    { NULL,             NULL, -1 },                                 // TOKEN_EQUALS
    { NULL,             &Parser::binaryOp, PRECEDENCE_TERM },       // TOKEN_PLUS
    { NULL,             &Parser::binaryOp, PRECEDENCE_TERM },       // TOKEN_MINUS
    { NULL,             &Parser::binaryOp, PRECEDENCE_PRODUCT },    // TOKEN_STAR
    { NULL,             &Parser::binaryOp, PRECEDENCE_PRODUCT },    // TOKEN_SLASH
    { NULL,             &Parser::binaryOp, PRECEDENCE_PRODUCT },    // TOKEN_PERCENT
    { NULL,             &Parser::binaryOp, PRECEDENCE_COMPARISON }, // TOKEN_LESS_THAN

    // Keywords.
    { NULL,             &Parser::and_, PRECEDENCE_LOGICAL },         // TOKEN_AND
    { NULL,             NULL, -1 },                                  // TOKEN_CASE
    { NULL,             NULL, -1 },                                  // TOKEN_DEF
    { NULL,             NULL, -1 },                                  // TOKEN_DO
    { NULL,             NULL, -1 },                                  // TOKEN_ELSE
    { NULL,             NULL, -1 },                                  // TOKEN_END
    { &Parser::boolean, NULL, -1 },                                  // TOKEN_FALSE
    { NULL,             NULL, -1 },                                  // TOKEN_FOR
    { NULL,             NULL, -1 },                                  // TOKEN_IF
    { NULL,             NULL, -1 },                                  // TOKEN_IS
    { NULL,             NULL, -1 },                                  // TOKEN_MATCH
    { &Parser::not_,    NULL, -1 },                                  // TOKEN_NOT
    { &Parser::nothing, NULL, -1 },                                  // TOKEN_NOTHING
    { NULL,             &Parser::or_, PRECEDENCE_LOGICAL },          // TOKEN_OR
    { NULL,             NULL, -1 },                                  // TOKEN_RETURN
    { NULL,             NULL, -1 },                                  // TOKEN_THEN
    { &Parser::boolean, NULL, -1 },                                  // TOKEN_TRUE
    { NULL,             NULL, -1 },                                  // TOKEN_VAL
    { NULL,             NULL, -1 },                                  // TOKEN_VAR
    { NULL,             NULL, -1 },                                  // TOKEN_WHILE
    { NULL,             NULL, -1 },                                  // TOKEN_XOR

    { &Parser::record,  NULL, -1 },                                  // TOKEN_FIELD
    { &Parser::name,    &Parser::call, PRECEDENCE_CALL },            // TOKEN_NAME
    { &Parser::number,  NULL, -1 },                                  // TOKEN_NUMBER
    { &Parser::string,  NULL, -1 },                                  // TOKEN_STRING

    { NULL,             NULL, -1 },                                  // TOKEN_LINE
    { NULL,             NULL, -1 },                                  // TOKEN_ERROR
    { NULL,             NULL, -1 }                                   // TOKEN_EOF
  };

  gc<Node> Parser::parseModule()
  {
    Array<gc<Node> > exprs;

    do
    {
      if (lookAhead(TOKEN_EOF)) break;
      exprs.add(statementLike());
    }
    while (match(TOKEN_LINE));

    consume(TOKEN_EOF, "Expected end of file.");
    
    // TODO(bob): Should validate that we are at EOF here.
    return createSequence(exprs);
  }

  gc<Node> Parser::parseBlock(TokenType endToken)
  {
    TokenType dummy;
    return parseBlock(endToken, endToken, &dummy);
  }

  gc<Node> Parser::parseBlock(TokenType end1, TokenType end2,
                              TokenType* outEndToken)
  {
    // If we have a newline, then it's an actual block, otherwise it's a
    // single expression.
    if (match(TOKEN_LINE))
    {
      Array<gc<Node> > exprs;

      do
      {
        if (lookAhead(end1)) break;
        if (lookAhead(end2)) break;
        exprs.add(statementLike());
      }
      while (match(TOKEN_LINE));

      // Return which kind of token we ended the block with, for callers that
      // care.
      *outEndToken = current().type();

      // If the block ends with 'end', then we want to consume that token,
      // otherwise we want to leave it unconsumed to be consistent with the
      // single-expression block case.
      if (current().is(TOKEN_END)) consume();

      return createSequence(exprs);
    }
    else
    {
      // Not a block, so no block end token.
      *outEndToken = TOKEN_EOF;
      return statementLike();
    }
  }

  gc<Node> Parser::statementLike()
  {
    if (match(TOKEN_DEF))
    {
      SourcePos start = last().pos();

      gc<Pattern> leftParam;
      if (match(TOKEN_LEFT_PAREN))
      {
        leftParam = parsePattern();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after pattern.");
      }

      gc<Token> name = consume(TOKEN_NAME,
                               "Expect a method name after 'def'.");

      gc<Pattern> rightParam;
      if (match(TOKEN_LEFT_PAREN))
      {
        if (match(TOKEN_RIGHT_PAREN))
        {
          // Allow () empty pattern to just mean "no parameter".
        }
        else
        {
          rightParam = parsePattern();
          consume(TOKEN_RIGHT_PAREN, "Expect ')' after pattern.");
        }
      }

      gc<Node> body = parseBlock();
      SourcePos span = start.spanTo(last().pos());
      return new DefMethodNode(span, leftParam, name->text(), rightParam, body);
    }

    if (match(TOKEN_DO))
    {
      SourcePos start = last().pos();
      gc<Node> body = parseBlock();
      SourcePos span = start.spanTo(last().pos());
      return new DoNode(span, body);
    }

    if (match(TOKEN_IF))
    {
      SourcePos start = last().pos();

      gc<Node> condition = parseBlock(TOKEN_THEN);
      consume(TOKEN_THEN, "Expect 'then' after 'if' condition.");

      TokenType endToken;
      gc<Node> thenArm = parseBlock(TOKEN_ELSE, TOKEN_END, &endToken);
      gc<Node> elseArm;

      // Don't look for an else arm if the then arm was a block that
      // specifically ended with 'end'.
      if ((endToken != TOKEN_END) && match(TOKEN_ELSE))
      {
        elseArm = parseBlock();
      }

      SourcePos span = start.spanTo(last().pos());
      return new IfNode(span, condition, thenArm, elseArm);
    }

    if (match(TOKEN_RETURN))
    {
      SourcePos start = last().pos();

      // Parse the value if there is one.
      gc<Node> value;
      if (!lookAhead(TOKEN_LINE))
      {
        // TODO(bob): Using statementLike() here is weird. Some stuff makes
        // sense like "return if ...". But "return var..." and
        // "return return..." are pretty crazy. Should probably refine the
        // grammar here.
        value = statementLike();
      }

      SourcePos span = start.spanTo(last().pos());
      return new ReturnNode(span, value);
    }

    if (match(TOKEN_VAR) || match(TOKEN_VAL))
    {
      SourcePos start = last().pos();

      // TODO(bob): Distinguish between var and val.
      bool isMutable = false;

      gc<Pattern> pattern = parsePattern();
      consume(TOKEN_EQUALS, "Expect '=' after variable declaration.");
      // TODO(bob): What precedence? Should also allow "if" here.
      gc<Node> value = parsePrecedence();

      SourcePos span = start.spanTo(last().pos());
      return new VariableNode(span, isMutable, pattern, value);
    }

    return parsePrecedence();
  }

  gc<Node> Parser::parsePrecedence(int precedence)
  {
    gc<Token> token = consume();
    PrefixParseFn prefix = expressions_[token->type()].prefix;

    if (prefix == NULL)
    {
      gc<String> tokenText = token->toString();
      reporter_.error(token->pos(), "Unexpected token '%s'.",
                      tokenText->cString());
      return NULL;
    }

    gc<Node> left = (this->*prefix)(token);

    while (precedence < expressions_[current().type()].precedence)
    {
      token = consume();
      InfixParseFn infix = expressions_[token->type()].infix;
      left = (this->*infix)(left, token);
    }

    return left;
  }

  // Prefix parsers -----------------------------------------------------------

  gc<Node> Parser::boolean(gc<Token> token)
  {
    return new BoolNode(token->pos(), token->type() == TOKEN_TRUE);
  }

  gc<Node> Parser::name(gc<Token> token)
  {
    return call(gc<Node>(), token);
  }

  gc<Node> Parser::not_(gc<Token> token)
  {
    gc<Node> value = parsePrecedence(PRECEDENCE_CALL);
    return new NotNode(token->pos().spanTo(value->pos()), value);
  }

  gc<Node> Parser::nothing(gc<Token> token)
  {
    return new NothingNode(token->pos());
  }

  gc<Node> Parser::number(gc<Token> token)
  {
    double number = atof(token->text()->cString());
    return new NumberNode(token->pos(), number);
  }
  
  gc<Node> Parser::record(gc<Token> token)
  {
    Array<Field> fields;
    
    gc<Node> value = parsePrecedence(PRECEDENCE_LOGICAL);
    fields.add(Field(token->text(), value));
    
    int i = 1;
    while (match(TOKEN_COMMA))
    {
      gc<String> name;
      if (match(TOKEN_FIELD))
      {
        // A named field.
        name = last().text();
      }
      else
      {
        // Infer the name from its position.
        name = String::format("%d", i);
      }
      
      value = parsePrecedence(PRECEDENCE_LOGICAL);
      fields.add(Field(name, value));
      
      i++;
    }
    
    return new RecordNode(token->pos().spanTo(last().pos()), fields);
  }
  
  gc<Node> Parser::string(gc<Token> token)
  {
    return new StringNode(token->pos(), token->text());
  }

  // Infix parsers ------------------------------------------------------------

  gc<Node> Parser::and_(gc<Node> left, gc<Token> token)
  {
    gc<Node> right = parsePrecedence(expressions_[token->type()].precedence);
    return new AndNode(token->pos(), left, right);
  }

  gc<Node> Parser::binaryOp(gc<Node> left, gc<Token> token)
  {
    // TODO(bob): Support right-associative infix. Needs to do precedence
    // - 1 here, to be right-assoc.
    gc<Node> right = parsePrecedence(expressions_[token->type()].precedence);

    return new BinaryOpNode(token->pos(), left, token->type(), right);
  }

  gc<Node> Parser::call(gc<Node> left, gc<Token> token)
  {
    // See if we have an argument on the right.
    bool hasRightArg = false;
    gc<Node> right;
    if (match(TOKEN_LEFT_PAREN))
    {
      hasRightArg = true;
      if (match(TOKEN_RIGHT_PAREN))
      {
        // Allow () to distinguish a call from a name, but don't provide an
        // argument.
      }
      else
      {
        // TODO(bob): Is this right? Do we want to allow variable declarations
        // here?
        right = statementLike();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after call argument.");
      }
    }

    if (left.isNull() && !hasRightArg)
    {
      // Just a bare name.
      return new NameNode(token->pos(), token->text());
    }

    // TODO(bob): Better position.
    return new CallNode(token->pos(), left, token->text(), right);
  }

  gc<Node> Parser::infixRecord(gc<Node> left, gc<Token> token)
  {
    Array<Field> fields;

    // First field is positional.
    gc<String> name = String::create("0");
    fields.add(Field(name, left));
    
    int i = 1;
    do
    {
      gc<String> name;
      if (match(TOKEN_FIELD))
      {
        // A named field.
        name = last().text();
      }
      else
      {
        // Infer the name from its position.
        name = String::format("%d", i);
      }
      
      gc<Node> value = parsePrecedence(PRECEDENCE_LOGICAL);
      fields.add(Field(name, value));
      
      i++;
    }
    while (match(TOKEN_COMMA));
    
    return new RecordNode(left->pos().spanTo(last().pos()), fields);
  }

  gc<Node> Parser::or_(gc<Node> left, gc<Token> token)
  {
    gc<Node> right = parsePrecedence(expressions_[token->type()].precedence);
    return new OrNode(token->pos(), left, right);
  }

  gc<Pattern> Parser::parsePattern()
  {
    if (match(TOKEN_NAME))
    {
      return new VariablePattern(last().pos(), last().text());
    }
    else if (match(TOKEN_NOTHING))
    {
      return new NothingPattern(last().pos());
    }
    else
    {
      reporter_.error(current().pos(), "Expected pattern.");
      return gc<Pattern>();
    }
  }

  gc<Node> Parser::createSequence(const Array<gc<Node> >& exprs)
  {
    // If there is just one expression in the sequence, don't wrap it.
    if (exprs.count() == 1) return exprs[0];

    SourcePos span = exprs[0]->pos().spanTo(exprs[-1]->pos());
    return new SequenceNode(span, exprs);
  }
  
  const Token& Parser::current()
  {
    fillLookAhead(1);
    return *read_[0];
  }

  bool Parser::lookAhead(TokenType type)
  {
    fillLookAhead(1);
    return read_[0]->type() == type;
  }

  bool Parser::lookAhead(TokenType current, TokenType next)
  {
    fillLookAhead(2);
    return read_[0]->is(current) && read_[1]->is(next);
  }

  bool Parser::match(TokenType type)
  {
    if (lookAhead(type))
    {
      consume();
      return true;
    }
    else
    {
      return false;
    }
  }

  void Parser::expect(TokenType expected, const char* errorMessage)
  {
    if (!lookAhead(expected)) reporter_.error(current().pos(), errorMessage);
  }

  gc<Token> Parser::consume()
  {
    fillLookAhead(1);
    last_ = read_.dequeue();
    return last_;
  }

  gc<Token> Parser::consume(TokenType expected, const char* errorMessage)
  {
    if (lookAhead(expected)) return consume();
    reporter_.error(current().pos(), errorMessage);
    
    // Just so that we can keep going and try to find other errors, consume the
    // failed token and proceed.
    return consume();
  }

  void Parser::fillLookAhead(int count)
  {
    while (read_.count() < count) read_.enqueue(lexer_.readToken());
  }
}

