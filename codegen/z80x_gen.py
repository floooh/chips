import yaml, copy
from string import Template
from typing import Optional, TypeVar

FIRST_DECODER_STEP = 5
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
    def __init__(self, name:str, cond:str, flags: dict[str,str]):
        self.name: str = name
        self.cond: str = cond
        self.cond_compiled = compile(cond, '<string>', 'eval')
        self.flags: dict[str,str] = flags
        self.opcode: int = -1
        self.pip: int = 0
        self.num_cycles = 0
        self.num_steps = 0
        self.decoder_offset: int = 0
        self.mcycles: list[MCycle] = []

OP_PATTERNS: list[Op] = []
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

r_map   = [ 'cpu->b', 'cpu->c', 'cpu->d', 'cpu->e', 'cpu->hlx[cpu->hlx_idx].h', 'cpu->hlx[cpu->hlx_idx].l', 'XXX', 'cpu->a' ]
rr_map  = [ 'cpu->b', 'cpu->c', 'cpu->d', 'cpu->e', 'cpu->h', 'cpu->l', 'XXX', 'cpu->a' ]
rp_map  = [ 'cpu->bc', 'cpu->de', 'cpu->hlx[cpu->hlx_idx].hl', 'cpu->sp' ]
rpl_map = [ 'cpu->c', 'cpu->e', 'cpu->hlx[cpu->hlx_idx].l', 'cpu->spl']
rph_map = [ 'cpu->b', 'cpu->d', 'cpu->hlx[cpu->hlx_idx].h', 'cpu->sph']
rp2_map = [ 'cpu->bc', 'cpu->de', 'cpu->hlx[cpu->hlx_idx].hl', 'cpu->af' ]
rp2l_map = [ 'cpu->c', 'cpu->e', 'cpu->hlx[cpu->hlx_idx].l', 'cpu->f']
rp2h_map = [ 'cpu->b', 'cpu->d', 'cpu->hlx[cpu->hlx_idx].h', 'cpu->a']
alu_map = [ 'z80_add8(cpu,', 
            'z80_adc8(cpu,', 
            'z80_sub8(cpu,',
            'z80_sbc8(cpu,',
            'z80_and8(cpu,',
            'z80_xor8(cpu,',
            'z80_or8(cpu,',
            'z80_cp8(cpu,' ]
cc_map = [ '_cc_nz', '_cc_z', '_cc_nc', '_cc_c', '_cc_po', '_cc_pe', '_cc_p', '_cc_m' ]

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
        .replace('RP2', rp2_comment[p])\
        .replace('RP', rp_comment[p])\
        .replace('CC-4', cc_comment[y-4])\
        .replace('CC', cc_comment[y])\
        .replace('Y*8', f'{y*8:X}h')

def map_cpu(inp:str, y:int, z:int, p:int, q:int) -> str:
    return inp\
        .replace('ADDR', 'cpu->addr')\
        .replace('ALU(', alu_map[y])\
        .replace('RRY', rr_map[y])\
        .replace('RRZ', rr_map[z])\
        .replace('RY', r_map[y])\
        .replace('RZ', r_map[z])\
        .replace('RP2L', rp2l_map[p])\
        .replace('RP2H', rp2h_map[p])\
        .replace('RP2', rp2_map[p])\
        .replace('RPL', rpl_map[p])\
        .replace('RPH', rph_map[p])\
        .replace('RP', rp_map[p])\
        .replace('CC-4', cc_map[y-4])\
        .replace('CC', cc_map[y])\
        .replace('PC', 'cpu->pc')\
        .replace('AF', 'cpu->af')\
        .replace('BC', 'cpu->bc')\
        .replace('DE', 'cpu->de')\
        .replace('HL', 'cpu->hlx[cpu->hlx_idx].hl')\
        .replace('SP', 'cpu->sp')\
        .replace('WZL', 'cpu->wzl')\
        .replace('WZH', 'cpu->wzh')\
        .replace('WZ', 'cpu->wz')\
        .replace('DLATCH', 'cpu->dlatch')\
        .replace('A', 'cpu->a')\
        .replace('C', 'cpu->c')\
        .replace('B', 'cpu->b')\
        .replace('E', 'cpu->e')\
        .replace('D', 'cpu->d')\
        .replace('L', 'cpu->hlx[cpu->hlx_idx].l')\
        .replace('H', 'cpu->hlx[cpu->hlx_idx].h')

