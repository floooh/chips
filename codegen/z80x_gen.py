import yaml, pprint, copy

# these are essentially opcode patterns
OP_PATTERNS = []
# these are stamped out opcode descriptions
OPS = [None for i in range(0,256)]

# register mapping tables, see: http://www.z80.info/decoding.htm
r_human = [ 'B', 'C', 'D', 'E', 'H', 'L', '(HL)', 'A' ]
rp_human = [ 'BC', 'DE', 'HL', 'SP' ]
rp2_human = [ 'BC', 'DE', 'HL', 'AF' ]
alu_human = [ 'ADD', 'ADC', 'SUB', 'SBC', 'AND', 'XOR', 'OR', 'CP' ]
cc_human = [ 'NZ', 'Z', 'NC', 'C', 'PO', 'PE', 'P', 'M' ]

def err(msg: str):
    raise BaseException(msg)

class MCycle:
    def __init__(self, type: str, tcycles: int, items: list[str]):
        self.type = type
        self.tcycles = tcycles
        self.items = items

class Op:
    def __init__(self, name:str, cond:str):
        self.name = name
        self.cond = cond
        self.cond_compiled = compile(cond, '<string>', 'eval')
        self.mcycles: list[MCycle] = []

def map_regs_human(inp: str, y: int, z: int, p: int, q:int) -> str:
    return inp\
        .replace('r[y]', r_human[y])\
        .replace('r[z]', r_human[z])\
        .replace('alu[y]', alu_human[y])\
        .replace('rp[p]', rp_human[p])\
        .replace('rp2[p]', rp2_human[p])\
        .replace('cc[y-4]', cc_human[y-4])\
        .replace('cc[y]', cc_human[y])\
        .replace('y*8', f'{y*8:X}h')

def parse_mcycle_name(name: str) -> tuple[str, int]:
    tokens = name.replace('[',' ').replace(']',' ').split(' ')
    if len(tokens) not in [1,3]:
        err(f'invalid mcycle syntax: {name}')
    type = tokens[0]
    if type not in ['fetch', 'mread', 'mwrite', 'ioread', 'iowrite', 'generic', 'overlapped']:
        err(f'invalid mcycle type: {type}')
    tcycles = 0
    if len(tokens) == 1:
        # standard machine cycle lengths
        if type == 'fetch':
            # not a bug (because of overlapped execute/fetch)
            tcycles = 2
        elif type in ['mread', 'mwrite']:
            tcycles = 3
        elif type in ['ioread', 'iowrite']:
            tcycles = 4
        elif type == 'generic':
            err(f'generic mcycles must have an explicit length')
        elif type == 'overlapped':
            tcycles = 2
    else:
        tcycles = int(tokens[1])
        if type == 'fetch':
            # fix overlapped fetch tcycle length 
            tcycles -= 2
    if (tcycles < 1) or (tcycles > 6):
        err(f'invalid mcycle len: {tcycles}')
    return type, tcycles

def parse_mc_items(mc_desc: str):
    res: list[str] = []
    if mc_desc is not None:
        tokens = mc_desc.split(',')
        res: list[str] = []
        for token in tokens:
            if (token != ''):
                res.append(token.strip())
    return res

def parse_opdescs():
    with open('z80_desc.yml') as fp:
        desc = yaml.load(fp, Loader=yaml.FullLoader)
        for (op_name, op_desc) in desc.items():
            if 'cond' not in op_desc:
                err(f"op '{op_name}'' has no condition!")
            op = Op(op_name,op_desc['cond'])
            num_fetch = 0
            num_overlapped = 0
            for (mc_name, mc_desc) in op_desc.items():
                if mc_name != 'cond':
                    mc_type, mc_tcycles = parse_mcycle_name(mc_name)
                    if mc_type == 'fetch':
                        num_fetch += 1
                    elif mc_type == 'overlapped':
                        num_overlapped += 1
                    mc = MCycle(mc_type, mc_tcycles, parse_mc_items(mc_desc))
                    op.mcycles.append(mc)
            if num_fetch == 0:
                op.mcycles.insert(0, MCycle('fetch', 2, []))
            if num_overlapped == 0:
                op.mcycles.append(MCycle('overlapped', 2, []))
            OP_PATTERNS.append(op)

def expand_ops():
    for opcode in range(0,256):
        #  76 543 210
        # |xx|yyy|zzz|
        #    |ppq|
        x = opcode >> 6
        y = (opcode >> 3) & 7
        z = opcode & 7
        p = y >> 1
        q = y & 1
        for op_desc_index,op_desc in enumerate(OP_PATTERNS):
            if eval(op_desc.cond_compiled):
                if OPS[opcode] is not None:
                    err(f"Condition collision for opcode {op_desc_index:02X} and '{op_desc.name}'")
                op = copy.deepcopy(OP_PATTERNS[op_desc_index])
                op.name = map_regs_human(op.name, y, z, p, q)
                OPS[opcode] = op

def dump():
    for op_desc in OP_PATTERNS:
        print(f"\nop(name='{op_desc.name}', cond='{op_desc.cond}'):")
        for mc in op_desc.mcycles:
            print(f'  mcycle(type={mc.type}, tcycles={mc.tcycles})')
            for item in mc.items:
                print(f'    {item}')
    print('\n')
    for i,op in enumerate(OPS):
        op_name = '??'
        if op is not None:
            op_name = op.name
        print(f"{i:02X}: {op_name}")

if __name__=='__main__':
    parse_opdescs()
    expand_ops()
    dump()
