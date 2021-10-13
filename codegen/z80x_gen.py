import yaml, copy
from typing import Optional, TypeVar

# a machine cycle description
class MCycle:
    def __init__(self, type: str, tcycles: int, items: dict[str,str]):
        self.type = type
        self.tcycles = tcycles
        self.items = items

# an opcode description
class Op:
    def __init__(self, name:str, cond:str):
        self.name = name
        self.cond = cond
        self.cond_compiled = compile(cond, '<string>', 'eval')
        self.opcode: int = -1
        self.mcycles: list[MCycle] = []

# these are essentially opcode patterns
OP_PATTERNS: list[Op] = []
# these are stamped out opcode descriptions
OPS: list[Optional[Op]] = [None for _ in range(0,256)]

# a fetch machine cycle is processed as 2 parts because it overlaps
# with the 'action' of the previous instruction
FETCH_TCYCLES = 3
OVERLAPPED_TCYCLES = 1
MEM_TCYCLES = 3
IO_TCYCLES = 4

# register mapping tables, see: http://www.z80.info/decoding.htm
r_comment   = [ 'B', 'C', 'D', 'E', 'H', 'L', '(HL)', 'A' ]
rp_comment  = [ 'BC', 'DE', 'HL', 'SP' ]
rp2_comment = [ 'BC', 'DE', 'HL', 'AF' ]
alu_comment = [ 'ADD', 'ADC', 'SUB', 'SBC', 'AND', 'XOR', 'OR', 'CP' ]
cc_comment  = [ 'NZ', 'Z', 'NC', 'C', 'PO', 'PE', 'P', 'M' ]

r_set   = [ 'cpu->b=_X_', 'cpu->c=_X_', 'cpu->d=_X_', 'cpu->e=_X_', 'cpu->h=_X_', 'cpu->l=_X_', 'XXX', 'cpu->a=_X_' ]
rp_set  = [ '_sbc(_X_)', '_sde(_X_)', '_shl(_X_)', '_ssp(_X_)' ]
rp2_set = [ '_sbc(_X_)', '_sde(_X_)', '_shl(_X_)', '_saf(_X_)' ]

r_get   = [ 'cpu->b', 'cpu->c', 'cpu->d', 'cpu->e', 'cpu->h', 'cpu->l', 'XXX', 'cpu->a' ]
rp_get  = [ 'bc', 'de', 'hl', 'sp' ]
rp2_get = [ 'bc', 'de', 'hl', 'af' ]

def err(msg: str):
    raise BaseException(msg)

T = TypeVar('T')
def unwrap(maybe_value: Optional[T]) -> T:
    if maybe_value is None:
        err('Expected valid value, found None')
    return maybe_value

# append a source code line
out_lines = ''
def l(s: str) :
    global out_lines
    out_lines += s + '\n'

def map_comment(inp:str, y:int, z:int, p:int, q:int) -> str:
    return inp\
        .replace('ry', r_comment[y])\
        .replace('rz', r_comment[z])\
        .replace('aluy', alu_comment[y])\
        .replace('rp', rp_comment[p])\
        .replace('rp2', rp2_comment[p])\
        .replace('ccy-4', cc_comment[y-4])\
        .replace('ccy', cc_comment[y])\
        .replace('y*8', f'{y*8:X}h')

def map_set(inp:str, y:int, z:int, p:int, q:int) -> str:
    return inp\
        .replace('ry', r_set[y])\
        .replace('rz', r_set[z])\
        .replace('rp', rp_set[p])\
        .replace('rp2', rp2_set[p])\
        .replace('dlatch', 'cpu->dlatch=_X_')

def map_get(inp:str, y:int, z:int, p:int, q:int) -> str:
    return inp\
        .replace('ry', r_get[y])\
        .replace('rz', r_get[z])\
        .replace('rp', rp_get[p])\
        .replace('rp2', rp2_get[p])\
        .replace('af', '_gaf()')\
        .replace('bc', '_gbc()')\
        .replace('de', '_gde()')\
        .replace('hl', '_ghl()')\
        .replace('dlatch', 'cpu->dlatch')

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

