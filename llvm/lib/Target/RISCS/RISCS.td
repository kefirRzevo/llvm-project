//===----------------------------------------------------------------------===//
// This is the top level entry point for the RISCS target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// RISCS Subtarget features
//===----------------------------------------------------------------------===//

defvar GRV64 = DefaultMode;

//===----------------------------------------------------------------------===//
// Target-dependent interfaces
//===----------------------------------------------------------------------===//

include "RISCSRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "RISCSSchedule.td"
include "RISCSCallingConv.td"

include "RISCSInstrsInfo.td"
def RISCSInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// RISCS processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def RISCSAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
  let AllowDuplicateRegisterNames = 1;
}

def RISCS : Target {
  let InstructionSet = RISCSInstrInfo;
}
