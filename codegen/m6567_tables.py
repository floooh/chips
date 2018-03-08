#-------------------------------------------------------------------------------
#   m6567_tables.py
#   Generate various static data tables for the VIC-II
#
#   See http://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt
#   BUT: the tick tables don't start at the IRQ tick, but at the
#   last visible pixel (in vic-ii ticks at tick 56).
#   
#-------------------------------------------------------------------------------
import sys

OutPath = '../chips/_m6567_tables.h'
Out = None

# tick actions
I_ACCESS        = (1<<0)
C_ACCESS        = (1<<1)
G_ACCESS        = (1<<2)
S_ACCESS        = (1<<3)
P_ACCESS        = (1<<4)
BA_ACTIVE       = (1<<5)
AEC_ACTIVE      = (1<<6)
TEST_IRQ        = (1<<7)
H_RETRACE       = (1<<8)
RESET_VC_VMLI   = (1<<9)
RESET_RC        = (1<<10)
VCBASE_RC       = (1<<11)
GSEQ_START      = (1<<12)
GSEQ_STOP       = (1<<13)

#-------------------------------------------------------------------------------
def l(s):
    Out.write(s+'\n')

#-------------------------------------------------------------------------------
def is_badline(line_type):
    return 1 == line_type

#-------------------------------------------------------------------------------
def write_tick_table():
    l('uint16_t _m6567_ticks[10][64] = {')
    # line_types:
    #   0: no badline, no sprites
    #   1: badline, no sprites
    #   2: overlay bits for sprite 0 active
    #   3: overlay bits for sprite 1 active
    #   4: overlay bits for sprite 2 active
    #   5: overlay bits for sprite 3 active
    #   6: overlay bits for sprite 4 active
    #   7: overlay bits for sprite 5 active
    #   8: overlay bits for sprite 6 active
    #   9: overlay bits for sprite 7 active
    for line_type in range(0,10):
        line = '{ '
        for tick in range(0,64):
            bits = 0
            if line_type < 2:
                # the IRQ check tick
                if tick == 7:
                    bits |= TEST_IRQ
                # reload vcbase, bump rc (not in each line though, but this is the tick)
                if tick == 2:
                    bits |= VCBASE_RC
                # p-accesses for loading sprite pointers
                if tick == 2:
                    bits |= P_ACCESS 
                if tick == 4:
                    bits |= P_ACCESS
                if tick == 6:
                    bits |= P_ACCESS
                if tick == 8:
                    bits |= P_ACCESS
                if tick == 10:
                    bits |= P_ACCESS
                if tick == 12:
                    bits |= P_ACCESS
                if tick == 14:
                    bits |= P_ACCESS
                if tick == 16:
                    bits |= P_ACCESS
                # g-accesses and graphics sequencer active
                if tick >= 23 and tick < 63:
                    bits |= G_ACCESS
                if tick == 23:
                    bits |= GSEQ_START
                if tick == 62:
                    bits |= GSEQ_STOP
                # reload vc and vmli
                if tick == 21:
                    bits |= RESET_VC_VMLI
                
                # badline specific tick actions
                if is_badline(line_type):
                    # BA pin
                    if tick >= 19 and tick < 62:
                        bits |= BA_ACTIVE
                    # reset RC
                    if tick == 21:
                        bits |= RESET_RC
                    # AEC pin and c-accesses
                    if tick >= 22 and tick < 62:
                        bits |= AEC_ACTIVE
                        bits |= C_ACCESS
            line += '0x{:04X},'.format(bits)
        line += '},'
        l(line)
    l('};')

#-------------------------------------------------------------------------------
#   execution starts here
#
Out = open(OutPath, 'w')
l("/* machine generated, don't edit!*/")
write_tick_table()
Out.close();