def parse_opdescs():
    with open('z80_desc.yml') as fp:
        desc = yaml.load(fp, Loader=yaml.FullLoader) # type: ignore
        for (op_name, op_desc) in desc.items():
            if 'cond' not in op_desc:
                err(f"op '{op_name}'' has no condition!")
            op = Op(op_name,op_desc['cond'])
            num_fetch = 0
            num_overlapped = 0
            for (mc_name, mc_items) in op_desc.items():
                if mc_name != 'cond':
                    mc_type, mc_tcycles = parse_mcycle_name(mc_name)
                    if mc_type == 'fetch':
                        num_fetch += 1
                    elif mc_type == 'overlapped':
                        num_overlapped += 1
                    mc = MCycle(mc_type, mc_tcycles, mc_items)
                    op.mcycles.append(mc)
            if num_fetch == 0:
                op.mcycles.insert(0, MCycle('fetch', FETCH_TCYCLES, {}))
            if num_overlapped == 0:
                op.mcycles.append(MCycle('overlapped', OVERLAPPED_TCYCLES, {}))
            OP_PATTERNS.append(op)

def stampout_mcycle_items(mcycle_items: dict[str,str], y: int, z: int, p: int, q: int) -> dict[str,str]:
    res: dict[str,str] = {}
    for key,val in mcycle_items.items():
        if key in ['ab', 'db', 'code']:
            val = map_get(val, y, z, p, q)
        elif key == 'dst':
            val = map_set(val, y, z, p, q)
        res[key] = val
    return res

def expand_optable():
    for opcode in range(0,256):
        #  76 543 210
        # |xx|yyy|zzz|
        #    |ppq|
        x = opcode >> 6 # type: ignore (generated unused warning, but x is needed in 'eval')
        y = (opcode >> 3) & 7
        z = opcode & 7
        p = y >> 1
        q = y & 1
        for op_desc_index,op_desc in enumerate(OP_PATTERNS):
            if eval(op_desc.cond_compiled):
                if OPS[opcode] is not None:
                    err(f"Condition collission for opcode {op_desc_index:02X} and '{op_desc.name}'")
                op = copy.deepcopy(OP_PATTERNS[op_desc_index])
                op.name = map_comment(op.name, y, z, p, q)
                op.opcode = opcode
                for mcycle in op.mcycles:
                    mcycle.items = stampout_mcycle_items(mcycle.items, y, z, p, q)
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
    for maybe_op in OPS:
        op = unwrap(maybe_op)

        def add(opcode: int, action: str):
            nonlocal step
            l(f'case (0x{opcode:02X}<<3)|{step}: {action}; break;')
            step += 1

        step = 0
        opc = op.opcode
        l(f'\n// {op.name}')
        for i,mcycle in enumerate(op.mcycles):
            # execution mask for the current machine cycle
            pip = build_pip(mcycle)
            if mcycle.type == 'fetch':
                l(f'// -- M1')
            elif mcycle.type == 'overlapped':
                l(f'// -- OVERLAP')
            else:
                l(f'// -- M{i+1}')
            if mcycle.type == 'fetch':
                # initialize next tcycle mask
                add(opc, f'cpu->pip=0x{pip:X}')
                # refresh cycle
                add(opc, '_rfsh()')
            elif mcycle.type == 'mread':
                addr = mcycle.items['ab']
                store = mcycle.items['dst'].replace('_X_', '_gd()')
                add(opc, f'cpu->pip=0x{pip:X};_mr({addr})')
                add(opc, f'{store}')
            elif mcycle.type == 'write':
                add(opc, f'cpu->pip=0x{pip:X}')
                add(opc, '_mw(0xFFFF,0xFF)/*FIXME: address and data!*/')
            elif mcycle.type == 'ioread':
                add(opc, f'cpu->pip=0x{pip:X}')
                add(opc, '_ior(0xFFFF)/*FIXME: address!*/')
                add(opc, '/*FIXME: read data bus*/')
            elif mcycle.type == 'iowrite':
                add(opc, f'cpu->pip=0x{pip:X}')
                add(opc, '_iow(0xFFFF,0xFF)/*FIXME: address and data!*/')
            elif mcycle.type == 'generic':
                add(opc, f'cpu->pip=0x{pip:X}/*FIXME: action*/')
            elif mcycle.type == 'overlapped':
                action = mcycle.items['code'] if 'code' in mcycle.items else ''
                add(opc, f'cpu->pip=0x{pip:X};_fetch();{action}')
        if step > 7:
            err(f"too many steps in instruction '{op.name}'!")

def dump():
    # for op_desc in OP_PATTERNS:
    #     print(f"\nop(name='{op_desc.name}', cond='{op_desc.cond}'):")
    #     for mc in op_desc.mcycles:
    #         print(f'  mcycle(type={mc.type}, tcycles={mc.tcycles})')
    #         for key,val in mc.items.items():
    #             print(f'    {key}:{val}')
    # print('\n')
    print(out_lines)

if __name__=='__main__':
    parse_opdescs()
    expand_optable()
    gen_decoder()
    dump()
