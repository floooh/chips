import yaml, pprint

OPS = []

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
        self.mcycles: list[MCycle] = []

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

def parse():
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
            OPS.append(op)

def dump():
    for op in OPS:
        print(f"\nop(name='{op.name}', cond='{op.cond}'):")
        for mc in op.mcycles:
            print(f'  mcycle(type={mc.type}, tcycles={mc.tcycles})')
            for item in mc.items:
                print(f'    {item}')

if __name__=='__main__':
    parse()
    dump()
