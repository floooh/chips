import yaml, copy
import templ

DESC_PATH  = 'z80_desc.yml'
INOUT_PATH  = '../chips/z80.h'
TAB_WIDTH  = 4

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
        self.multiple = False
        self.multiple_first_op_index = -1
        self.num_cycles = 0
        self.num_steps = 0
        self.step_index = -1
        self.extra_step_index = -1
        self.mcycles = []

OP_DESCS = []

NUM_SPECIAL_OPS = 7

OPS = [None for _ in range(0,2*256 + NUM_SPECIAL_OPS)]

# a fetch machine cycle is processed as 2 parts because it overlaps
# with the 'action' of the previous instruction
FETCH_TCYCLES = 3
OVERLAPPED_FETCH_TCYCLES = 1
MEM_TCYCLES = 3
IO_TCYCLES = 4

# register mapping tables, see: http://www.z80.info/decoding.htm
r_comment   = [ 'B', 'C', 'D', 'E', 'H', 'L', '(HL)', 'A' ]
rp_comment  = [ 'BC', 'DE', 'HL', 'SP' ]
rp2_comment = [ 'BC', 'DE', 'HL', 'AF' ]
alu_comment = [ 'ADD', 'ADC', 'SUB', 'SBC', 'AND', 'XOR', 'OR', 'CP' ]
rot_comment = [ 'RLC', 'RRC', 'RL', 'RR', 'SLA', 'SRA', 'SLL', 'SRL' ]
cc_comment  = [ 'NZ', 'Z', 'NC', 'C', 'PO', 'PE', 'P', 'M' ]

r_map    = [ 'cpu->b', 'cpu->c', 'cpu->d', 'cpu->e', 'cpu->hlx[cpu->hlx_idx].h', 'cpu->hlx[cpu->hlx_idx].l', 'XXX', 'cpu->a' ]
rr_map   = [ 'cpu->b', 'cpu->c', 'cpu->d', 'cpu->e', 'cpu->h', 'cpu->l', 'XXX', 'cpu->a' ]
rp_map   = [ 'cpu->bc', 'cpu->de', 'cpu->hlx[cpu->hlx_idx].hl', 'cpu->sp' ]
rrp_map  = [ 'cpu->bc', 'cpu->de', 'cpu->hl', 'cpu->sp' ]
rpl_map  = [ 'cpu->c', 'cpu->e', 'cpu->hlx[cpu->hlx_idx].l', 'cpu->spl']
rrpl_map = [ 'cpu->c', 'cpu->e', 'cpu->l', 'cpu->spl']
rph_map  = [ 'cpu->b', 'cpu->d', 'cpu->hlx[cpu->hlx_idx].h', 'cpu->sph']
rrph_map  = [ 'cpu->b', 'cpu->d', 'cpu->h', 'cpu->sph']
rp2_map  = [ 'cpu->bc', 'cpu->de', 'cpu->hlx[cpu->hlx_idx].hl', 'cpu->af' ]
rp2l_map = [ 'cpu->c', 'cpu->e', 'cpu->hlx[cpu->hlx_idx].l', 'cpu->f']
rp2h_map = [ 'cpu->b', 'cpu->d', 'cpu->hlx[cpu->hlx_idx].h', 'cpu->a']
cc_map   = [ '_cc_nz', '_cc_z', '_cc_nc', '_cc_c', '_cc_po', '_cc_pe', '_cc_p', '_cc_m' ]
alu_map  = [ '_z80_add8(cpu,',
             '_z80_adc8(cpu,',
             '_z80_sub8(cpu,',
             '_z80_sbc8(cpu,',
             '_z80_and8(cpu,',
             '_z80_xor8(cpu,',
             '_z80_or8(cpu,',
             '_z80_cp8(cpu,' ]
rot_map  = [ '_z80_rlc(cpu,',
             '_z80_rrc(cpu,',
             '_z80_rl(cpu,',
             '_z80_rr(cpu,',
             '_z80_sla(cpu,',
             '_z80_sra(cpu,',
             '_z80_sll(cpu,',
             '_z80_srl(cpu,' ]
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