def parse_opdescs():
    with open(DESC_PATH, 'r') as fp:
        desc = yaml.load(fp, Loader=yaml.FullLoader) # type: ignore
        for (op_name, op_desc) in desc.items():
            if 'cond' not in op_desc:
                err(f"op '{op_name}' has no condition!")
            if 'mcycles' not in op_desc:
                err(f"op '{op_name}' has no mcycles!")
            flags: dict[str,str] = {}
            if 'flags' in op_desc:
                flags = op_desc['flags']
            op = Op(op_name,op_desc['cond'], flags)
            num_fetch = 0
            num_overlapped = 0
            for mc_desc in op_desc['mcycles']:
                mc_type = mc_desc['type']
                if mc_type == 'fetch':
                    num_fetch += 1
                elif mc_type == 'overlapped':
                    num_overlapped += 1
                if 'tcycles' in mc_desc:
                    mc_tcycles = mc_desc['tcycles']
                else:
                    default_tcycles = {
                        'fetch': FETCH_TCYCLES,
                        'mread': MEM_TCYCLES,
                        'mwrite': MEM_TCYCLES,
                        'ioread': IO_TCYCLES,
                        'iowrite': IO_TCYCLES,
                        'overlapped': OVERLAPPED_TCYCLES,
                        'generic': -1
                    }
                    mc_tcycles = default_tcycles[mc_type]
                if mc_tcycles == -1:
                    err(f'generic mcycles must have explicit length (op: {op_name})')
                if (mc_tcycles < 1) or (mc_tcycles > 6):
                    err(f'invalid mcycle len: {mc_tcycles}')
                if mc_type == 'fetch':
                    mc_tcycles -= OVERLAPPED_TCYCLES
                mc = MCycle(mc_type, mc_tcycles, mc_desc)
                op.mcycles.append(mc)
            if num_fetch == 0:
                op.mcycles.insert(0, MCycle('fetch', FETCH_TCYCLES, {}))
            if num_overlapped == 0:
                op.mcycles.append(MCycle('overlapped', OVERLAPPED_TCYCLES, {}))
            OP_PATTERNS.append(op)

def stampout_mcycle_items(mcycle_items: dict[str,str], y: int, z: int, p: int, q: int) -> dict[str,str]:
    res: dict[str,str] = {}
    for key,val in mcycle_items.items():
        if type(val) == str:
            res[key] = map_cpu(val, y, z, p, q)
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
            tcycles([], -1, mcycle.tcycles - 2)
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
    op.num_cycles = cycle + 2
    return pip

# generate code for one op
def gen_decoder():
    global indent
    indent = 3
    decoder_step = FIRST_DECODER_STEP

    def add(opcode: int, action: str):
        nonlocal decoder_step
        nonlocal step
        l(f'case 0x{decoder_step:04X}: {action} break;')
        decoder_step += 1
        step += 1
    
    for maybe_op in OPS:
        op = unwrap(maybe_op)
        step = 0
        opc = op.opcode
        op.pip = build_pip(op)
        op.decoder_offset = decoder_step

        l('')
        l(f'// 0x{op.opcode:02X}: {op.name} (M:{len(op.mcycles)-1} T:{op.num_cycles})')
        for i,mcycle in enumerate(op.mcycles):
            if mcycle.type == 'fetch':
                pass
            elif mcycle.type == 'mread':
                l(f'// -- M{i+1}')
                addr = mcycle.items['ab']
                store = mcycle.items['dst'].replace('_X_', '_gd()')
                action = (f"{mcycle.items['action']};" if 'action' in mcycle.items else '')
                add(opc, f'_mread({addr});')
                add(opc, f'{store}=_gd();{action}')
            elif mcycle.type == 'mwrite':
                l(f'// -- M{i+1}')
                addr = mcycle.items['ab']
                data = mcycle.items['db']
                action = (f"{mcycle.items['action']};" if 'action' in mcycle.items else '')
                add(opc, f'_mwrite({addr},{data});{action}')
            elif mcycle.type == 'ioread':
                l(f'// -- M{i+1} (ioread)')
                addr = mcycle.items['ab']
                store = mcycle.items['dst'].replace('_X_', '_gd()')
                add(opc, f'_ioread(addr);')
                add(opc, f'{store}=_gd();')
            elif mcycle.type == 'iowrite':
                l(f'// -- M{i+1} (iowrite)')
                addr = mcycle.items['ab']
                data = mcycle.items['db']
                add(opc, f'_iowrite({addr},{data});')
            elif mcycle.type == 'generic':
                l(f'// -- M{i+1} (generic)')
                action = (f"{mcycle.items['action']};" if 'action' in mcycle.items else '')
                add(opc, f'{action}')
            elif mcycle.type == 'overlapped':
                l(f'// -- OVERLAP')
                action = (f"{mcycle.items['action']};" if 'action' in mcycle.items else '')
                if 'prefix' in mcycle.items:
                    prefix = mcycle.items['prefix']
                    if prefix == 'ix':
                        fetch = '_fetch_ix();'
                    elif prefix == 'iy':
                        fetch = '_fetch_iy();'
                else:
                    fetch = '_fetch();'
                add(opc, f'{action}{fetch}')
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
        res += tab() + f'// 0x{op.opcode:02X}: {op.name} (M:{len(op.mcycles)-1} T:{op.num_cycles} steps:{op.num_steps})\n'
        flags = ''
        if 'indirect' in op.flags and op.flags['indirect']:
            flags += 'Z80_OPSTATE_FLAGS_INDIRECT'
        if 'imm8' in op.flags and op.flags['imm8']:
            if flags != '':
                flags += '|'
            flags += 'Z80_OPSTATE_FLAGS_IMM8'
        if flags == '':
            flags = '0'
        res += tab() + f'{{ 0x{op.pip:016X}, 0x{op.decoder_offset:04X}, {flags} }},\n' 
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
