package com.stuffwithstuff.magpie.interpreter;

import com.stuffwithstuff.magpie.ast.FnExpr;
import com.stuffwithstuff.magpie.ast.pattern.Pattern;

/**
 * Wraps a raw FnExpr in the data and logic needed to execute a user-defined
 * function given the context to do it in.
 */
public class Function implements Callable {
  public Function(Scope closure, FnExpr function) {
    mClosure = closure;
    mFunction = function;
  }

  public Scope getClosure() { return mClosure; }
  public FnExpr getFunction() { return mFunction; }
  
  @Override
  public Obj invoke(Interpreter interpreter, Obj thisObj, Obj arg) {
    try {
      Profiler.push(mFunction.getPosition());
      
      // Create a local scope for the function with the arguments bounds to
      // the pattern.
      Pattern pattern = mFunction.getType().getPattern();
      EvalContext context = PatternBinder.bind(interpreter, pattern, arg, 
          new EvalContext(mClosure, thisObj));
      
      try {
        return interpreter.evaluate(mFunction.getBody(), context);
      } catch (ReturnException ex) {
        // There was an early return in the function, so return the value of that.
        return ex.getValue();
      }
    } finally {
      Profiler.pop();
    }
  }
  
  @Override
  public Obj getType(Interpreter interpreter) {
    // Evaluate it in the context of its closure so that any static arguments
    // defined in the surrounding scope are available for use in this type
    // annotation.
    EvalContext context = new EvalContext(mClosure, interpreter.nothing());
   
    Obj type = interpreter.evaluateFunctionType(mFunction.getType(), context);

    // TODO(bob): Hackish. Cram the original function body in there too. That
    // way, if it's a static function, we have access to it during check time.
    type.setValue(mFunction);
    
    return type;
  }
  
  private final Scope mClosure;
  private final FnExpr mFunction;
}
