package com.stuffwithstuff.magpie.interpreter;

import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import com.stuffwithstuff.magpie.ast.pattern.Pattern;
import com.stuffwithstuff.magpie.ast.pattern.PatternVisitor;
import com.stuffwithstuff.magpie.ast.pattern.RecordPattern;
import com.stuffwithstuff.magpie.ast.pattern.TypePattern;
import com.stuffwithstuff.magpie.ast.pattern.ValuePattern;
import com.stuffwithstuff.magpie.ast.pattern.VariablePattern;
import com.stuffwithstuff.magpie.ast.pattern.WildcardPattern;

/**
 * Compares two methods to see which takes precedence over the other. It is
 * important that both methods be applicable to some single argument type or
 * ambiguous method errors may result. For example, two unrelated classes like
 * Int and Bool can't be linearized since there is no relationship between them.
 * However, by restricting this to applicable methods, that case should be
 * avoided since there's no single argument value for which both of those is
 * applicable.
 * 
 * The basic linearization process is relatively simple. First we determine the
 * kind of the two methods. There are four kinds that matter:
 * - Value patterns
 * - Nominal (i.e. class) type patterns
 * - Structural (i.e. record or tuple) type patterns
 * - Any (i.e. wildcard or simple variable name) patterns
 * 
 * Earlier kinds take precedence of later kinds in the above list, so a method
 * specialized to a value will always win over one specialized to a class.
 * 
 * If comparing kinds doesn't yield an ordering, we compare within kind. For
 * value or any patterns, it is an error to have multiple methods with that
 * kind and an error is raised. For type patterns, we then compare the types.
 * 
 * Classes are linearized based on their inheritance tree. Subclasses take
 * precedence over superclasses. Later siblings take precedence over earlier
 * ones. (If D inherits from A and B, in that order, B takes precedence over A.)
 * Since class hierarchies are strict trees, this is enough to order two
 * classes.
 * 
 * With structural types, their fields are linearized. If all fields sort the
 * same way (or are the same) then the type with the winning fields wins. For
 * example, (Derived, Int) beats (Base, Int).
 * 
 * TODO(bob): Note that only a fraction of the above is currently implemented.
 * It's getting there...
 * TODO(bob): What about OrTypes, function types, generics, and user-defined
 * types?
 */
public class MethodLinearizer implements Comparator<Callable> {
  public MethodLinearizer(Interpreter interpreter) {
    mInterpreter = interpreter;
  }

  @Override
  public int compare(Callable method1, Callable method2) {
    return compare(method1.getPattern(), new EvalContext(method1.getClosure()),
                   method2.getPattern(), new EvalContext(method2.getClosure()));
  }

  private int compare(Pattern pattern1, EvalContext context1,
      Pattern pattern2, EvalContext context2) {
    final int firstWins = -1;
    final int secondWins = 1;
    
    pattern1 = pattern1.accept(new PatternSimplifier(), null);
    pattern2 = pattern2.accept(new PatternSimplifier(), null);
    
    PatternKind kind1 = pattern1.accept(new PatternKinder(), null);
    PatternKind kind2 = pattern2.accept(new PatternKinder(), null);
    
    // Ironically, this bit of code would really benefit from multiple dispatch.
    switch (kind1) {
    case ANY:
      switch (kind2) {
      case ANY:     return 0;
      case RECORD:  return secondWins;
      case TYPE:    return secondWins;
      case VALUE:   return secondWins;
      default:
        throw new UnsupportedOperationException("Unknown pattern kind.");
      }
    case RECORD:
      switch (kind2) {
      case ANY:     return firstWins;
      case RECORD:  return compareRecords(pattern1, context1, pattern2, context2);
      case TYPE:    return compareTypes(pattern1, context1, pattern2, context2);
      case VALUE:   return secondWins;
      default:
        throw new UnsupportedOperationException("Unknown pattern kind.");
      }
    case TYPE:
      switch (kind2) {
      case ANY:     return firstWins;
      case RECORD:  return compareTypes(pattern1, context1, pattern2, context2);
      case TYPE:    return compareTypes(pattern1, context1, pattern2, context2);
      case VALUE:   return secondWins;
      default:
        throw new UnsupportedOperationException("Unknown pattern kind.");
      }
    case VALUE:
      switch (kind2) {
      case ANY:     return firstWins;
      case RECORD:  return firstWins;
      case TYPE:    return firstWins;
      case VALUE:   return compareValues(pattern1, context1, pattern2, context2);
      default:
        throw new UnsupportedOperationException("Unknown pattern kind.");
      }
    default:
      throw new UnsupportedOperationException("Unknown pattern kind.");
    }
  }

  private Set<String> intersect(Set<String> a, Set<String> b) {
    Set<String> intersect = new HashSet<String>();
    for (String field : a) {
      if (b.contains(field)) intersect.add(field);
    }
    
    return intersect;
  }

  /**
   * Returns true if a contains elements that are not in b.
   */
  private boolean containsOthers(Set<String> a, Set<String> b) {
    for (String field : a) {
      if (!b.contains(field)) return true;
    }
    
    return false;
  }
  
