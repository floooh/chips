from os import X_OK
import yaml, copy
from string import Template
from typing import Optional, TypeVar

DESC_PATH  = 'z80_desc.yml'
TEMPL_PATH = 'z80x.template.h'
OUT_PATH   = '../chips/z80x.h'
TAB_WIDTH  = 4

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
        self.pip: int = 0
        self.num_cycles = 0
        self.num_steps = 0
        self.decoder_offset: int = 0
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
r_comment   = [ 'b', 'c', 'd', 'e', 'h', 'l', '(hl)', 'a' ]
rp_comment  = [ 'bc', 'de', 'hl', 'sp' ]
rp2_comment = [ 'bc', 'de', 'hl', 'af' ]
alu_comment = [ 'add', 'adc', 'sub', 'sbc', 'and', 'xor', 'or', 'cp' ]
cc_comment  = [ 'nz', 'z', 'nc', 'c', 'po', 'pe', 'p', 'm' ]

r_set   = [ 'cpu->b=_X_', 'cpu->c=_X_', 'cpu->d=_X_', 'cpu->e=_X_', 'cpu->h=_X_', 'cpu->l=_X_', 'XXX', 'cpu->a=_X_' ]
rp_set  = [ '_sbc(_X_)', '_sde(_X_)', '_shl(_X_)', '_ssp(_X_)' ]
rp2_set = [ '_sbc(_X_)', '_sde(_X_)', '_shl(_X_)', '_saf(_X_)' ]
alu_map = [ 'z80_add',  'z80_adc', 'z80_sub', 'z80_sbc', 'z80_and', 'z80_xor', 'z80_or', 'z80_cp' ]

r_get   = [ 'cpu->b', 'cpu->c', 'cpu->d', 'cpu->e', 'cpu->h', 'cpu->l', 'XXX', 'cpu->a' ]
rp_get  = [ 'BC', 'DE', 'HL', 'SP' ]
rp2_get = [ 'BC', 'DE', 'HL', 'AF' ]

def err(msg: str):
    raise BaseException(msg)

T = TypeVar('T')
def unwrap(maybe_value: Optional[T]) -> T:
    if maybe_value is None:
        err('Expected valid value, found None')
    return maybe_value

# append a source code line
indent: int = 0
out_lines: str = ''

def tab() -> str:
    return ' ' * TAB_WIDTH * indent

def l(s: str) :
    global out_lines
    out_lines += tab() + s + '\n'

def map_comment(inp:str, y:int, z:int, p:int, q:int) -> str:
    return inp\
        .replace('RY', r_comment[y])\
        .replace('RZ', r_comment[z])\
        .replace('ALU', alu_comment[y])\
        .replace('RP', rp_comment[p])\
        .replace('RP2', rp2_comment[p])\
        .replace('CC-4', cc_comment[y-4])\
        .replace('CC', cc_comment[y])\
        .replace('Y*8', f'{y*8:X}h')

def map_set(inp:str, y:int, z:int, p:int, q:int) -> str:
    return inp\
        .replace('RY', r_set[y])\
        .replace('RZ', r_set[z])\
        .replace('RP', rp_set[p])\
        .replace('RP2', rp2_set[p])\
        .replace('DLATCH', 'cpu->dlatch=_X_')

def map_get(inp:str, y:int, z:int, p:int, q:int) -> str:
    return inp\
        .replace('ALU', alu_map[y])\
        .replace('RY', r_get[y])\
        .replace('RZ', r_get[z])\
        .replace('RP', rp_get[p])\
        .replace('RP2', rp2_get[p])\
        .replace('AF', '_gaf()')\
        .replace('BC', '_gbc()')\
        .replace('DE', '_gde()')\
        .replace('HL', '_ghl()')\
        .replace('DLATCH', 'cpu->dlatch')

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
    with open(DESC_PATH, 'r') as fp:
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

