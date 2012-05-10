#include "VM.h"
#include "Module.h"
#include "Object.h"
#include "Primitives.h"

#define DEF_PRIMITIVE(name, signature) \
        methods_.define(String::create(signature), name##Primitive); \

namespace magpie
{
  VM::VM()
  : modules_(),
    recordTypes_(),
    fiber_()
  {
    Memory::initialize(this, 1024 * 1024 * 2); // TODO(bob): Use non-magic number.
    
    fiber_ = new Fiber(*this);
    
    DEF_PRIMITIVE(print, "print()");
    
    true_ = new BoolObject(true);
    false_ = new BoolObject(false);
    nothing_ = new NothingObject();
  }

  void VM::loadModule(Module* module)
  {
    modules_.add(module);
    fiber_->init(module->body());
    
    while (true)
    {
      gc<Object> result = fiber_->run();
      
      // If the fiber returns null, it's still running but it did a GC run.
      // Since that moves the fiber, we return back to here so we can invoke
      // run() again at its new location in memory.
      if (!result.isNull()) return;
    }
  }

  void VM::reachRoots()
  {
    methods_.reach();
    Memory::reach(recordTypes_);
    Memory::reach(fiber_);
    Memory::reach(true_);
    Memory::reach(false_);
    Memory::reach(nothing_);
    
    for (int i = 0; i < modules_.count(); i++)
    {
      modules_[i]->reach();
    }
  }
  
  int VM::addRecordType(gc<String> signature)
  {
    // TODO(bob): Should use a hash table or something more optimal.
    // See if we already have a type for this signature.
    for (int i = 0; i < recordTypes_.count(); i++)
    {
      if (recordTypes_[i]->signature() == signature) return i;
    }
    
    // It's a new type, so add it.
    gc<RecordType> type = new RecordType(signature);
    recordTypes_.add(type);
    return recordTypes_.count() - 1;
  }

  gc<RecordType> VM::getRecordType(int id)
  {
    return recordTypes_[id];
  }
}

