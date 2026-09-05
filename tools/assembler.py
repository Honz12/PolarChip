import struct
from dataclasses import dataclass
from sys import argv
from typing import Dict, List

# --- Command Line Argument Parsing ---

passed = argv[1:]

input_files = []
output_file = "out.bin"

argi = 0
while argi < len(passed):
    a = passed[argi]
    if a == "-o":
        argi += 1
        output_file = passed[argi]
    else:
        input_files.append(a)
    argi += 1

if not input_files:
    print("Usage: python script.py <input_files...> [-o output_file]")
    exit(1)

# --- Instruction Set Schema ---

FORMAT_SPEC = """
NOP
RET
CALL    imd32
INT     imd8
LDI8    imd8    imd8
LDI16   imd8    imd8
LDI24   imd8    imd16
LDI32   imd8    imd32
LD8     imd8    imd32
LD16    imd8    imd32
LD24    imd8    imd32
LD32    imd8    imd32
LDP8    imd8    imd8
LDP16   imd8    imd8
LDP24   imd8    imd8
LDP32   imd8    imd8
ST8     imd8    imd32
ST16    imd8    imd32
ST24    imd8    imd32
ST32    imd8    imd32
STP8    imd8    imd8
STP16   imd8    imd8
STP24   imd8    imd8
STP32   imd8    imd8
"""


@dataclass
class InstructionDef:
    opcode: int
    name: str
    operands: List[str]


# Map operand string identifiers to struct endian/size tokens (Little Endian)
OPERAND_FORMATS = {
    "imd8": "B",  # 1 byte unsigned char
    "imd16": "<H",  # 2 byte unsigned short
    "imd32": "<I",  # 4 byte unsigned int
}

# Parse FORMAT_SPEC into a lookup table indexed by mnemonic
instruction_set: Dict[str, InstructionDef] = {}

for opcode, line in enumerate(
    filter(None, (l.strip() for l in FORMAT_SPEC.splitlines()))
):
    parts = line.split()
    mnemonic = parts[0]
    operands = parts[1:]
    instruction_set[mnemonic] = InstructionDef(
        opcode=opcode, name=mnemonic, operands=operands
    )


def assemble_line(line: str) -> bytes:
    # Strip comments and extra whitespace
    line = line.split(";")[0].strip()
    if not line:
        return b""

    tokens = line.split()
    mnemonic = tokens[0].upper()
    args = tokens[1:]

    if mnemonic not in instruction_set:
        raise ValueError(f"Unknown instruction '{mnemonic}'")

    idef = instruction_set[mnemonic]
    if len(args) != len(idef.operands):
        raise ValueError(
            f"Instruction '{mnemonic}' expects {len(idef.operands)} operands, got {len(args)}"
        )

    # Encode opcode as single byte
    encoded = bytearray([idef.opcode])

    # Encode each operand according to its type
    for arg_str, op_type in zip(args, idef.operands):
        val = int(arg_str, 0)  # Handles decimal, hex (0x..), and binary (0b..)
        fmt = OPERAND_FORMATS[op_type]
        encoded.extend(struct.pack(fmt, val))

    return bytes(encoded)


# --- File Reader & Binary Assembly ---

binary_output = bytearray()

for ifile in input_files:
    with open(ifile, "r", encoding="utf-8") as f:
        for line_num, line in enumerate(f, 1):
            try:
                binary_output.extend(assemble_line(line))
            except Exception as e:
                print(f"Error in {ifile} on line {line_num}: {e}")
                exit(1)

with open(output_file, "wb") as f:
    f.write(binary_output)

print(
    f"Successfully assembled {len(input_files)} file(s) -> {output_file} ({len(binary_output)} bytes)"
)