# build the execution pipeline bitmask for a given instruction
def build_pip(op: Op) -> int:
    pip = 0
    cycle = 0

    def tcycles(bits: list[int], wait_cycle: int, total: int):
        nonlocal cycle
        nonlocal pip
        if wait_cycle != -1:
            pip |= (1<<(32 + wait_cycle + cycle))
        for i in range(0,total):
            if i < len(bits):
                if bits[i] != 0:
                    pip |=  (1<<(i + cycle))
        cycle += total

    for mcycle in op.mcycles:
        if mcycle.type == 'fetch':
            # the last 3 tcycles of instruction fetch, the wait pin is
            # sampled on the first cycle (T2 of the whole fetch machine cycle),
            # and in T3 the refresh cycle is initiated
            tcycles([0,1], 0, mcycle.tcycles)
        elif mcycle.type == 'mread':
            # memory read machine cycle, initiate the read in T1 and T2 to store the result
            # the wait pin is sampled on T2
            tcycles([1,1,0], 1, mcycle.tcycles)
        elif mcycle.type == 'mwrite':
            # memory write machine cycle, initiate the write and sample wait pin in T2
            tcycles([0,1,0], 1, mcycle.tcycles)
        elif mcycle.type == 'ioread':
            # io read is like a memory read delayed by one cycle
            tcycles([0,1,1,0], 2, mcycle.tcycles)
        elif mcycle.type == 'iowrite':
            # io write is like a 
            tcycles([0,1,0,0], 2, mcycle.tcycles)
        elif mcycle.type == 'generic':
            tcycles([1], -1, mcycle.tcycles)
        elif mcycle.type == 'overlapped':
            # the final overlapped tcycle is actually the first tcycle
            # of the next instruction and only initiates a memory fetch
            tcycles([1], -1, mcycle.tcycles)
    op.num_cycles = cycle
    return pip

# generate code for one op
def gen_decoder():
    global indent
    indent = 3
    decoder_step = 0

    def add(opcode: int, action: str):
        nonlocal decoder_step
        nonlocal step
        l(f'case 0x{decoder_step:04X}: {action}; break;')
        decoder_step += 1
        step += 1
    
    for maybe_op in OPS:
        op = unwrap(maybe_op)
        step = 0
        opc = op.opcode
        op.pip = build_pip(op)
        op.decoder_offset = decoder_step

        l('')
        l(f'// {op.name} (M:{len(op.mcycles)-1} T:{op.num_cycles})')
        for i,mcycle in enumerate(op.mcycles):
            if mcycle.type == 'fetch':
                l(f'// -- M1 (fetch/rfsh)')
                add(opc, '_rfsh()')
            elif mcycle.type == 'mread':
                l(f'// -- M{i+1} (mread)')
                addr = mcycle.items['ab']
                store = mcycle.items['dst'].replace('_X_', '_gd()')
                add(opc, f'_mr({addr})')
                add(opc, f'{store}')
            elif mcycle.type == 'mwrite':
                l(f'// -- M{i+1} (mwrite)')
                add(opc, '_mw(0xFFFF,0xFF)/*FIXME: address and data!*/')
            elif mcycle.type == 'ioread':
                l(f'// -- M{i+1} (ioread)')
                add(opc, '_ior(0xFFFF)/*FIXME: address!*/')
                add(opc, '/*FIXME: read data bus*/')
            elif mcycle.type == 'iowrite':
                l(f'// -- M{i+1} (iowrite)')
                add(opc, '_iow(0xFFFF,0xFF)/*FIXME: address and data!*/')
            elif mcycle.type == 'generic':
                l(f'// -- M{i+1} (generic)')
                add(opc, f'/*FIXME: action*/')
            elif mcycle.type == 'overlapped':
                l(f'// -- OVERLAP')
                action = (f"{mcycle.items['code']};" if 'code' in mcycle.items else '')
                add(opc, f'{action}_fetch()')
        op.num_steps = step
        # the number of steps must match the number of step-bits in the
        # execution pipeline
        step_pip = op.pip & ((1<<32)-1)
        if step != bin(step_pip).count('1'):
            err(f"Pipeline vs steps mismatch in '{op.name}(0x{op.opcode:02X})': {step_pip:b} vs {op.num_steps}")

def dump():
    # for op_desc in OP_PATTERNS:
    #     print(f"\nop(name='{op_desc.name}', cond='{op_desc.cond}'):")
    #     for mc in op_desc.mcycles:
    #         print(f'  mcycle(type={mc.type}, tcycles={mc.tcycles})')
    #         for key,val in mc.items.items():
    #             print(f'    {key}:{val}')
    # print('\n')
    print(out_lines)

def pip_table_to_string() -> str:
    global indent
    indent = 1
    res: str = ''
    for maybe_op in OPS:
        op = unwrap(maybe_op)
        res += tab() + f'// {op.name} (M:{len(op.mcycles)-1} T:{op.num_cycles} steps:{op.num_steps})\n'
        res += tab() + f'{{ 0x{op.pip:016X}, 0x{op.decoder_offset:04X} }},\n' 
    return res

def write_result():
    with open(TEMPL_PATH, 'r') as templf:
        templ = Template(templf.read())
        c_src = templ.safe_substitute(decode_block=out_lines, pip_table_block=pip_table_to_string())
        with open(OUT_PATH, 'w') as outf:
            outf.write(c_src)

if __name__=='__main__':
    parse_opdescs()
    expand_optable()
    gen_decoder()
    write_result()