  private int compareRecords(Pattern pattern1, EvalContext context1,
      Pattern pattern2, EvalContext context2) {
    Map<String, Pattern> record1 = ((RecordPattern)pattern1).getFields();
    Map<String, Pattern> record2 = ((RecordPattern)pattern2).getFields();
    
    // Take the intersection of their fields.
    Set<String> intersect = intersect(record1.keySet(), record2.keySet());
    
    // If the records don't have the same number of fields, one must be a
    // strict superset of the other.
    if ((record1.size() != intersect.size()) ||
        (record2.size() != intersect.size())) {
      if (containsOthers(record1.keySet(), record2.keySet()) &&
          containsOthers(record2.keySet(), record1.keySet())) {
        throw ambiguous(pattern1, pattern2);
      }
    }

    // Fields that are common to the two cannot disagree on sort order.
    int currentComparison = 0;
    for (String name : intersect) {
      Pattern field1 = record1.get(name);
      Pattern field2 = record2.get(name);
      
      int compare = compare(field1, context1, field2, context2);
      
      if (currentComparison == 0) {
        currentComparison = compare;
      } else if (compare == 0) {
        // Do nothing.
      } else if (compare != currentComparison) {
        // If we get here, the fields don't agree.
        throw ambiguous(pattern1, pattern2);
      }
    }
    
    return currentComparison;
  }
  
  private int compareTypes(Pattern pattern1, EvalContext context1,
      Pattern pattern2, EvalContext context2) {
    Obj type1 = mInterpreter.evaluate(PatternTyper.evaluate(pattern1), context1);
    Obj type2 = mInterpreter.evaluate(PatternTyper.evaluate(pattern2), context2);
    
    // TODO(bob): WIP getting rid of types.
    if (type1 instanceof ClassObj && type2 instanceof ClassObj) {
      ClassObj class1 = (ClassObj)type1;
      ClassObj class2 = (ClassObj)type2;
      
      // Same class.
      if (class1 == class2) return 0;
      
      if (class1.isSubclassOf(class2)) {
        // Class1 is a subclass, so it's more specific.
        return -1;
      } else if (class2.isSubclassOf(class1)) {
        // Class2 is a subclass, so it's more specific.
        return 1;
      } else {
        // No class relation between the two, so they can't be linearized.
        throw ambiguous(pattern1, pattern2);
      }
    }
    
    throw new UnsupportedOperationException("Must be class now!");
  }

  private int compareValues(Pattern pattern1, EvalContext context1,
      Pattern pattern2, EvalContext context2) {
    Obj value1 = mInterpreter.evaluate(((ValuePattern)pattern1).getValue(), context1);
    Obj value2 = mInterpreter.evaluate(((ValuePattern)pattern2).getValue(), context2);
    
    // Identical values are ordered the same. This lets us have tuples with
    // some identical value fields (like nothing) which are then sorted by
    // other fields.
    if (mInterpreter.objectsEqual(value1, value2)) return 0;
    
    // Any other paid of values can't be sorted.
    throw ambiguous(pattern1, pattern2);
  }
  
  private ErrorException ambiguous(Pattern pattern1, Pattern pattern2) {
    return mInterpreter.error(Name.AMBIGUOUS_METHOD_ERROR, 
        "Cannot choose a method between " + pattern1 + " and " + pattern2);
  }

  private enum PatternKind {
    ANY,
    RECORD,
    TYPE,
    VALUE
  }
  
  /**
   * Removes all variable patterns from a pattern since the linearizer doesn't
   * care about them.
   */
  private static class PatternSimplifier implements PatternVisitor<Pattern, Void> {
    @Override
    public Pattern visit(RecordPattern pattern, Void dummy) {
      Map<String, Pattern> fields = new HashMap<String, Pattern>();
      for (Entry<String, Pattern> field : pattern.getFields().entrySet()) {
        fields.put(field.getKey(), field.getValue().accept(this, null));
      }
      
      return Pattern.record(fields);
    }
    
    @Override
    public Pattern visit(TypePattern pattern, Void dummy) {
      return pattern;
    }

    @Override
    public Pattern visit(ValuePattern pattern, Void dummy) {
      return pattern;
    }

    @Override
    public Pattern visit(VariablePattern pattern, Void dummy) {
      return pattern.getPattern().accept(this, null);
    }

    @Override
    public Pattern visit(WildcardPattern pattern, Void dummy) {
      return pattern;
    }
  }
  
  private static class PatternKinder implements PatternVisitor<PatternKind, Void> {
    @Override
    public PatternKind visit(RecordPattern pattern, Void dummy) {
      return PatternKind.RECORD;
    }
    
    @Override
    public PatternKind visit(TypePattern pattern, Void dummy) {
      return PatternKind.TYPE;
    }

    @Override
    public PatternKind visit(ValuePattern pattern, Void dummy) {
      return PatternKind.VALUE;
    }

    @Override
    public PatternKind visit(VariablePattern pattern, Void dummy) {
      return pattern.getPattern().accept(this, null);
    }

    @Override
    public PatternKind visit(WildcardPattern pattern, Void dummy) {
      return PatternKind.ANY;
    }
  }
  
  private final Interpreter mInterpreter;
}