def map_cpu(inp, y, z, p, q):
    return inp\
        .replace('$ADDR', 'cpu->addr')\
        .replace('$ALU(', alu_map[y])\
        .replace('$ROT(', rot_map[y])\
        .replace('$RRPL', rrpl_map[p])\
        .replace('$RRPH', rrph_map[p])\
        .replace('$RRP', rrp_map[p])\
        .replace('$RRY', rr_map[y])\
        .replace('$RRZ', rr_map[z])\
        .replace('$RY', r_map[y])\
        .replace('$RZ', r_map[z])\
        .replace('$RP2L', rp2l_map[p])\
        .replace('$RP2H', rp2h_map[p])\
        .replace('$RP2', rp2_map[p])\
        .replace('$RPL', rpl_map[p])\
        .replace('$RPH', rph_map[p])\
        .replace('$RP', rp_map[p])\
        .replace('$CC-4', cc_map[y-4])\
        .replace('$CC', cc_map[y])\
        .replace('$PCL', 'cpu->pcl')\
        .replace('$PCH', 'cpu->pch')\
        .replace('$PC', 'cpu->pc')\
        .replace('$AF', 'cpu->af')\
        .replace('$BC', 'cpu->bc')\
        .replace('$DE', 'cpu->de')\
        .replace('$HL', 'cpu->hlx[cpu->hlx_idx].hl')\
        .replace('$SP', 'cpu->sp')\
        .replace('$WZL', 'cpu->wzl')\
        .replace('$WZH', 'cpu->wzh')\
        .replace('$WZ', 'cpu->wz')\
        .replace('$DLATCH', 'cpu->dlatch')\
        .replace('$A', 'cpu->a')\
        .replace('$C', 'cpu->c')\
        .replace('$B', 'cpu->b')\
        .replace('$E', 'cpu->e')\
        .replace('$D', 'cpu->d')\
        .replace('$L', 'cpu->hlx[cpu->hlx_idx].l')\
        .replace('$H', 'cpu->hlx[cpu->hlx_idx].h')\
        .replace('$IMY', im_map[y])\
        .replace('$IM', 'cpu->im')\
        .replace('$Y*8', f'0x{y*8:02X}')\

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
            if 'mcycles' not in op_desc:
                err(f"op '{op_name}' has no mcycles!")
            flags = {}
            if 'flags' in op_desc:
                flags = op_desc['flags']
            op = Op(op_name,op_desc['cond'], flags)
            if 'prefix' in op_desc:
                op.prefix = op_desc['prefix']
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
                        'overlapped': OVERLAPPED_FETCH_TCYCLES,
                        'generic': -1
                    }
                    mc_tcycles = default_tcycles[mc_type]
                if mc_tcycles == -1:
                    err(f'generic mcycles must have explicit length (op: {op_name})')
                if (mc_tcycles < 1) or (mc_tcycles > 7):
                    err(f'invalid mcycle len: {mc_tcycles}')
                if mc_type == 'fetch':
                    if mc_tcycles != 4:
                        err('fetch ticks must have exactly 4 tcycles')
                    mc_tcycles -= OVERLAPPED_FETCH_TCYCLES
                if mc_type == 'overlapped' and mc_tcycles != 1:
                    err('overlapped ticks must have exactly 1 tcycle!')
                mc = MCycle(mc_type, mc_tcycles, mc_desc)
                op.mcycles.append(mc)
            if num_fetch == 0:
                op.mcycles.insert(0, MCycle('fetch', FETCH_TCYCLES, {}))
            if num_overlapped == 0:
                op.mcycles.append(MCycle('overlapped', OVERLAPPED_FETCH_TCYCLES, {}))
            OP_DESCS.append(op)

def find_opdesc(name):
    for op_desc in OP_DESCS:
        if op_desc.name == name:
            return op_desc
    err(f"opdesc not found for '{name}'")
    return None

def find_op(name):
    for op in OPS:
        if op.name == name:
            return op
    err(f"op not found for '{name}'")
    return None

def stampout_mcycle_items(mcycle_items, y, z, p, q):
    res = {}
    for key,val in mcycle_items.items():
        if type(val) == str:
            res[key] = map_cpu(val, y, z, p, q)
    return res

