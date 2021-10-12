import yaml, pprint, copy

# these are essentially opcode patterns
OP_PATTERNS = []
# these are stamped out opcode descriptions
OPS = [None for i in range(0,256)]

# a fetch machine cycle is processed as 2 parts because it overlaps
# with the 'action' of the previous instruction
FETCH_TCYCLES = 3
OVERLAPPED_TCYCLES = 1
MEM_TCYCLES = 3
IO_TCYCLES = 4
IO_TCYCLES = 4

# register mapping tables, see: http://www.z80.info/decoding.htm
r_human = [ 'B', 'C', 'D', 'E', 'H', 'L', '(HL)', 'A' ]
rp_human = [ 'BC', 'DE', 'HL', 'SP' ]
rp2_human = [ 'BC', 'DE', 'HL', 'AF' ]
alu_human = [ 'ADD', 'ADC', 'SUB', 'SBC', 'AND', 'XOR', 'OR', 'CP' ]
cc_human = [ 'NZ', 'Z', 'NC', 'C', 'PO', 'PE', 'P', 'M' ]

def err(msg: str):
    raise BaseException(msg)

# append a source code line
out_lines = ''
def l(s) :
    global out_lines
    out_lines += s + '\n'

# a machine cycle description
class MCycle:
    def __init__(self, type: str, tcycles: int, items: list[str]):
        self.type = type
        self.tcycles = tcycles
        self.items = items

# an opcode description
class Op:
    def __init__(self, name:str, cond:str):
        self.name = name
        self.cond = cond
        self.cond_compiled = compile(cond, '<string>', 'eval')
        self.opcode = -1
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
            tcycles = FETCH_TCYCLES
        elif type in ['mread', 'mwrite']:
            tcycles = MEM_TCYCLES
        elif type in ['ioread', 'iowrite']:
            tcycles = IO_TCYCLES
        elif type == 'generic':
            err(f'generic mcycles must have an explicit length')
        elif type == 'overlapped':
            tcycles = OVERLAPPED_TCYCLES
    else:
        tcycles = int(tokens[1])
        if type == 'fetch':
            # fix overlapped fetch tcycle length 
            tcycles -= OVERLAPPED_TCYCLES
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
                op.mcycles.insert(0, MCycle('fetch', FETCH_TCYCLES, []))
            if num_overlapped == 0:
                op.mcycles.append(MCycle('overlapped', OVERLAPPED_TCYCLES, []))
            OP_PATTERNS.append(op)

def expand_optable():
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
                op.opcode = opcode
                OPS[opcode] = op

# build the execution pipeline bitmask for a given machine cycle
def build_pip(mcycle: MCycle) -> int:
    pip = 0

    def tcycles(tc: int, bits: list[int], total: int) -> int:
        pos = 0
        for pos in range(0,total):
            if pos < len(bits):
                if bits[pos] != 0:
                    tc |=  (1<<pos)
        # this triggers the next machine cycle
        tc |= (1<<total)
        if tc >= 256:
            err('tcycle overflow!')
        return tc

    def sample_wait(tc: int, pos: int) -> int:
        return (tc | (1<<(pos + 16)))

    def loadir(tc: int) -> int:
        return (tc |(1<<9))
        
    if mcycle.type == 'fetch':
        # 3-cycle 'part-fetch' because of overlapped execute/fetch
        pip = tcycles(pip, [1,1], mcycle.tcycles)
    elif mcycle.type == 'mread':
        # memory read cycle needs one tcycle to initiate read, and next tcycle to store result
        pip = tcycles(pip, [1,1,0], mcycle.tcycles)
        pip = sample_wait(pip, 1)
    elif mcycle.type == 'mwrite':
        # memory write 
        pip = tcycles(pip, [1,1,0], mcycle.tcycles)
        pip = sample_wait(pip, 1)
    elif mcycle.type == 'ioread':
        pip = tcycles(pip, [1,1,1,0], mcycle.tcycles)
        pip = sample_wait(pip, 2)
    elif mcycle.type == 'iowrite':
        # same as memory write, but one extra cycle
        pip = tcycles(pip, [1,1,0,0], mcycle.tcycles)
        pip = sample_wait(pip, 2)
    elif mcycle.type == 'generic':
        pip = tcycles(pip, [1], mcycle.tcycles)
    elif mcycle.type == 'overlapped':
        pip = tcycles(pip, [1], mcycle.tcycles)
        pip = sample_wait(pip, 1)
        pip = loadir(pip)
    return pip

# generate code for one op
def gen_decoder():
    for op in OPS:

        step = 0
        action = ''

        def add(action):
            nonlocal step
            l(f'case 0x{op.opcode:02X}|(1<<{step}): {action} break;')
            step += 1

        for i,mcycle in enumerate(op.mcycles):
            # execution mask for the current machine cycle
            pip = build_pip(mcycle)
            if mcycle.type == 'fetch':
                l(f'// {op.name}: M1')
            elif mcycle.type == 'overlapped':
                l(f'// {op.name}: OVERLAP')
            else:
                l(f'// {op.name}: M{i+1}')
            if mcycle.type == 'fetch':
                # initialize next tcycle mask
                add(f'c->pip=0x{pip:X}/*{pip:08b}*/')
                # refresh cycle
                add('_RFSH();')
            elif mcycle.type == 'mread':
                add(f'c->pip=0x{pip:X}/*{pip:08b}*/;_SAX(0xFFFF,Z80_MREQ|Z80_RD);/*FIXME: address!*/')
                add('/*FIXME: read data bus*/')
            elif mcycle.type == 'write':
                add(f'c->pip=0x{pip:X}/*{pip:08b}*/')
                add('_SADX(0xFFFF,0xFF,Z80_MREQ|Z80_WR);/*FIXME: address and data!*/')
            elif mcycle.type == 'ioread':
                add(f'c->pip=0x{pip:X}/*{pip:08b}*/;')
                add('_SAX(0xFFFF,Z80_IOREQ|Z80_RD);/*FIXME: address!*/')
                add('/*FIXME: read data bus*/')
            elif mcycle.type == 'iowrite':
                add(f'c->pip=0x{pip:X}/*{pip:08b}*/;')
                add('_SADX(0xFFFF,0xFF,Z80_IORQ|Z80_WR);/*FIXME: address and data!*/')
            elif mcycle.type == 'generic':
                add(f'c->pip=0x{pip:X}/*{pip:08b}*/;/*FIXME: action*/')
            elif mcycle.type == 'overlapped':
                add(f'c->pip=0x{pip:X}/*{pip:08b}*/;_FETCH();/*FIXME: overlapped action!*/')

def dump():
    for op_desc in OP_PATTERNS:
        print(f"\nop(name='{op_desc.name}', cond='{op_desc.cond}'):")
        for mc in op_desc.mcycles:
            print(f'  mcycle(type={mc.type}, tcycles={mc.tcycles})')
            for item in mc.items:
                print(f'    {item}')
    print('\n')
    for op in OPS:
        op_name = '??'
        if op is not None:
            op_name = op.name
        print(f"{op.opcode:02X}: {op_name}")

    print(out_lines)

if __name__=='__main__':
    parse_opdescs()
    expand_optable()
    gen_decoder()
    dump()
