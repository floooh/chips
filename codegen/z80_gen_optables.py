import yaml, copy

DESC_PATH  = 'z80_desc.yml'

# a machine cycle description
class MCycle:
    def __init__(self, type, tcycles, items):
        self.type = type
        self.tcycles = tcycles
        self.items = items

# an opcode description
class Op:
    def __init__(self, name, cond, flags):
        self.name = name
        self.cond = cond
        self.cond_compiled = compile(cond, '<string>', 'eval')
        self.flags = flags
        self.opcode = -1
        self.prefix = ''
        self.single = False
        self.special_timing = False

OP_PATTERNS = []

# 0..255:   core opcodes
# 256..511: ED prefix opcodes
# 512..514: special decoder blocks for CB-prefix
# 515..519: special decoder blocks for interrupt handling
OPS = [None for _ in range(0,3*256)]

# register mapping tables, see: http://www.z80.info/decoding.htm
r_comment   = [ 'B', 'C', 'D', 'E', 'H', 'L', '(HL)', 'A' ]
rp_comment  = [ 'BC', 'DE', 'HL', 'SP' ]
rp2_comment = [ 'BC', 'DE', 'HL', 'AF' ]
alu_comment = [ 'ADD', 'ADC', 'SUB', 'SBC', 'AND', 'XOR', 'OR', 'CP' ]
rot_comment = [ 'RLC', 'RRC', 'RL', 'RR', 'SLA', 'SRA', 'SLL', 'SRL' ]
cc_comment  = [ 'NZ', 'Z', 'NC', 'C', 'PO', 'PE', 'P', 'M' ]
im_map = [ '0', '0', '1', '2', '0', '0', '1', '2' ]

def err(msg: str):
    raise BaseException(msg)

def map_comment(inp, y, z, p, q):
    return inp\
        .replace('$RY', r_comment[y])\
        .replace('$RZ', r_comment[z])\
        .replace('$ALU', alu_comment[y])\
        .replace('$ROT', rot_comment[y])\
        .replace('$RP2', rp2_comment[p])\
        .replace('$RP', rp_comment[p])\
        .replace('$CC-4', cc_comment[y-4])\
        .replace('$CC', cc_comment[y])\
        .replace('$IMY', im_map[y])\
        .replace('$Y*8', f'{y*8:X}h')\
        .replace('$Y', f'{y}')

def flag(op, flag):
    if flag in op.flags:
        return op.flags[flag]
    else:
        return False

def parse_opdescs():
    with open(DESC_PATH, 'r') as fp:
        desc = yaml.load(fp, Loader=yaml.FullLoader) # type: ignore
        for (op_name, op_desc) in desc.items():
            if 'cond' not in op_desc:
                op_desc['cond'] = 'True'
            flags = {}
            if 'flags' in op_desc:
                flags = op_desc['flags']
            op = Op(op_name,op_desc['cond'], flags)
            if 'prefix' in op_desc:
                op.prefix = op_desc['prefix']
            # check if this instruction has extra cycles
            for mc_desc in op_desc['mcycles']:
                if 'tcycles' in mc_desc:
                    op.special_timing = True
            OP_PATTERNS.append(op)

def find_opdesc(name):
    for op_desc in OP_PATTERNS:
        if op_desc.name == name:
            return op_desc
    return None

def stampout_op(prefix, opcode, op_index, op_desc):
    #  76 543 210
    # |xx|yyy|zzz|
    #    |ppq|
    y = (opcode >> 3) & 7
    z = opcode & 7
    p = y >> 1
    q = y & 1
    op = copy.deepcopy(op_desc)
    op.name = map_comment(op.name, y, z, p, q)
    op.prefix = prefix
    op.opcode = opcode
    OPS[op_index] = op

def expand_optable():
    for oprange,prefix in enumerate(['', 'ed']):
        for opcode in range(0,256):
            x = opcode >> 6 # type: ignore (generated unused warning, but x is needed in 'eval')
            y = (opcode >> 3) & 7 
            z = opcode & 7 # type: ignore
            p = y >> 1 # type: ignore
            q = y & 1 # type: ignore
            op_index = oprange * 256 + opcode
            for op_desc_index,op_desc in enumerate(OP_PATTERNS):
                if not flag(op_desc, 'special'):
                    if eval(op_desc.cond_compiled) and op_desc.prefix == prefix:
                        if OPS[op_index] is not None:
                            err(f"Condition collission for opcode {op_desc_index:02X} and '{op_desc.name}'")
                        stampout_op(prefix, opcode, op_index, op_desc)
    # add CB instruction block
    for opcode in range(0,256):
        x = opcode >> 6 # type: ignore (generated unused warning, but x is needed in 'eval')
        y = (opcode >> 3) & 7 
        z = opcode & 7 # type: ignore
        p = y >> 1 # type: ignore
        q = y & 1 # type: ignore
        op_index = 2 * 256 + opcode

        if x == 0:
            op_name = map_comment("$ROT $RZ", y, z, p, q)
        elif x == 1:
            op_name = map_comment(f"BIT {y},$RZ", y, z, p, q)
        elif x == 2:
            op_name = map_comment(f"RES {y},$RZ", y, z, p, q)
        elif x == 3:
            op_name = map_comment(f"SET {y},$RZ", y, z, p, q)
        OPS[op_index] = Op(op_name, "True", {})
        if z == 6:
            OPS[op_index].special_timing = True

# generate code for one op
def write_table(offset, x):
    font="font-size:80%;font-weight:bold;"
    border = "border:1px solid black;border-collapse:collapse;padding:5px;"
    default_color = "color:black;background-color:PaleGreen;"
    special_color = "color:black;background-color:LightPink;"
    hdr_color = "color:black;background-color:Gainsboro"
    print('<style>')
    print(f'.z80t {{ {border} }}')
    print(f'.z80h {{ {border}{hdr_color} }}')
    print(f'.z80c0 {{ {border}{font}{default_color} }}')
    print(f'.z80c1 {{ {border}{font}{special_color} }}')
    print('</style>')
    print(f'<table class="z80t">')
    # header
    print(f'<tr class="z80t">', end='')
    print(f'<th class="z80h">x={x:02b}</th>', end='')
    for z in range(0,8):
        print(f'<th class="z80h">z={z:03b}</th>', end='')
    print("</tr>", end='')
    for y in range(0,8):
        print(f'<tr class="z80t">', end='')
        print(f'<th class="z80h">y={y:03b}</th>', end='')
        for z in range(0,8):
            op_index = ((x<<6) | (y<<3) | (z)) + offset
            op = OPS[op_index]
            print(f'<td class="{"z80c1" if op.special_timing else "z80c0"}">{op.name}</td>', end='')
        print("</tr>", end='')
    print("\n</table><br>")

if __name__=='__main__':
    parse_opdescs()
    expand_optable()
    for x in range(0,4):
         write_table(512, x)