def stampout_op(prefix, opcode, op_index, op_desc):
    #  76 543 210
    # |xx|yyy|zzz|
    #    |ppq|
    y = (opcode >> 3) & 7
    z = opcode & 7
    p = y >> 1
    q = y & 1
    if op_desc.multiple_first_op_index == -1:
        op_desc.multiple_first_op_index = op_index
    op = copy.deepcopy(op_desc)
    op.name = map_comment(op.name, y, z, p, q)
    op.prefix = prefix
    op.opcode = opcode
    if flag(op, 'multiple') and op.multiple_first_op_index != op_index:
        op.flags['redundant'] = True
    for mcycle in op.mcycles:
        mcycle.items = stampout_mcycle_items(mcycle.items, y, z, p, q)
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
            for op_desc_index,op_desc in enumerate(OP_DESCS):
                if not flag(op_desc, 'special'):
                    if eval(op_desc.cond_compiled) and op_desc.prefix == prefix:
                        if OPS[op_index] is not None:
                            err(f"Condition collission for opcode {op_desc_index:02X} and '{op_desc.name}'")
                        stampout_op(prefix, opcode, op_index, op_desc)
    op_index += 1; stampout_op('cb', -1, op_index, find_opdesc('cb'))
    op_index += 1; stampout_op('cb', -1, op_index, find_opdesc('cbhl'))
    op_index += 1; stampout_op('cb', -1, op_index, find_opdesc('ddfdcb'))
    op_index += 1; stampout_op('', -1, op_index, find_opdesc('int_im0'))
    op_index += 1; stampout_op('', -1, op_index, find_opdesc('int_im1'))
    op_index += 1; stampout_op('', -1, op_index, find_opdesc('int_im2'))
    op_index += 1; stampout_op('', -1, op_index, find_opdesc('nmi'))

# compute number of tcycles in an instruction
def compute_tcycles(op):
    cycles = 0
    for mcycle in op.mcycles:
        cycles += mcycle.tcycles
    return cycles

# generate code for one op
def gen_decoder():
    indent = 2
    cur_step = 0
    cur_extra_step = 512 # main and ed ops
    out_lines = ''
    out_extra_lines = ''

    def tab():
        return ' ' * TAB_WIDTH * indent

    def l(s):
        nonlocal out_lines
        out_lines += tab() + s + '\n'

    def lx(s):
        nonlocal out_extra_lines
        out_extra_lines += tab() + s + '\n'

    def add(action):
        nonlocal cur_step, cur_extra_step, op_step, op
        # NOTE: special ops (interrupt handling etc) are entirely written into the 'extra' decoder block
        if op_step == 0 and not flag(op, 'special'):
            next_step = cur_extra_step
            # check if this is a redundant op which needs to step to a shared payload
            if flag(op, 'redundant'):
                next_step = OPS[op.multiple_first_op_index].extra_step_index
            action = action.replace("$NEXTSTEP", f'{next_step}')
            l(f'case {cur_step:4}: {action}_goto({next_step}); // {op.name} T:{op_step}')
            cur_step += 1
        else:
            # do not write a payload for redundant ops
            if not flag(op, 'redundant'):
                action = action.replace("$NEXTSTEP", f'{cur_extra_step + 1}')
                lx(f'case {cur_extra_step:4}: {action}_step(); // {op.name} T:{op_step}')
                cur_extra_step += 1
        op_step += 1

    def add_fetch(action):
        nonlocal cur_step, cur_extra_step, op_step, op
        if op_step == 0 and not flag(op, 'special'):
            l(f'case {cur_step:4}: {action}_fetch(); // {op.name} T:{op_step}')
            cur_step += 1
        else:
            lx(f'case {cur_extra_step:4}: {action}_fetch(); // {op.name} T:{op_step}')
            cur_extra_step += 1
        op_step += 1

    def add_stepto(action):
        nonlocal cur_step, cur_extra_step, op_step, op
        if op_step == 0:
            l(f'case {cur_step:4}: {action}goto step_to; // {op.name} T:{op_step}')
            cur_step += 1
        else:
            lx(f'case {cur_extra_step:4}: {action}goto step_to; // {op.name} T:{op_step}')
            cur_extra_step += 1
        op_step += 1

    for op in OPS:
        op_step = 0
        op.num_cycles = compute_tcycles(op)
        op.step_index = cur_step
        op.extra_step_index = cur_extra_step

        for i,mcycle in enumerate(op.mcycles):
            action = (f"{mcycle.items['action']};" if 'action' in mcycle.items else '')
            if mcycle.type == 'fetch':
                pass
            elif mcycle.type == 'mread':
                addr = mcycle.items['ab']
                store = mcycle.items['dst'].replace('_X_', '_gd()')
                add('')
                add(f'_wait();_mread({addr});')
                add(f'{store}=_gd();{action}')
                for _ in range(3,mcycle.tcycles):
                    add('')
            elif mcycle.type == 'mwrite':
                addr = mcycle.items['ab']
                data = mcycle.items['db']
                add('')
                add(f'_wait();_mwrite({addr},{data});{action}')
                add('')
                for _ in range(3,mcycle.tcycles):
                    add('')
            elif mcycle.type == 'ioread':
                addr = mcycle.items['ab']
                store = mcycle.items['dst'].replace('_X_', '_gd()')
                add('')
                add('')
                add(f'_wait();_ioread({addr});')
                add(f'{store}=_gd();{action}')
                for _ in range(4,mcycle.tcycles):
                    add('')
            elif mcycle.type == 'iowrite':
                addr = mcycle.items['ab']
                data = mcycle.items['db']
                add('')
                add(f'_iowrite({addr},{data});')
                add(f'_wait();{action}')
                add('')
                for _ in range(4,mcycle.tcycles):
                    add('')
            elif mcycle.type == 'generic':
                add(f'{action}')
                for _ in range(1,mcycle.tcycles):
                    add('')
            elif mcycle.type == 'overlapped':
                action = (f"{mcycle.items['action']};" if 'action' in mcycle.items else '')
                if 'post_action' in mcycle.items:
                    # if a post-action is defined we can jump to the common fetch block but
                    # instead squeeze the fetch before the fetch action
                    post_action = (f"{mcycle.items['post_action']};" if 'post_action' in mcycle.items else '')
                    add_stepto(f"{action}pins=_z80_fetch(cpu,pins);{post_action}")
                elif 'prefix' in mcycle.items:
                    # likewise if this is a prefix instruction special case
                    add_stepto(f"{action}_fetch_{mcycle.items['prefix']}();")
                else:
                    # regular case, jump to the shared fetch block after the
                    add_fetch(f'{action}')
        op.num_steps = op_step
    return { 'out_lines': out_lines + out_extra_lines, 'max_step': cur_extra_step }

