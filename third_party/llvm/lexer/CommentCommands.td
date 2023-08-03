//===----------------------------------------------------------------------===//
// Define command classes.
//===----------------------------------------------------------------------===//

class Command<string name> {
  string Name = name;

  // Global directives
  bit IsExportCommand = 0;

  // Function directives
  bit IsParamCommand = 0;
  bit IsExitCommand = 0;
  bit IsContractCommand = 0;

  list<string> EquivalentDoxygenCommands = [];
}

def Headers : Command<"headers"> {
  let IsExportCommand = 1;
}

def Modules : Command<"modules"> {
  let IsExportCommand = 1;
}

def Param : Command<"param"> {
  let IsParamCommand = 1;
}

def Returns : Command<"returns"> {
  let IsExitCommand = 1;
  let EquivalentDoxygenCommands = ["return", "result", "retval"];
}

def Pre : Command<"pre"> {
  let IsContractCommand = 1;
}

def Post : Command<"post"> {
  let IsContractCommand = 1;
}

def Throws : Command<"throws"> {
  let IsExitCommand = 1;
  let EquivalentDoxygenCommands = ["throw", "exception"];
}

def ExitsVia : Command<"exits-via"> {
  let IsExitCommand = 1;
}