def extra_step_defines_string(max_step):
    manual_steps = [
        "M1_T2",
        "M1_T3",
        "M1_T4",
        "DDFD_M1_T2",
        "DDFD_M1_T3",
        "DDFD_M1_T4",
        "DDFD_D_T1",
        "DDFD_D_T2",
        "DDFD_D_T3",
        "DDFD_D_T4",
        "DDFD_D_T5",
        "DDFD_D_T6",
        "DDFD_D_T7",
        "DDFD_D_T8",
        "DDFD_LDHLN_WR_T1",
        "DDFD_LDHLN_WR_T2",
        "DDFD_LDHLN_WR_T3",
        "DDFD_LDHLN_OVERLAPPED",
        "ED_M1_T2",
        "ED_M1_T3",
        "ED_M1_T4",
        "CB_M1_T2",
        "CB_M1_T3",
        "CB_M1_T4",
    ]
    res = ''
    step_index = max_step
    for step_name in manual_steps:
        res += f'#define Z80_{step_name} {step_index}\n'
        step_index += 1
    special_steps = {
        'cb': 'CB_STEP',
        'cbhl': 'CBHL_STEP',
        'ddfdcb': 'DDFDCB_STEP',
        'int_im0': 'INT_IM0_STEP',
        'int_im1': 'INT_IM1_STEP',
        'int_im2': 'INT_IM2_STEP',
        'nmi': 'NMI_STEP',
    }
    for op_name, step_name in special_steps.items():
        op = find_op(op_name)
        res += f'#define Z80_{step_name} {op.extra_step_index}\n'
    return res

def indirect_table_string():
    res = ''
    for i in range(0, 256):
        op = OPS[i]
        if i % 16 == 0:
            res += '    '
        if flag(op, 'indirect'):
            res += '1,'
        else:
            res += '0,'
        if i % 16 == 15:
            res += '\n'
    return res

def write_result(decoder_output):
    out_lines = decoder_output['out_lines']
    max_step = decoder_output['max_step']
    with open(INOUT_PATH, 'r') as f:
        lines = f.read().splitlines()
        lines = templ.replace(lines, 'decoder', out_lines)
        lines = templ.replace(lines, 'extra_step_defines', extra_step_defines_string(max_step))
        lines = templ.replace(lines, 'indirect_table', indirect_table_string())
    out_str = '\n'.join(lines) + '\n'
    with open(INOUT_PATH, 'w') as f:
        f.write(out_str)

if __name__ == '__main__':
    parse_opdescs()
    expand_optable()
    decoder_output = gen_decoder()
    write_result(decoder_output)
