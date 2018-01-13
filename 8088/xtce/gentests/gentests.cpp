#include "alfe/main.h"
#include "alfe/space.h"

static const UInt16 testSegment = 0x10a8;

class Instruction
{
public:
    Instruction() { }
    Instruction(Byte opcode, Byte modrm = 0, Word offset = 0,
        Word immediate = 0)
      : _opcode(opcode), _modrm(modrm), _offset(offset), _immediate(immediate)
    { }
    bool hasModrm()
    {
        static const Byte hasModrmTable[] = {
            1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
            1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
            1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
            1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1};
        return hasModrmTable[_opcode] != 0;
    }
    int immediateBytes()
    {
        static const Byte immediateBytesTable[] = {
            0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0,
            0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0,
            0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0,
            0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0,
            2, 2, 2, 2, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 0, 2, 0, 1, 1, 0, 0, 2, 0, 2, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 4, 1, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        return immediateBytesTable[_opcode];
    }
    int modRMLength()
    {
        static const Byte modRMLengthsTable[] = {
            0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 
            0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 
            0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 
            0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        if (hasModrm())
            return 1 + modRMLengthsTable[_modrm];
        return 0;
    }
    int length()
    {
        return 1 + modRMLength() + immediateBytes();
    }
    int defaultSegment()
    {
        if (hasModrm() && _opcode != 0x8d) {
            static const signed char defaultSegmentTable[] = {
                3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 
                3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 
                3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 
                3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
                3, 3, 2, 2, 3, 3, 2, 3, 3, 3, 2, 2, 3, 3, 2, 3, 
               -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
               -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
               -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
               -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
            return defaultSegmentTable[_modrm];
        }
        int o = _opcode & 0xfe;
        if (o == 0xa0 || o == 0xa2 || o == 0xa4 || o == 0xa6 || o == 0xac ||
            _opcode == 0xd7)
            return 3;
        return -1;

    }
    void output(Byte* p)
    {
        *p = _opcode;
        ++p;
        if (hasModrm()) {
            *p = _modrm;
            ++p;
            int l = modRMLength();
            if (l > 0) {
                *p = (_offset & 0xff);
                if (l > 1)
                    p[1] = _offset >> 8;
                p += l;
            }
        }
        int l = immediateBytes();
        if (l > 0) {
            *p = _immediate;
            if (l > 1) {
                p[1] = _immediate >> 8;
                if (l == 4) {
                    p[2] = _immediate >> 16;
                    p[3] = _immediate >> 24;
                }
            }
        }
    }
    bool isGroup()
    {
        int o = _opcode & 0xfe;
        return o == 0xf6 || o == 0xfe || o == 0x80 || o == 0x82;
    }  
    void write()
    {
        console.write("{" + hex(_opcode, 2) + ", " + hex(_modrm, 2) + ", " +
            hex(_offset, 4) + ", " + hex(_immediate) + "}");
    }
    Byte opcode() { return _opcode; }
private:
    Byte _opcode;
    Byte _modrm;
    Word _offset;
    DWord _immediate;
};

class Test
{
public:
    Test(int queueFiller = 0, int nops = 0)
      : _queueFiller(queueFiller), _nops(nops), _address(0)
    { }
    void addInstruction(Instruction instruction)
    {
        Byte opcode = instruction.opcode();
        switch (opcode) {
            case 0x07: case 0x0f: case 0x17: case 0x1f: // POP segreg
                _preamble.append(opcode - 1);  // PUSH segreg
                break;
            case 0x58: case 0x59: case 0x5a: case 0x5b:
            case 0x5c: case 0x5d: case 0x5e: case 0x5f: // POP rw
                _preamble.append(opcode - 8);  // PUSH rw
                break;
            case 0x8f: // POP rmw
                _preamble.append(0x50);  // PUSH AX
                break;
            case 0x9d: // POPF
                _preamble.append(0x9c);  // PUSHF
                break;
            case 0x9a: // CALL cp
            case 0xea: // JMP cp
                _fixups.append(static_cast<Word>(0x0103));
                _fixups.append(static_cast<Word>(0x0302));
                break;
            case 0xc0: case 0xc2: // RET iw
                _fixups.append(static_cast<Word>(0x0301));
                break;
            case 0xc1: case 0xc3: // RET
                _fixups.append(static_cast<Word>(0x0101));
                break;
            case 0xc8: case 0xca: // RETF iw
                _fixups.append(static_cast<Word>(0x0000));
                _fixups.append(static_cast<Word>(0x0301));
                break;
            case 0xc9: case 0xcb: // RETF
                _fixups.append(static_cast<Word>(0x0000));
                _fixups.append(static_cast<Word>(0x0101));
                break;
            case 0xcf: // IRET
                _fixups.append(static_cast<Word>(0x0000));  // flags
                _fixups.append(static_cast<Word>(0x0000));
                _fixups.append(static_cast<Word>(0x0101));
                break;
        }

        _address += instruction.length();
        _instructions.append(instruction);
    }
    int codeLength()  // Just the code bytes
    {
        int l = 0;
        for (auto i : _instructions)
            l += i.length();
        return l;
    }
    int length()  // Entire test
    {
        return 4 + _preamble.count() + 1 + codeLength() + 1 +
            2*_fixups.count();
    }
    void output(Byte* p)       // For the real hardware
    {
        *p = (_queueFiller << 5) + _nops;
        ++p;
        int pc = _preamble.count();
        *p = pc;
        ++p;
        for (int i = 0; i < pc; ++i) {
            *p = _preamble[i];
            ++p;
        }
        int l = codeLength();
        *p = l;
        ++p;
        outputBytes(p);
        p += l;
        int f = _fixups.count();
        *p = f;
        ++p;
        for (int i = 0; i < f; ++i) {
            *p = static_cast<Byte>(_fixups[i]);
            p[1] = _fixups[i] >> 8;
            p += 2;
        }
    }
    Byte* outputBytes(Byte* p)
    {
        for (auto i : _instructions) {
            int l = i.length();
            i.output(p);
            p += l;
        }
        return p;
    }
    Byte* outputCode(Byte* p, Word* newSp)  // For the emulator
    {
        Byte* pStart = p;
        Word sp = 0;

        int pc = _preamble.count();
        for (int i = 0; i < pc; ++i)
            p[i] = _preamble[i];
        p += pc;

        int ql = 0;
        switch (_queueFiller) {
            case 0:
                p[0] = 0xb0;
                p[1] = 0x00;
                p[2] = 0xf6;
                p[3] = 0xe0;
                ql = 4;
                break;
            default:
                throw Exception("Unknown queue filler.");
        }
        p += ql;
        for (int i = 0; i < _nops; ++i) {
            *p = 0x90;
            ++p;
        }
        Word instructionsOffset = ql + _nops;
        p = outputBytes(p);
        for (int i = 0; i < _fixups.count(); ++i) {
            Word f = _fixups[i];
            Word di = instructionsOffset + (f >> 8);
            switch (f & 0xff) {
                case 0:  // pushSegment
                    sp -= 2;
                    pStart[sp] = (Byte)testSegment;
                    pStart[sp + 1] = testSegment >> 8;
                    break;
                case 1:  // pushOffset
                    sp -= 2;
                    pStart[sp] = static_cast<Byte>(di);
                    pStart[sp + 1] = di >> 8;
                    break;
                case 2:  // storeSegment
                    pStart[di] = (Byte)testSegment;
                    pStart[di + 1] = testSegment >> 8;
                    break;
                case 3:  // storeOffset4
                    pStart[di] = di + 4;
                    pStart[di + 1] = (di + 4) >> 8;
                    break;
            }
        }

        p[0] = 0xeb;
        p[1] = 0x00;
        p[2] = 0xcd;
        p[3] = 0xff;
        for (int i = 0; i < 4; ++i)
            p[i + 4] = 0;
        *newSp = sp;
        return p + 4;
    }
    void write()
    {
        bool first = true;
        console.write("{{");
        for (auto i : _instructions) {
            if (!first)
                console.write(",\n");
            else
                console.write("  ");
            i.write();
            first = false;
        }
        console.write("}, " + decimal((_queueFiller << 5) + _nops) + "}\n");
    }
    void setCycles(int cycles) { _cycles = cycles; }
    int cycles() { return _cycles; }
private:
    UInt32 _address;
    int _queueFiller;
    int _nops;

    AppendableArray<Byte> _preamble;
    AppendableArray<Instruction> _instructions;
    AppendableArray<Word> _fixups;
    int _cycles;
};

class Disassembler
{
public:
    Disassembler() : _byteCount(0) { }
    String disassemble(Byte byte, bool firstByte)
    {
        String bytes;
        if (firstByte) {
            if (_byteCount != 0)
                bytes = "!a";
            _byteCount = 0;
        }
        _code[_byteCount] = byte;
        ++_byteCount;
        _lastOffset = 0;
        String instruction = disassembleInstruction();
        if (_lastOffset >= _byteCount)
            return bytes;  // We don't have the complete instruction yet
        _byteCount = 0;
        for (int i = 0; i <= _lastOffset; ++i)
            bytes += hex(_code[i], 2, false);
        return bytes.alignLeft(12) + " " + instruction;
    }
private:
    String disassembleInstruction()
    {
        _wordSize = (opcode() & 1) != 0;
        _doubleWord = false;
        _offset = 1;
        if ((opcode() & 0xc4) == 0)
            return alu(op1()) + regMemPair();
        if ((opcode() & 0xc6) == 4)
            return alu(op1()) + accum() + ", " + imm();
        if ((opcode() & 0xe7) == 6)
            return "PUSH " + segreg(op1());
        if ((opcode() & 0xe7) == 7)
            return "POP " + segreg(op1());
        if ((opcode() & 0xe7) == 0x26)
            return segreg(op1() & 3) + ":";
        if ((opcode() & 0xf8) == 0x40)
            return "INC " + rwo();
        if ((opcode() & 0xf8) == 0x48)
            return "DEC " + rwo();
        if ((opcode() & 0xf8) == 0x50)
            return "PUSH " + rwo();
        if ((opcode() & 0xf8) == 0x58)
            return "POP " + rwo();
        if ((opcode() & 0xfc) == 0x80)
            return alu(reg()) + ea() + ", " +
            (opcode() == 0x81 ? iw(true) : sb(true));
        if ((opcode() & 0xfc) == 0x88)
            return "MOV " + regMemPair();
        if ((opcode() & 0xf8) == 0x90)
            if (opcode() == 0x90)
                return "NOP";
            else
                return "XCHG AX, " + rwo();
        if ((opcode() & 0xf8) == 0xb0)
            return "MOV " + rbo() + ", " + ib();
        if ((opcode() & 0xf8) == 0xb8)
            return "MOV " + rwo() + ", " + iw();
        if ((opcode() & 0xfc) == 0xd0) {
            static String shifts[8] = {
                "ROL", "ROR", "RCL", "RCR", "SHL", "SHR", "SHL", "SAR"};
            return shifts[reg()] + " " + ea() + ", " +
                ((op0() & 2) == 0 ? String("1") : byteRegs(1));
        }
        if ((opcode() & 0xf8) == 0xd8) {
            _wordSize = false;
            _doubleWord = true;
            return String("ESC ") + op0() + ", " + reg() + ", " + ea();
        }
        if ((opcode() & 0xf6) == 0xe4)
            return "IN " + accum() + ", " + port();
        if ((opcode() & 0xf6) == 0xe6)
            return "OUT " + port() + ", " + accum();
        if ((opcode() & 0xe0) == 0x60) {
            static String conds[16] = {
                "O", "NO", "B", "AE", "E", "NE", "BE", "A",
                "S", "NS", "P", "NP", "L", "GE", "LE", "G"};
            return "J" + conds[opcode() & 0xf] + " " + cb();
        }
        switch (opcode()) {
            case 0x27: return "DAA";
            case 0x2f: return "DAS";
            case 0x37: return "AAA";
            case 0x3f: return "AAS";
            case 0x84:
            case 0x85: return "TEST " + regMemPair();
            case 0x86:
            case 0x87: return "XCHG " + regMemPair();
            case 0x8c:
                _wordSize = true;
                return "MOV " + ea() + ", " + segreg(reg());
            case 0x8d:
                _doubleWord = true;
                _wordSize = false;
                return "LEA " + rw() + ", " + ea();
            case 0x8e:
                _wordSize = true;
                return "MOV " + segreg(reg()) + ", " + ea();
            case 0x8f: return "POP " + ea();
            case 0x98: return "CBW";
            case 0x99: return "CWD";
            case 0x9a: return "CALL " + cp();
            case 0x9b: return "WAIT";
            case 0x9c: return "PUSHF";
            case 0x9d: return "POPF";
            case 0x9e: return "SAHF";
            case 0x9f: return "LAHF";
            case 0xa0:
            case 0xa1: return "MOV " + accum() + ", " + size() + "[" + iw() + "]";
            case 0xa2:
            case 0xa3: return "MOV " + size() + "[" + iw() + "], " + accum();
            case 0xa4:
            case 0xa5: return "MOVS" + size();
            case 0xa6:
            case 0xa7: return "CMPS" + size();
            case 0xa8:
            case 0xa9: return "TEST " + accum() + ", " + imm();
            case 0xaa:
            case 0xab: return "STOS" + size();
            case 0xac:
            case 0xad: return "LODS" + size();
            case 0xae:
            case 0xaf: return "SCAS" + size();
            case 0xc0:
            case 0xc2: return "RET " + iw();
            case 0xc1:
            case 0xc3: return "RET";
            case 0xc4: _doubleWord = true; return "LDS " + rw() + ", " + ea();
            case 0xc5:
                _doubleWord = true;
                _wordSize = false;
                return "LES " + rw() + ", " + ea();
            case 0xc6:
            case 0xc7: return "MOV " + ea() + ", " + imm(true);
            case 0xc8:
            case 0xca: return "RETF " + iw();
            case 0xc9:
            case 0xcb: return "RETF";
            case 0xcc: return "INT 3";
            case 0xcd: return "INT " + ib();
            case 0xce: return "INTO";
            case 0xcf: return "IRET";
            case 0xd4: return "AAM " + ib();
            case 0xd5: return "AAD " + ib();
            case 0xd6: return "SALC";
            case 0xd7: return "XLATB";
            case 0xe0: return "LOOPNE " + cb();
            case 0xe1: return "LOOPE " + cb();
            case 0xe2: return "LOOP " + cb();
            case 0xe3: return "JCXZ " + cb();
            case 0xe8: return "CALL " + cw();
            case 0xe9: return "JMP " + cw();
            case 0xea: return "JMP " + cp();
            case 0xeb: return "JMP " + cb();
            case 0xf0:
            case 0xf1: return "LOCK";
            case 0xf2: return "REPNE ";
            case 0xf3: return "REP ";
            case 0xf4: return "HLT";
            case 0xf5: return "CMC";
            case 0xf6:
            case 0xf7:
                switch (reg()) {
                    case 0:
                    case 1: return "TEST " + ea() + ", " + imm(true);
                    case 2: return "NOT " + ea();
                    case 3: return "NEG " + ea();
                    case 4: return "MUL " + ea();
                    case 5: return "IMUL " + ea();
                    case 6: return "DIV " + ea();
                    case 7: return "IDIV " + ea();
                }
            case 0xf8: return "CLC";
            case 0xf9: return "STC";
            case 0xfa: return "CLI";
            case 0xfb: return "STI";
            case 0xfc: return "CLD";
            case 0xfd: return "STD";
            case 0xfe:
            case 0xff:
                switch (reg()) {
                    case 0: return "INC " + ea();
                    case 1: return "DEC " + ea();
                    case 2: return "CALL " + ea();
                    case 3: _doubleWord = true; return "CALL " + ea();
                    case 4: return "JMP " + ea();
                    case 5: _doubleWord = true; return "JMP " + ea();
                    case 6: return "PUSH " + ea();
                    case 7: return "??? " + ea();
                }
        }
        return "!b";
    }
    UInt8 getByte(int offset)
    {
        _lastOffset = max(_lastOffset, offset);
        return _code[offset];
    }
    UInt16 getWord(int offset)
    {
        return getByte(offset) | (getByte(offset + 1) << 8);
    }
    String regMemPair()
    {
        if ((op0() & 2) == 0)
            return ea() + ", " + r();
        return r() + ", " + ea();
    }
    String r() { return !_wordSize ? rb() : rw(); }
    String rb() { return byteRegs(reg()); }
    String rw() { return wordRegs(reg()); }
    String rbo() { return byteRegs(op0()); }
    String rwo() { return wordRegs(op0()); }
    String byteRegs(int r)
    {
        static String b[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
        return b[r];
    }
    String wordRegs(int r)
    {
        static String w[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
        return w[r];
    }
    String ea()
    {
        String s;
        switch (mod()) {
            case 0: s = disp(); break;
            case 1: s = disp() + sb(); _offset = 3; break;
            case 2: s = disp() + "+" + iw(); _offset = 4; break;
            case 3: return !_wordSize ? byteRegs(rm()) : wordRegs(rm());
        }
        return size() + "[" + s + "]";
    }
    String size()
    {
        if (!_doubleWord)
            return (!_wordSize ? "B" : "W");
        else
            return (!_wordSize ? "" : "D");
    }
    String disp()
    {
        static String d[8] = {
            "BX+SI", "BX+DI", "BP+SI", "BP+DI", "SI", "DI", "BP", "BX"};
        if (mod() == 0 && rm() == 6) {
            String s = iw();
            _offset = 4;
            return s;
        }
        return d[rm()];
    }
    String alu(int op)
    {
        static String o[8] = {
            "ADD ", "OR ", "ADC ", "SBB ", "AND ", "SUB ", "XOR ", "CMP "};
        return o[op];
    }
    Byte opcode() { return getByte(0); }
    int op0() { return opcode() & 7; }
    int op1() { return (opcode() >> 3) & 7; }
    Byte modRM() { _offset = 2; return getByte(1); }
    int mod() { return modRM() >> 6; }
    int reg() { return (modRM() >> 3) & 7; }
    int rm() { return modRM() & 7; }
    String imm(bool m = false) { return !_wordSize ? ib(m) : iw(m); }
    String iw(bool m = false)
    {
        if (m)
            ea();
        return hex(getWord(_offset), 4, false);
    }
    String ib(bool m = false)
    {
        if (m)
            ea();
        return hex(getByte(_offset), 2, false);
    }
    String sb(bool m = false)
    {
        if (m)
            ea();
        UInt8 byte = getByte(_offset);
        if ((byte & 0x80) == 0)
            return "+" + hex(byte, 2, false);
        return "-" + hex(-byte, 2, false);
    }
    String accum() { return !_wordSize ? "AL" : "AX"; }
    String segreg(int r)
    {
        static String sr[8] = {"ES", "CS", "SS", "DS", "??", "??", "??", "??"};
        return sr[r];
    }
    String cb()
    {
        return "IP" + sb();
        //hex(_address + static_cast<SInt8>(getByte(_offset)), 4, false);
    }
    String cw()
    {
        return "IP+" + iw();
        //return hex(_address + getWord(_offset), 4, false);
    }
    String cp()
    {
        return hex(getWord(_offset + 2), 4, false) + ":" +
            hex(getWord(_offset), 4, false);
    }
    String port() { return ((op1() & 1) == 0 ? ib() : wordRegs(2)); }

    UInt16 _ip;
    UInt8 _code[6];
    int _byteCount;
    bool _wordSize;
    bool _doubleWord;
    int _offset;
    int _lastOffset;
};

class SnifferDecoder
{
public:
    void reset()
    {
        _cpu_rqgt0 = false;  // Used by 8087 for bus mastering, NYI
        _cpu_ready = true;   // Used for DMA and wait states, NYI
        _cpu_test = false;   // Used by 8087 for synchronization, NYI
        _bus_dma = 0;        // NYI
        _bus_irq = 0xfc;     // NYI
        _bus_iochrdy = true; // Used for wait states, NYI
        _bus_aen = false;    // Used for DMA, NYI
        _bus_tc = false;     // Used for DMA, NYI

        _t = 0;  
        _tNext = 0;
        _d = -1;  
        _queueLength = 0;
        _lastS = 0;
        _pitCycle = 0;
        _cpu_s = 7;
    }
    String getLine()
    {
        _bus_pit = (_pitCycle & 3) < 2 ? 4 : 5;

        static const char qsc[] = ".IES";
        static const char sc[] = "ARWHCrwp";
        String line;
        if (_cpuDataFloating)
            line = String(hex(_cpu_ad >> 8, 3, false)) + "??";
        else
            line = String(hex(_cpu_ad, 5, false));
        line += " " +
            codePoint(qsc[_cpu_qs]) + codePoint(sc[_cpu_s]) +
            (_cpu_rqgt0 ? "G" : ".") + (_cpu_ready ? "." : "z") +
            (_cpu_test ? "T" : ".") +
            "  " + hex(_bus_address, 5, false) + " ";
        if (_isaDataFloating)
            line += "??";
        else
            line += hex(_bus_data, 2, false);
        line += " " + hex(_bus_dma, 2, false) +
            " " + hex(_bus_irq, 2, false) + " " +
            hex(_bus_pit, 1, false) + " " + (_bus_ior ? "R" : ".") +
            (_bus_iow ? "W" : ".") + (_bus_memr ? "r" : ".") +
            (_bus_memw ? "w" : ".") + (_bus_iochrdy ? "." : "z") +
            (_bus_aen ? "D" : ".") +
            (_bus_tc ? "T" : ".");
        line += "  ";
        if (_cpu_s != 7 && _cpu_s != 3)
            switch (_tNext) {
                case 0:
                case 4:
                    // T1 state occurs after transition out of passive
                    _tNext = 1;
                    break;
                case 1:
                    _tNext = 2;
                    break;
                case 2:
                    _tNext = 3;
                    break;
                case 3:
                    _tNext = 5;
                    break;
            }
        else
            switch (_t) {
                case 4:
                    _d = -1;
                case 0:
                    _tNext = 0;
                    break;
                case 1:
                case 2:
                    _tNext = 6;
                    break;
                case 3:
                case 5:
                    _d = -1;
                    _tNext = 4;
                    break;
            }
        switch (_t) {
            case 0: line += "  "; break;
            case 1: line += "T1"; break;
            case 2: line += "T2"; break;
            case 3: line += "T3"; break;
            case 4: line += "T4"; break;
            case 5: line += "Tw"; break;
            default: line += "!c"; _tNext = 0; break;
        }
        line += " ";
        if (_bus_aen)
            switch (_d) {
                // This is a bit of a hack since we don't have access
                // to the right lines to determine the DMA state
                // properly. This probably breaks for memory-to-memory
                // copies.
                case -1: _d = 0; break;
                case 0: _d = 1; break;
                case 1: _d = 2; break;
                case 2: _d = 3; break;
                case 3:
                case 5:
                    if ((_bus_iow && _bus_memr) || (_bus_ior && _bus_memw))
                        _d = 4;
                    else
                        _d = 5;
                    break;
                case 4:
                    _d = -1;
            }
        switch (_d) {
            case -1: line += "  "; break;
            case 0: line += "S0"; break;
            case 1: line += "S1"; break;
            case 2: line += "S2"; break;
            case 3: line += "S3"; break;
            case 4: line += "S4"; break;
            case 5: line += "SW"; break;
            default: line += "!d"; _t = 0; break;
        }
        line += " ";
        String instruction;
        if (_cpu_qs != 0) {
            if (_cpu_qs == 2)
                _queueLength = 0;
            else {
                Byte b = _queue[0];
                for (int i = 0; i < 3; ++i)
                    _queue[i] = _queue[i + 1];
                --_queueLength;
                if (_queueLength < 0) {
                    line += "!g";
                    _queueLength = 0;
                }
                instruction = _disassembler.disassemble(b, _cpu_qs == 1);
            }
        }
        if (_tNext == 4 || _d == 4) {
            if (_tNext == 4 && _d == 4)
                line += "!e";
            String seg;
            switch (_cpu_ad & 0x30000) {
                case 0x00000: seg = "ES "; break;
                case 0x10000: seg = "SS "; break;
                case 0x20000: seg = "CS "; break;
                case 0x30000: seg = "DS "; break;
            }
            String type = "-";
            if (_lastS == 0)
                line += hex(_bus_data, 2, false) + " <-i           ";
            else {
                if (_lastS == 4) {
                    type = "f";
                    seg = "   ";
                }
                if (_d == 4) {
                    type = "d";
                    seg = "   ";
                }
                line += hex(_bus_data, 2, false) + " ";
                if (_bus_ior || _bus_memr)
                    line += "<-" + type + " ";
                else
                    line += type + "-> ";
                if (_bus_memr || _bus_memw)
                    line += "[" + seg + hex(_bus_address, 5, false) + "]";
                else
                    line += "port[" + hex(_bus_address, 4, false) + "]";
                if (_lastS == 4 && _d != 4) {
                    if (_queueLength >= 4)
                        line += "!f";
                    else {
                        _queue[_queueLength] = _bus_data;
                        ++_queueLength;
                    }
                }
            }
            line += " ";
        }
        else
            line += "                  ";
        if (_cpu_qs != 0)
            line += codePoint(qsc[_cpu_qs]);
        else
            line += " ";
        line += " " + instruction + "\n";
        _lastS = _cpu_s;
        _t = _tNext;
        if (_t == 4 || _d == 4) {
            _bus_ior = false;
            _bus_iow = false;
            _bus_memr = false;
            _bus_memw = false;
        }
        ++_pitCycle;
        _cpu_qs = 0;
        return line;
    }
    void queueOperation(int qs) { _cpu_qs = qs; }
    void setStatus(int s) { _cpu_s = s; }
    void setStatusHigh(int segment)
    {
        _cpu_ad &= 0xcffff;
        switch (segment) {
            case 0:  // ES
                break;
            case 1:  // CS or none
                _cpu_ad |= 0x20000;
                break;
            case 2:  // SS
                _cpu_ad |= 0x10000;
                break;
            case 3:  // DS
                _cpu_ad |= 0x30000;
                break;
        }
        setBusFloating();
    }
    void setInterruptFlag(bool intf)
    {
        _cpu_ad = (_cpu_ad & 0xbffff) | (intf ? 0x40000 : 0);
    }
    void setBusOperation(int s)
    {
        switch (s) {
            case 1: _bus_ior = true; break;
            case 2: _bus_iow = true; break;
            case 4:
            case 5: _bus_memr = true; break;
            case 6: _bus_memw = true; break;
        }
    }
    void setData(Byte data)
    {
        _cpu_ad = (_cpu_ad & 0xfff00) | data;
        _bus_data = data;
        _cpuDataFloating = false;
        _isaDataFloating = false;
    }
    void setAddress(UInt32 address)
    {
        _cpu_ad = address;
        _bus_address = address;
        _cpuDataFloating = false;
    }
    void setBusFloating()
    {
        _cpuDataFloating = true;
        _isaDataFloating = true;
    }
private:
    Disassembler _disassembler;

    // Internal variables that we use to keep track of what's going on in order
    // to be able to print useful logs.
    int _t;  // 0 = Tidle, 1 = T1, 2 = T2, 3 = T3, 4 = T4, 5 = Tw
    int _tNext;
    int _d;  // -1 = SI, 0 = S0, 1 = S1, 2 = S2, 3 = S3, 4 = S4, 5 = SW
    Byte _queue[4];
    int _queueLength;
    int _lastS;
    int _pitCycle;

    // These represent the CPU and ISA bus pins used to create the sniffer
    // logs.
    UInt32 _cpu_ad;
    // A19/S6        O ADDRESS/STATUS: During T1, these are the four most significant address lines for memory operations. During I/O operations, these lines are LOW. During memory and I/O operations, status information is available on these lines during T2, T3, Tw, and T4. S6 is always low.
    // A18/S5        O The status of the interrupt enable flag bit (S5) is updated at the beginning of each clock cycle.
    // A17/S4        O  S4*2+S3 0 = Alternate Data, 1 = Stack, 2 = Code or None, 3 = Data
    // A16/S3        O
    // A15..A8       O ADDRESS BUS: These lines provide address bits 8 through 15 for the entire bus cycle (T1�T4). These lines do not have to be latched by ALE to remain valid. A15�A8 are active HIGH and float to 3-state OFF during interrupt acknowledge and local bus ``hold acknowledge''.
    // AD7..AD0     IO ADDRESS DATA BUS: These lines constitute the time multiplexed memory/IO address (T1) and data (T2, T3, Tw, T4) bus. These lines are active HIGH and float to 3-state OFF during interrupt acknowledge and local bus ``hold acknowledge''.
    UInt8 _cpu_qs;
    // QS0           O QUEUE STATUS: provide status to allow external tracking of the internal 8088 instruction queue. The queue status is valid during the CLK cycle after which the queue operation is performed.
    // QS1           0 = No operation, 1 = First Byte of Opcode from Queue, 2 = Empty the Queue, 3 = Subsequent Byte from Queue
    UInt8 _cpu_s;
    // -S0           O STATUS: is active during clock high of T4, T1, and T2, and is returned to the passive state (1,1,1) during T3 or during Tw when READY is HIGH. This status is used by the 8288 bus controller to generate all memory and I/O access control signals. Any change by S2, S1, or S0 during T4 is used to indicate the beginning of a bus cycle, and the return to the passive state in T3 and Tw is used to indicate the end of a bus cycle. These signals float to 3-state OFF during ``hold acknowledge''. During the first clock cycle after RESET becomes active, these signals are active HIGH. After this first clock, they float to 3-state OFF.
    // -S1           0 = Interrupt Acknowledge, 1 = Read I/O Port, 2 = Write I/O Port, 3 = Halt, 4 = Code Access, 5 = Read Memory, 6 = Write Memory, 7 = Passive
    // -S2
    bool _cpu_rqgt0;    // -RQ/-GT0 !87 IO REQUEST/GRANT: pins are used by other local bus masters to force the processor to release the local bus at the end of the processor's current bus cycle. Each pin is bidirectional with RQ/GT0 having higher priority than RQ/GT1. RQ/GT has an internal pull-up resistor, so may be left unconnected.
    bool _cpu_ready;    // READY        I  READY: is the acknowledgement from the addressed memory or I/O device that it will complete the data transfer. The RDY signal from memory or I/O is synchronized by the 8284 clock generator to form READY. This signal is active HIGH. The 8088 READY input is not synchronized. Correct operation is not guaranteed if the set up and hold times are not met.
    bool _cpu_test;     // -TEST        I  TEST: input is examined by the ``wait for test'' instruction. If the TEST input is LOW, execution continues, otherwise the processor waits in an ``idle'' state. This input is synchronized internally during each clock cycle on the leading edge of CLK.
    UInt32 _bus_address;
    // +A19..+A0      O Address bits: These lines are used to address memory and I/O devices within the system. These lines are generated by either the processor or DMA controller.
    UInt8 _bus_data;
    // +D7..+D0      IO Data bits: These lines provide data bus bits 0 to 7 for the processor, memory, and I/O devices.
    UInt8 _bus_dma;
    // +DRQ0 JP6/1 == U28.19 == U73.9
    // +DRQ1..+DRQ3  I  DMA Request: These lines are asynchronous channel requests used by peripheral devices to gain DMA service. They are prioritized with DRQ3 being the lowest and DRQl being the highest. A request is generated by bringing a DRQ line to an active level (high). A DRQ line must be held high until the corresponding DACK line goes active.
    // -DACK0..-DACK3 O -DMA Acknowledge: These lines are used to acknowledge DMA requests (DRQ1-DRQ3) and to refresh system dynamic memory (DACK0). They are active low.
    UInt8 _bus_irq;
    // +IRQ2..+IRQ7  I  Interrupt Request lines: These lines are used to signal the processor that an I/O device requires attention. An Interrupt Request is generated by raising an IRQ line (low to high) and holding it high until it is acknowledged by the processor (interrupt service routine).
    UInt8 _bus_pit;     // clock, gate, output
    bool _bus_ior;      // -IOR         O -I/O Read Command: This command line instructs an I/O device to drive its data onto the data bus. It may be driven by the processor or the DMA controller. This signal is active low.
    bool _bus_iow;      // -IOW         O -I/O Write Command: This command line instructs an I/O device to read the data on the data bus. It may be driven by the processor or the DMA controller. This signal is active low.
    bool _bus_memr;     // -MEMR        O Memory Read Command: This command line instructs the memory to drive its data onto the data bus. It may be driven by the processor or the DMA controller. This signal is active low.
    bool _bus_memw;     // -MEMW        O Memory Write Command: This command line instructs the memory to store the data present on the data bus. It may be driven by the processor or the DMA controller. This signal is active low.
    bool _bus_iochrdy;  // +I/O CH RDY I  I/O Channel Ready: This line, normally high (ready), is pulled low (not ready) by a memory or I/O device to lengthen I/O or memory cycles. It allows slower devices to attach to the I/O channel with a minimum of difficulty. Any slow device using this line should drive it low immediately upon detecting a valid address and a read or write command. This line should never be held low longer than 10 clock cycles. Machine cycles (I/O or memory) are extended by an integral number of CLK cycles (210 ns).
    bool _bus_aen;      // +AEN         O Address Enable: This line is used to de-gate the processor and other devices from the I/O channel to allow DMA transfers to take place. When this line is active (high), the DMA controller has control of the address bus, data bus, read command lines (memory and I/O), and the write command lines (memory and I/O).
    bool _bus_tc;       // +T/C         O Terminal Count: This line provides a pulse when the terminal count for any DMA channel is reached. This signal is active high.
    bool _cpuDataFloating;
    bool _isaDataFloating;
};

class Emulator
{
public:
    Emulator() : _logging(false), _ram(0xa0000)
    {
        static String b[8] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
        static String w[8] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
        static String s[8] = {"ES", "CS", "SS", "DS", "??", "??", "??", "??"};
        for (int i = 0; i < 8; ++i) {
            _wordRegisters[i].init(w[i], &_registerData[i]);
            _byteRegisters[i].init(b[i], reinterpret_cast<UInt8*>(
                &_registerData[i & 3]) + (i >= 4 ? 1 : 0));
            _segmentRegisters[i].init(s[i], &_segmentRegisterData[i]);
        }
        _flags.init("F", &_flagsData);
    }
    String log(Test test)
    {
        _logging = true;
        _test = test;
        run();
        return _log;
    }
    int expected(Test test)
    {
        _logging = false;
        _test = test;
        try {
            run();
        } catch (...) { }
        return _cycle;
    }
private:
    void run()
    {
        _cycle = 0;
        ax() = 0;
        cx() = 0;
        dx() = 0;
        bx() = 0;
        bp() = 0;
        si() = 0;
        di() = 0;
        es() = testSegment;
        cs() = testSegment;
        ss() = testSegment;
        ds() = testSegment;

        Word newSP = 0;
        Byte* stopP = _test.outputCode(&_ram[testSegment << 4], &newSP);
        sp() = newSP;
        _stopIP = stopP - &_ram[(testSegment << 4) + 2];

        _busState = tIdle;
        _queueCycle = 0;
        _ready = true;
        _ioInProgress._type = ioPassive;
        _ioNext = _ioInProgress;
        _lastIO = ioPassive;
        _snifferDecoder.reset();
        _prefetchedEU = 0;
        _prefetchedBIU = 0;
        _prefetchedRemove = false;
        _statusSet = false;
        _prefetching = true;
        _log = "";
        _logSkip = 3;
        _synchronousDone = true;
        _ip = 0;
        _segmentOverride = -1;
        _prefetchAddress = 0;
        _rep = 0;
        _repeating = false;

        do {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
            executeOneInstruction();
        } while (_ip != _stopIP && _cycle < 2048);
    }
    void wait(int cycles)
    {
        while (cycles > 0) {
            _snifferDecoder.setInterruptFlag(intf());
            bool write = _ioInProgress._type == ioWriteMemory ||
                _ioInProgress._type == ioWritePort;
            switch (_busState) {
                case t1:
                    _snifferDecoder.setStatusHigh(_ioInProgress._segment);
                    _snifferDecoder.setBusOperation((int)_ioInProgress._type);
                    if (write) {
                        _snifferDecoder.setData(_ioInProgress._data);
                        _synchronousDone = true;
                    }
                    _busState = t2;
                    break;
                case t2:
                    _snifferDecoder.setStatus((int)ioPassive);
                    // TODO: Need to handle wait states
                    switch (_ioInProgress._type) {
                        case ioInterruptAcknowledge:
                            _ioInProgress._data = 8;
                            break;
                        case ioReadPort:
                            _ioInProgress._data = 0xff;
                            break;
                        case ioReadMemory:
                            _ioInProgress._data = _ram[_ioInProgress._address];
                            break;
                        case ioCodeAccess:
                            _ioInProgress._data = _ram[_ioInProgress._address];
                            _prefetchQueue[
                                (_prefetchOffset + _prefetchedBIU) & 3] =
                                _ioInProgress._data;
                            ++_prefetchAddress;
                            ++_prefetchedBIU;
                            _queueCycle = 3;
                            break;
                    }
                    if (_ioInProgress._type != ioCodeAccess)  // This happens at the last Tw/T3 state
                        _synchronousDone = true;
                    if (!write)
                        _snifferDecoder.setData(_ioInProgress._data);
                    _busState = t3;
                    break;
                case t3:
                    _statusSet = false;
                    switch (_ioInProgress._type) {
                        case ioWritePort:
                            break;
                        case ioWriteMemory:
                            _ram[_ioInProgress._address] = _ioInProgress._data;
                            break;
                    }
                    // Fall through
                case tWait:
                    _busState = tWait;
                    if (!_ready)
                        break;
                    _busState = t4;
                    break;
                case t4:
                    _busState = tFirstIdle;
                    break;
                case tFirstIdle:
                    _busState = tIdle;
                    break;
                case tIdle:
                    _lastIO = ioPassive;
                    break;
            }
            if (_busState == tFirstIdle || _busState == tIdle) {
                _snifferDecoder.setBusFloating();
                if (_statusSet) {
                    _ioInProgress = _ioNext;
                    _ioNext._type = ioPassive;
                    if (_ioInProgress._type != ioPassive) {
                        _busState = t1;
                        _lastIO = _ioInProgress._type;
                        _snifferDecoder.setAddress(_ioInProgress._address);
                    }
                }
            }
            if (!_statusSet && _busState != tFirstIdle) {
                if (_ioNext._type == ioPassive && _prefetchedBIU < 4 &&
                    _prefetching && (_lastIO != ioCodeAccess || _busState == t4 || _busState == t3 || _busState == tWait)) {
                    _ioNext._type = ioCodeAccess;
                    _ioNext._address = physicalAddress(1, _prefetchAddress);
                    _ioNext._segment = 1;
                }
                if (_ioNext._type != ioPassive) {
                    _snifferDecoder.setStatus((int)_ioNext._type);
                    _statusSet = true;
                }
            }
            if (_queueCycle > 0) {
                --_queueCycle;
                if (_queueCycle == 0)
                    ++_prefetchedEU;
            }
            if (_prefetchedRemove) {
                --_prefetchedBIU;
                _prefetchOffset = (_prefetchOffset + 1) & 3;
                _prefetchedRemove = false;
            }
            if (_logging) {
                String l = _snifferDecoder.getLine();
                if (_logSkip > 0)
                    --_logSkip;
                else
                    _log += l;
            }
            ++_cycle;
            --cycles;
        }
    }
    enum IOType
    {
        ioInterruptAcknowledge = 0,
        ioReadPort = 1,
        ioWritePort = 2,
        ioHalt = 3,
        ioCodeAccess = 4,
        ioReadMemory = 5,
        ioWriteMemory = 6,
        ioPassive = 7
    };
    void initIO(IOType type, UInt32 address)
    {
        _ioNext._segment = _segment;
        if (_segmentOverride != -1)
            _ioNext._segment = _segmentOverride;
        _ioNext._address = physicalAddress(_ioNext._segment, address);
        _ioNext._data = _data;
        _ioNext._type = type;
    }
    Byte busAccess(IOType type)
    {
        while (_ioNext._type != ioPassive)
            wait(1);
        initIO(type, _address);
        _synchronousDone = false;
        do {
            wait(1);
        } while (!_synchronousDone);
        return _ioInProgress._data;
    }
    Word busReadWord(IOType type)
    {
        while (_ioNext._type != ioPassive)
            wait(1);
        initIO(type, _address);
        _synchronousDone = false;
        do {
            wait(1);
        } while (_ioNext._type != ioPassive);
        initIO(type, _address + 1);
        do {
            wait(1);
        } while (!_synchronousDone);
        Word result = _ioInProgress._data;
        _synchronousDone = false;
        do {
            wait(1);
        } while (!_synchronousDone);
        return result | (_ioInProgress._data << 8);
    }
    void busWriteWord(IOType type)
    {
        busAccess(ioWriteMemory);
        _data >>= 8;
        ++_address;
        busAccess(ioWriteMemory);
        wait(3);
    }
    Byte fetchInstructionByte()
    {
        // Always wait at least one cycle so we don't fetch more than one byte
        // from the queue per cycle.
        do
            wait(1);
        while (_prefetchedEU == 0);
        UInt8 byte = _prefetchQueue[_prefetchOffset & 3];
        _prefetchedRemove = true;
        --_prefetchedEU;
        _snifferDecoder.queueOperation(3);
        ++_ip;
        return byte;
    }
    Word fetchInstructionWord()
    {
        Byte low = fetchInstructionByte();
        return (fetchInstructionByte() << 8) | low;
    }
    Word fetchInstruction()
    {
        if (_wordSize)
            return fetchInstructionWord();
        return fetchInstructionByte();
    }
    void initEA()
    {
        _modRM = fetchInstructionByte();
        if ((_modRM & 0xc0) == 0xc0) {
            _useMemory = false;
            _address = _modRM & 7;
            return;
        }
        _useMemory = true;
        int w = 2;
        switch (_modRM & 7) {
            case 0: w = 4; _address = bx() + si(); break;
            case 1: w = 5; _address = bx() + di(); break;
            case 2: w = 5; _address = bp() + si(); break;
            case 3: w = 4; _address = bp() + di(); break;
            case 4:        _address =        si(); break;
            case 5:        _address =        di(); break;
            case 6:        _address = bp();        break;
            case 7:        _address = bx();        break;
            default:
                throw Exception("Not yet implemented.");
        }
        wait(1);
        static int segments[8] = {3, 3, 2, 2, 3, 3, 2, 3};
        _segment = segments[_modRM & 7];
        switch (_modRM & 0xc0) {
            case 0x00:
                if ((_modRM & 7) == 6) {
                    _address = fetchInstructionWord();
                    _segment = 3;
                    w = 1;
                }
                break;
            case 0x40:
                wait(w);
                _address += signExtend(fetchInstructionByte());
                w = 3;
                break;
            case 0x80:
                wait(w);
                _address += fetchInstructionWord();
                w = 2;
                break;
        }
        wait(w);
    }
    void busRead()
    {
        if (_wordSize)
            _data = busReadWord(ioReadMemory);
        else
            _data = busAccess(ioReadMemory);
    }
    void readEA()
    {
        initEA();
        if (_useMemory) {
            _prefetching = false;
            wait(2);
            _prefetching = true;
            busRead();
            return;
        }
        if (!_wordSize)
            _data = _byteRegisters[_address];
        else
            _data = _wordRegisters[_address];
    }
    void busWrite()
    {
        if (_wordSize)
            busWriteWord(ioWriteMemory);
        else
            busAccess(ioWriteMemory);
    }
    void writeEA(UInt16 data, int w = 0)
    {
        _data = data;
        wait(w);
        if (_useMemory) {
            busWrite();
            return;
        }
        if (!_wordSize)
            _byteRegisters[_address] = _data;
        else
            _wordRegisters[_address] = _data;
    }

    void executeOneInstruction()
    {
        if (!_repeating) {
            _opcode = fetchInstructionByte();
            _wordSize = ((_opcode & 1) != 0);
            _snifferDecoder.queueOperation(1);
            _sourceIsRM = ((_opcode & 2) != 0);
        }
        bool completed = true;
        switch (_opcode) {
            case 0x00: case 0x01: case 0x02: case 0x03:
            case 0x08: case 0x09: case 0x0a: case 0x0b: 
            case 0x10: case 0x11: case 0x12: case 0x13:
            case 0x18: case 0x19: case 0x1a: case 0x1b:
            case 0x20: case 0x21: case 0x22: case 0x23:
            case 0x28: case 0x29: case 0x2a: case 0x2b: 
            case 0x30: case 0x31: case 0x32: case 0x33:
            case 0x38: case 0x39: case 0x3a: case 0x3b: // alu rm, r / r, rm
                readEA();
                _aluOperation = (_opcode >> 3) & 7;
                if (!_sourceIsRM) {
                    _destination = _data;
                    _source = getReg();
                }
                else {
                    _destination = getReg();
                    _source = _data;
                }
                if (_useMemory)
                    wait(2);
                wait(1);
                doALUOperation();
                if (_aluOperation != 7) {
                    if (!_sourceIsRM) {
                        if (_useMemory)
                            wait(4);
                        writeEA(_data);
                    }
                    else
                        setReg(_data);
                }
                break;
            case 0x04: case 0x05: case 0x0c: case 0x0d:
            case 0x14: case 0x15: case 0x1c: case 0x1d:
            case 0x24: case 0x25: case 0x2c: case 0x2d:
            case 0x34: case 0x35: case 0x3c: case 0x3d: // alu A, imm
                wait(1);
                _data = fetchInstruction();
                _destination = getAccum();
                _source = _data;
                _aluOperation = (_opcode >> 3) & 7;
                doALUOperation();
                if (_aluOperation != 7)
                    setAccum();
                wait(1);
                break;
            case 0x06: case 0x0e: case 0x16: case 0x1e: // PUSH segreg
                push(_segmentRegisters[_opcode >> 3]);
                break;
            case 0x07: case 0x0f: case 0x17: case 0x1f: // POP segreg
                _segmentRegisters[_opcode >> 3] = pop();
                break;
            case 0x26: case 0x2e: case 0x36: case 0x3e: // segreg:
                _segmentOverride = (_opcode >> 3) & 3;
                completed = false;
                wait(1);
                break;
            case 0x27: // DAA
                if (af() || (al() & 0x0f) > 9) {
                    _data = al() + 6;
                    al() = _data;
                    setAF(true);
                    if ((_data & 0x100) != 0)
                        setCF(true);
                }
                if (cf() || al() > 0x9f) {
                    al() += 0x60;
                    setCF(true);
                }
                da();
                break;
            case 0x2f: // DAS
                {
                    UInt8 t = al();
                    if (af() || ((al() & 0xf) > 9)) {
                        _data = al() - 6;
                        al() = _data;
                        setAF(true);
                        if ((_data & 0x100) != 0)
                            setCF(true);
                    }
                    if (cf() || t > 0x9f) {
                        al() -= 0x60;
                        setCF(true);
                    }
                }
                da();
                break;
            case 0x37: // AAA
                if (af() || ((al() & 0xf) > 9)) {
                    al() += 6;
                    ++ah();
                    setCA();
                }
                else
                    clearCA();
                aa();
                break;
            case 0x3f: // AAS
                if (af() || ((al() & 0xf) > 9)) {
                    al() -= 6;
                    --ah();
                    setCA();
                }
                else
                    clearCA();
                aa();
                break;
            case 0x40: case 0x41: case 0x42: case 0x43:
            case 0x44: case 0x45: case 0x46: case 0x47:
            case 0x48: case 0x49: case 0x4a: case 0x4b:
            case 0x4c: case 0x4d: case 0x4e: case 0x4f: // INCDEC rw
                _destination = rw();
                _source = 1;
                _wordSize = true;
                if ((_opcode & 8) == 0) {
                    _data = _destination + _source;
                    setOFAdd();
                }
                else {
                    _data = _destination - _source;
                    setOFSub();
                }
                doAF();
                setPZS();
                rw() = _data;
                wait(1);
                break;
            case 0x50: case 0x51: case 0x52: case 0x53:
            case 0x54: case 0x55: case 0x56: case 0x57: // PUSH rw
                push(rw());
                break;
            case 0x58: case 0x59: case 0x5a: case 0x5b:
            case 0x5c: case 0x5d: case 0x5e: case 0x5f: // POP rw
                rw() = pop();
                break;
            case 0x60: case 0x61: case 0x62: case 0x63:
            case 0x64: case 0x65: case 0x66: case 0x67:
            case 0x68: case 0x69: case 0x6a: case 0x6b:
            case 0x6c: case 0x6d: case 0x6e: case 0x6f:
            case 0x70: case 0x71: case 0x72: case 0x73:
            case 0x74: case 0x75: case 0x76: case 0x77:
            case 0x78: case 0x79: case 0x7a: case 0x7b:
            case 0x7c: case 0x7d: case 0x7e: case 0x7f: // Jcond cb
                wait(1);
                _data = fetchInstructionByte();
                {
                    bool jump;
                    switch (_opcode & 0x0e) {
                        case 0x00: jump =  of(); break;
                        case 0x02: jump =  cf(); break;
                        case 0x04: jump =  zf(); break;
                        case 0x06: jump =  cf() || zf(); break;
                        case 0x08: jump =  sf(); break;
                        case 0x0a: jump =  pf(); break;
                        case 0x0c: jump = (sf() != of()); break;
                        case 0x0e: jump = (sf() != of()) || zf(); break;
                    }
                    if ((_opcode & 1) != 0)
                        jump = !jump;
                    if (jump) {
                        wait(1);
                        jumpShort();
                        // TODO: Put the following two lines into jumpShort() if JMP has them
                        wait(1);
                        _prefetching = true;
                    }
                    wait(1);
                }
                break;
            case 0x80: case 0x81: case 0x82: case 0x83: // alu rm, imm
                readEA();
                _destination = _data;
                if (_useMemory)
                    wait(2);
                if (_opcode == 0x81)
                    _source = fetchInstructionWord();
                else
                    _source = signExtend(fetchInstructionByte());
                _aluOperation = modRMReg();
                doALUOperation();
                if (_useMemory) {
                    wait(2);
                    if (_aluOperation != 7) {
                        _prefetching = false;
                        wait(1);
                        _prefetching = true;
                        wait(1);
                    }
                }
                else
                    wait(3);
                if (_aluOperation != 7)
                    writeEA(_data);
                break;
            case 0x84: case 0x85: // TEST rm, reg
                readEA();
                test(_data, getReg());
                if (_useMemory)
                    wait(2);
                wait(1);
                break;
            case 0x86: case 0x87: // XCHG rm, reg
                readEA();
                _source = getReg();
                setReg(_data);
                wait(2);
                if (_useMemory)
                    wait(6);
                writeEA(_source);
                break;
            case 0x88: case 0x89: // MOV rm, reg
                initEA();
                if (_useMemory) {
                    wait(4);
                    _prefetching = false;
                    wait(2);
                    _prefetching = true;
                }
                writeEA(getReg());
                break;
            case 0x8a: case 0x8b: // MOV reg, rm
                readEA();
                setReg(_data);
                if (_useMemory)
                    wait(2);
                break;
            case 0x8c: // MOV rmw, segreg
                _wordSize = true;
                initEA();
                if (_useMemory) {
                    wait(3);
                    _prefetching = false;
                    wait(2);
                    _prefetching = true;
                }
                writeEA(_segmentRegisters[modRMReg() & 3]);
                break;
            case 0x8d: // LEA rw, rmw
                initEA();
                setReg(_address);
                if (_useMemory)
                    wait(2);
                break;
            case 0x8e: // MOV segreg, rmw
                _wordSize = true;
                readEA();
                _segmentRegisters[modRMReg() & 3] = _data;
                if (_useMemory)
                    wait(2);
                break;
            case 0x8f: // POP rmw
                initEA();
                if (_useMemory)
                    wait(2);
                _source = _address;
                _data = pop();
                _address = _source;
                wait(1);
                if (_useMemory) {
                    wait(2);
                    _prefetching = false;
                    wait(2);
                    _prefetching = true;
                }
                writeEA(_data);
                break;
            case 0x90: case 0x91: case 0x92: case 0x93:
            case 0x94: case 0x95: case 0x96: case 0x97: // XCHG AX, rw
                _data = rw();
                rw() = ax();
                ax() = _data;
                wait(2);
                break;
            case 0x98: // CBW
                ax() = signExtend(al());
                wait(1);
                break;
            case 0x99: // CWD
                dx() = ((ax() & 0x8000) == 0 ? 0x0000 : 0xffff);
                wait(4);
                break;
            case 0x9a: // CALL cd
                {
                    wait(1);
                    UInt16 savedIP = fetchInstructionWord();
                    UInt16 savedCS = fetchInstructionWord();
                    _prefetching = false;
                    wait(1);
                    push(cs());
                    _prefetching = false;
                    wait(4);
                    UInt16 oldIP = _ip;
                    cs() = savedCS;
                    setIP(savedIP);
                    wait(1);
                    _prefetching = true;
                    push2(oldIP);
                    wait(6);
                }
                break;
            case 0x9b: // WAIT
                throw Exception("Not yet implemented.");
                break;
            case 0x9c: // PUSHF
                push((_flags & 0x0fd7) | 0xf000);
                break;
            case 0x9d: // POPF
                _flags = pop() | 2;
                break;
            case 0x9e: // SAHF
                _flags = (_flags & 0xff02) | ah();
                wait(3);
                break;
            case 0x9f: // LAHF
                ah() = _flags & 0xd7;
                wait(1);
                break;
            case 0xa0: case 0xa1: // MOV A, [iw]
                wait(1);
                _address = fetchInstructionWord();
                _prefetching = false;
                wait(2);
                _prefetching = true;
                busRead();
                setAccum();
                break;
            case 0xa2: case 0xa3: // MOV [iw], A
                wait(1);
                _address = fetchInstructionWord();
                _data = getAccum();
                wait(1);
                _prefetching = false;
                wait(2);
                _prefetching = true;
                busWrite();
                break;
            case 0xa4: case 0xa5: // MOVS
                wait(5);
                if (_rep != 0)
                    wait(1);
                _repeating = false;
                if (_rep == 0 || cx() != 0) {
                    lodS();
                    wait(3);
                    stoS();
                    wait(3);
                    if (_rep != 0) {
                        --cx();
                        completed = cx() == 0;
                        _repeating = !completed;
                    }
                    checkInterrupts();
                }
                break;
            case 0xa6: case 0xa7: // CMPS
                throw Exception("Not yet implemented.");
                break;
            case 0xa8: case 0xa9: // TEST A, imm
                throw Exception("Not yet implemented.");
                break;
            case 0xaa: case 0xab: // STOS
                throw Exception("Not yet implemented.");
                break;
            case 0xac: case 0xad: // LODS
                throw Exception("Not yet implemented.");
                break;
            case 0xae: case 0xaf: // SCAS
                throw Exception("Not yet implemented.");
                break;
            case 0xb0: case 0xb1: case 0xb2: case 0xb3:
            case 0xb4: case 0xb5: case 0xb6: case 0xb7: // MOV rb, ib
                rb() = fetchInstructionByte();
                break;
            case 0xb8: case 0xb9: case 0xba: case 0xbb:
            case 0xbc: case 0xbd: case 0xbe: case 0xbf: // MOV rw, iw
                //rw() = fetchInstructionWord();
                throw Exception("Not yet implemented.");
                break;
            case 0xc0: case 0xc1: case 0xc2: case 0xc3: 
            case 0xc8: case 0xc9: case 0xca: case 0xcb: // RET
                throw Exception("Not yet implemented.");
                break;
            case 0xc4: case 0xc5: // LsS rw, rmd
                throw Exception("Not yet implemented.");
                break;
            case 0xc6: case 0xc7: // MOV rm, imm
                throw Exception("Not yet implemented.");
                break;
            case 0xcc: // INT 3
                throw Exception("Not yet implemented.");
                break;
            case 0xcd: // INT
                throw Exception("Not yet implemented.");
                break;
            case 0xce: // INTO
                throw Exception("Not yet implemented.");
                break;
            case 0xcf: // IRET
                throw Exception("Not yet implemented.");
                break;
            case 0xd0: case 0xd1: case 0xd2: case 0xd3: // rot rm
                throw Exception("Not yet implemented.");
                break;
            case 0xd4: // AAM
                throw Exception("Not yet implemented.");
                break;
            case 0xd5: // AAD
                throw Exception("Not yet implemented.");
                break;
            case 0xd6: // SALC
                throw Exception("Not yet implemented.");
                break;
            case 0xd7: // XLATB
                throw Exception("Not yet implemented.");
                break;
            case 0xd8: case 0xd9: case 0xda: case 0xdb:
            case 0xdc: case 0xdd: case 0xde: case 0xdf: // esc i, r, rm
                throw Exception("Not yet implemented.");
                break;
            case 0xe0: case 0xe1: case 0xe2: case 0xe3: // loop
                throw Exception("Not yet implemented.");
                break;
            case 0xe4: case 0xe5: case 0xe6: case 0xe7:
            case 0xec: case 0xed: case 0xee: case 0xef: // INOUT
                throw Exception("Not yet implemented.");
                break;
            case 0xe8: // CALL cw
                throw Exception("Not yet implemented.");
                break;
            case 0xe9: // JMP cw
                throw Exception("Not yet implemented.");
                break;
            case 0xea: // JMP cp
                throw Exception("Not yet implemented.");
                break;
            case 0xeb: // JMP cb
                wait(1);
                _data = fetchInstructionByte();
                jumpShort();
                _prefetching = true;
                break;
            case 0xf0: case 0xf1: // LOCK
                throw Exception("Not yet implemented.");
                break;
            case 0xf2: case 0xf3: // REP
                throw Exception("Not yet implemented.");
                break;
            case 0xf4: // HLT
                throw Exception("Not yet implemented.");
                break;
            case 0xf5: // CMC
                throw Exception("Not yet implemented.");
                break;
            case 0xf6: case 0xf7: // math
                readEA();
                if ((modRMReg() & 6) == 0) {
                    _destination = _data;
                    _data = fetchInstruction();
                }
                switch (modRMReg()) {
                    case 0:
                    case 1:  // TEST
                        //test(_destination, _data);
                        //end(_useMemory ? 7 : 5);
                        throw Exception("Not yet implemented.");
                        break;
                    case 2:  // NOT
                        //writeEA(~_data, _useMemory ? 8 : 3);
                        throw Exception("Not yet implemented.");
                        break;
                    case 3:  // NEG
                        //_source = _data;
                        //_destination = 0;
                        //sub();
                        //writeEA(_data, _useMemory ? 8 : 3);
                        throw Exception("Not yet implemented.");
                        break;
                    case 4:  // MUL
                    case 5:  // IMUL
                        _source = _data;
                        _destination = getAccum();
                        _data = _destination;
                        setSF();
                        setPF();
                        _data *= _source;
                        ax() = _data;
                        if (!_wordSize) {
                            if (modRMReg() == 4) {
                                setCF(ah() != 0);
                                wait(68);
                            }
                            else {
                                throw Exception("Not yet implemented.");
                                //if ((_source & 0x80) != 0)
                                //    ah() -= _destination;
                                //if ((_destination & 0x80) != 0)
                                //    ah() -= _source;
                                //setCF(ah() ==
                                //    ((al() & 0x80) == 0 ? 0 : 0xff));
                                //wait(80);
                            }
                        }
                        else {
                            throw Exception("Not yet implemented.");
                            //if (modRMReg() == 4) {
                            //    dx() = _data >> 16;
                            //    _data |= dx();
                            //    setCF(dx() != 0);
                            //    wait(118);
                            //}
                            //else {
                            //    dx() = _data >> 16;
                            //    if ((_source & 0x8000) != 0)
                            //        dx() -= _destination;
                            //    if ((_destination & 0x8000) != 0)
                            //        dx() -= _source;
                            //    _data |= dx();
                            //    setCF(dx() ==
                            //        ((ax() & 0x8000) == 0 ? 0 : 0xffff));
                            //    wait(128);
                            //}
                        }
                        setZF();
                        setOF(cf());
                        if (_useMemory)
                            wait(2);
                        break;
                    case 6:  // DIV
                    case 7:  // IDIV
                        throw Exception("Not yet implemented.");
                        //_source = _data;
                        //if (_source == 0) {
                        //    interrupt(0);
                        //    break;
                        //}
                        //if (!_wordSize) {
                        //    _destination = ax();
                        //    if (modRMReg() == 6) {
                        //        div();
                        //        if (_data > 0xff) {
                        //            interrupt(0);
                        //            break;
                        //        }
                        //        _wait = 80;
                        //    }
                        //    else {
                        //        _destination = ax();
                        //        if ((_destination & 0x8000) != 0)
                        //            _destination |= 0xffff0000;
                        //        _source = signExtend(_source);
                        //        div();
                        //        if (_data > 0x7f && _data < 0xffffff80) {
                        //            interrupt(0);
                        //            break;
                        //        }
                        //        _wait = 101;
                        //    }
                        //    ah() = _remainder;
                        //    al() = _data;
                        //}
                        //else {
                        //    _destination = (dx() << 16) + ax();
                        //    div();
                        //    if (modRMReg() == 6) {
                        //        if (_data > 0xffff) {
                        //            interrupt(0);
                        //            break;
                        //        }
                        //        _wait = 144;
                        //    }
                        //    else {
                        //        if (_data > 0x7fff && _data <  0xffff8000) {
                        //            interrupt(0);
                        //            break;
                        //        }
                        //        _wait = 165;
                        //    }
                        //    dx() = _remainder;
                        //    ax() = _data;
                        //}
                        //if (_useMemory)
                        //    _wait += 2;
                        //_state = stateEndInstruction;
                        break;
                }
                break;
            case 0xf8: case 0xf9: // CLCSTC
                throw Exception("Not yet implemented.");
                break;
            case 0xfa: case 0xfb: // CLISTI
                throw Exception("Not yet implemented.");
                break;
            case 0xfc: case 0xfd: // CLDSTD
                throw Exception("Not yet implemented.");
                break;
            case 0xfe: case 0xff: // misc
                throw Exception("Not yet implemented.");
                break;
        }
        if (completed) {
            _segmentOverride = -1;
            _rep = 0;
            checkInterrupts();
        }
    }
    void checkInterrupts()
    {
        //if (_nmiRequested) {
        //    _nmiRequested = false;
        //    interrupt(2);
        //}
        //if (intf() && _interruptRequested) {
        //    initIO(stateHardwareInt, ioInterruptAcknowledge,
        //        false);
        //    _interruptRequested = false;
        //    _wait = 1;
        //}
    }

    bool _logging;
    Test _test;
    String _log;
    int _logSkip;
    int _cycle;
    Array<Byte> _ram;
    int _stopIP;

    enum BusState
    {
        t1,
        t2,
        t3,
        tWait,
        t4,
        tFirstIdle,
        tIdle
    };

    void da()
    {
        _wordSize = false;
        _data = al();
        setPZS();
        wait(3);
    }
    void aa()
    {
        al() &= 0x0f;
        wait(8);
    }
    //void div()
    //{
    //    bool negative = false;
    //    bool dividendNegative = false;
    //    if (modRMReg() == 7) {
    //        if ((_destination & 0x80000000) != 0) {
    //            _destination =
    //                static_cast<UInt32>(-static_cast<SInt32>(_destination));
    //            negative = !negative;
    //            dividendNegative = true;
    //        }
    //        if ((_source & 0x8000) != 0) {
    //            _source = (static_cast<UInt32>(-static_cast<SInt32>(_source)))
    //                & 0xffff;
    //            negative = !negative;
    //        }
    //    }
    //    _data = _destination / _source;
    //    UInt32 product = _data * _source;
    //    // ISO C++ 2003 does not specify a rounding mode, but the x86 always
    //    // rounds towards zero.
    //    if (product > _destination) {
    //        --_data;
    //        product -= _source;
    //    }
    //    _remainder = _destination - product;
    //    if (negative)
    //        _data = static_cast<UInt32>(-static_cast<SInt32>(_data));
    //    if (dividendNegative)
    //        _remainder = static_cast<UInt32>(-static_cast<SInt32>(_remainder));
    //}
    void jumpShort()
    {
        wait(1);
        _prefetching = false;
        wait(3);
        while (_busState != tIdle || _ioNext._type != ioPassive)
            wait(1);
        wait(2);
        setIP(_ip + signExtend(_data));
    }
    //void interrupt(UInt8 number)
    //{
    //    _data = number;
    //    _state = stateIntAction;
    //}
    void test(UInt16 destination, UInt16 source)
    {
        _destination = destination;
        _source = source;
        bitwise(_destination & _source);
    }
    int stringIncrement()
    {
        int r = (_wordSize ? 2 : 1);
        return !df() ? r : -r;
    }
    void lodS()
    {
        _address = si();
        si() += stringIncrement();
        _segment = 3;
        busRead();
    }
    //void lodDIS(State state)
    //{
    //    _address = di();
    //    di() += stringIncrement();
    //    _segment = 0;
    //    initIO(state, ioRead, _wordSize);
    //}
    void stoS()
    {
        _address = di();
        di() += stringIncrement();
        _segment = 0;
        busWrite();
    }
    void push(UInt16 data)
    {
        wait(4);
        _prefetching = false;
        wait(2);
        _prefetching = true;
        push2(data);
    }
    void push2(UInt16 data)
    {
        _data = data;
        sp() -= 2;
        _address = sp();
        _segment = 2;
        busWriteWord(ioWriteMemory);
        wait(1);
    }
    Word pop()
    {
        wait(1);
        _prefetching = false;
        wait(2);
        _prefetching = true;
        return pop2();
    }
    Word pop2()
    {
        _address = sp();
        sp() += 2;
        _segment = 2;
        return busReadWord(ioReadMemory);
    }
    void setCA() { setCF(true); setAF(true); }
    void clearCA() { setCF(false); setAF(false); }
    void clearCAO() { clearCA(); setOF(false); }
    void setPZS() { setPF(); setZF(); setSF(); }
    void bitwise(UInt16 data) { _data = data; clearCAO(); setPZS(); }
    void doAF() { setAF(((_data ^ _source ^ _destination) & 0x10) != 0); }
    void doCF() { setCF((_data & (!_wordSize ? 0x100 : 0x10000)) != 0); }
    void setCAPZS() { setPZS(); doAF(); doCF(); }
    void setOFAdd()
    {
        UInt16 t = (_data ^ _source) & (_data ^ _destination);
        setOF((t & (!_wordSize ? 0x80 : 0x8000)) != 0);
    }
    void add() { _data = _destination + _source; setCAPZS(); setOFAdd(); }
    void setOFSub()
    {
        UInt16 t = (_destination ^ _source) & (_data ^ _destination);
        setOF((t & (!_wordSize ? 0x80 : 0x8000)) != 0);
    }
    void sub() { _data = _destination - _source; setCAPZS(); setOFSub(); }
    //void setOFRotate()
    //{
    //    setOF(((_data ^ _destination) & (!_wordSize ? 0x80 : 0x8000)) != 0);
    //}
    void doALUOperation()
    {
        switch (_aluOperation) {
            case 0: add(); break;
            case 1: bitwise(_destination | _source); break;
            case 2: _source += cf() ? 1 : 0; add(); break;
            case 3: _source += cf() ? 1 : 0; sub(); break;
            case 4: test(_destination, _source); break;
            case 5:
            case 7: sub(); break;
            case 6: bitwise(_destination ^ _source); break;
            default:
                throw Exception("Not yet implemented.");

        }
    }
    UInt16 signExtend(UInt8 data) { return data + (data < 0x80 ? 0 : 0xff00); }
    template<class U> class Register
    {
    public:
        void init(String name, U* data)
        {
            _name = name;
            _data = data;
            _read = false;
            _written = false;
        }
        const U& operator=(U value)
        {
            *_data = value;
            _writtenValue = value;
            _written = true;
            return *_data;
        }
        operator U()
        {
            if (!_read && !_written) {
                _read = true;
                _readValue = *_data;
            }
            return *_data;
        }
        const U& operator+=(U value)
        {
            return operator=(operator U() + value);
        }
        const U& operator-=(U value)
        {
            return operator=(operator U() - value);
        }
        const U& operator%=(U value)
        {
            return operator=(operator U() % value);
        }
        const U& operator&=(U value)
        {
            return operator=(operator U() & value);
        }
        const U& operator^=(U value)
        {
            return operator=(operator U() ^ value);
        }
        U operator-() const { return -operator U(); }
        void operator++() { operator=(operator U() + 1); }
        void operator--() { operator=(operator U() - 1); }
        String text()
        {
            String text;
            if (_read) {
                text = hex(_readValue, sizeof(U)==1 ? 2 : 4, false) + "<-" +
                    _name + "  ";
                _read = false;
            }
            if (_written) {
                text += _name + "<-" +
                    hex(_writtenValue, sizeof(U)==1 ? 2 : 4, false) + "  ";
                _written = false;
            }
            return text;
        }
    protected:
        String _name;
        U* _data;
        bool _read;
        bool _written;
        U _readValue;
        U _writtenValue;
    };
    class FlagsRegister : public Register<UInt16>
    {
    public:
        UInt32 operator=(UInt32 value)
        {
            Register<UInt16>::operator=(value);
            return Register<UInt16>::operator UInt16();
        }
        String text()
        {
            String text;
            if (this->_read) {
                text = flags(this->_readValue) + "<-" + this->_name + "  ";
                this->_read = false;
            }
            if (this->_written) {
                text += this->_name + "<-" + flags(this->_writtenValue) + "  ";
                this->_written = false;
            }
            return text;
        }
    private:
        String flags(UInt16 value) const
        {
            return String(value & 0x800 ? "O" : "p") +
                String(value & 0x400 ? "D" : "d") +
                String(value & 0x200 ? "I" : "i") +
                String(value & 0x100 ? "T" : "t") +
                String(value & 0x80 ? "S" : "s") +
                String(value & 0x40 ? "Z" : "z") +
                String(value & 0x10 ? "A" : "a") +
                String(value & 4 ? "P" : "p") +
                String(value & 1 ? "C" : "c");
        }
    };
    UInt32 physicalAddress(UInt16 segment, UInt16 offset)
    {
        return ((_segmentRegisterData[segment] << 4) + offset) & 0xfffff;
    }

    Register<UInt16>& rw() { return _wordRegisters[_opcode & 7]; }
    Register<UInt16>& ax() { return _wordRegisters[0]; }
    Register<UInt16>& cx() { return _wordRegisters[1]; }
    Register<UInt16>& dx() { return _wordRegisters[2]; }
    Register<UInt16>& bx() { return _wordRegisters[3]; }
    Register<UInt16>& sp() { return _wordRegisters[4]; }
    Register<UInt16>& bp() { return _wordRegisters[5]; }
    Register<UInt16>& si() { return _wordRegisters[6]; }
    Register<UInt16>& di() { return _wordRegisters[7]; }
    Register<UInt8>& rb() { return _byteRegisters[_opcode & 7]; }
    Register<UInt8>& al() { return _byteRegisters[0]; }
    Register<UInt8>& cl() { return _byteRegisters[1]; }
    Register<UInt8>& ah() { return _byteRegisters[4]; }
    Register<UInt16>& es() { return _segmentRegisters[0]; }
    Register<UInt16>& cs() { return _segmentRegisters[1]; }
    Register<UInt16>& ss() { return _segmentRegisters[2]; }
    Register<UInt16>& ds() { return _segmentRegisters[3]; }

    bool cf() { return (_flags & 1) != 0; }
    void setCF(bool cf) { _flags = (_flags & ~1) | (cf ? 1 : 0); }
    bool pf() { return (_flags & 4) != 0; }
    void setPF()
    {
        static UInt8 table[0x100] = {
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            0, 4, 4, 0, 4, 0, 0, 4, 4, 0, 0, 4, 0, 4, 4, 0,
            4, 0, 0, 4, 0, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 4};
        _flags = (_flags & ~4) | table[_data & 0xff];
    }
    bool af() { return (_flags & 0x10) != 0; }
    void setAF(bool af) { _flags = (_flags & ~0x10) | (af ? 0x10 : 0); }
    bool zf() { return (_flags & 0x40) != 0; }
    void setZF()
    {
        _flags = (_flags & ~0x40) |
            ((_data & (!_wordSize ? 0xff : 0xffff)) == 0 ? 0x40 : 0);
    }
    bool sf() { return (_flags & 0x80) != 0; }
    void setSF()
    {
        _flags = (_flags & ~0x80) |
            ((_data & (!_wordSize ? 0x80 : 0x8000)) != 0 ? 0x80 : 0);
    }
    //bool tf() { return (_flags & 0x100) != 0; }
    //void setTF(bool tf) { _flags = (_flags & ~0x100) | (tf ? 0x100 : 0); }
    bool intf() { return (_flags & 0x200) != 0; }
    //void setIF(bool intf) { _flags = (_flags & ~0x200) | (intf ? 0x200 : 0); }
    bool df() { return (_flags & 0x400) != 0; }
    //void setDF(bool df) { _flags = (_flags & ~0x400) | (df ? 0x400 : 0); }
    bool of() { return (_flags & 0x800) != 0; }
    void setOF(bool of) { _flags = (_flags & ~0x800) | (of ? 0x800 : 0); }
    int modRMReg() { return (_modRM >> 3) & 7; }
    Register<UInt16>& modRMRW() { return _wordRegisters[modRMReg()]; }
    Register<UInt8>& modRMRB() { return _byteRegisters[modRMReg()]; }
    UInt16 getReg()
    {
        if (!_wordSize)
            return static_cast<UInt8>(modRMRB());
        return modRMRW();
    }
    UInt16 getAccum()
    {
        if (!_wordSize)
            return static_cast<UInt8>(al());
        return ax();
    }
    void setAccum() { if (!_wordSize) al() = _data; else ax() = _data; }
    void setReg(UInt16 value)
    {
        if (!_wordSize)
            modRMRB() = static_cast<UInt8>(value);
        else
            modRMRW() = value;
    }
    void setIP(UInt16 value)
    {
        _ip = value;
        _prefetchedEU = 0;
        _prefetchedBIU = 0;
        _snifferDecoder.queueOperation(2);
        _prefetchAddress = _ip;
    }

    Register<UInt16> _wordRegisters[8];
    Register<UInt8> _byteRegisters[8];
    Register<UInt16> _segmentRegisters[8];
    UInt16 _registerData[8];      /* AX CX DX BX SP BP SI DI */
    UInt16 _segmentRegisterData[8];
    UInt16 _flagsData;
    //   0: CF = unsigned overflow?
    //   1:  1
    //   2: PF = parity: even number of 1 bits in result?
    //   3:  0
    //   4: AF = unsigned overflow for low nybble
    //   5:  0
    //   6: ZF = zero result?
    //   7: SF = result is negative?
    //   8: TF = interrupt after every instruction?
    //   9: IF = interrupts enabled?
    //  10: DF = SI/DI decrement in string operations
    //  11: OF = signed overflow?
    FlagsRegister _flags;
    UInt16 _ip;

    UInt8 _opcode;
    UInt8 _modRM;
    UInt32 _data;
    UInt32 _source;
    UInt32 _destination;
    UInt32 _remainder;
    UInt16 _address;
    bool _useMemory;
    bool _wordSize;
    int _aluOperation;
    bool _sourceIsRM;
    UInt16 _savedCS;
    UInt16 _savedIP;
    int _rep;
    bool _repeating;
    int _segment;

    UInt8 _prefetchQueue[4];
    UInt8 _prefetchOffset;
    UInt8 _prefetchedEU;
    UInt8 _prefetchedBIU;
    bool _prefetchedRemove;
    UInt16 _prefetchAddress;
    BusState _busState;
    bool _statusSet;
    bool _prefetching;

    struct IOInformation
    {
        IOType _type;
        UInt32 _address;
        UInt8 _data;
        int _segment;
    };

    IOInformation _ioInProgress;
    IOInformation _ioNext;
    bool _synchronousDone;
    IOType _lastIO;

    int _segmentOverride;

    bool _ready;
    int _queueCycle;

    SnifferDecoder _snifferDecoder;
};

class Program : public ProgramBase
{
public:
    void run()
    {
        Array<Byte> testProgram;
        File("runtests.bin").readIntoArray(&testProgram);

        for (int i = 0; i < 0x100; ++i) {
            Instruction instruction(i);
            if (instruction.hasModrm()) {
                static const Byte modrms[] = {
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0xc0};
                int jj = 1;
                if (instruction.isGroup())
                    jj = 8;
                for (int m = 0; m < 25; ++m) {
                    for (int j = 0; j < jj; ++j)
                        addTest(Instruction(i, modrms[m] + (j << 3)));
                }
            }
            else
                addTest(instruction);
        }

        _cache.allocate(_tests.count());
        _cacheHighWaterMark = 0;
        {
            File f(String("cache.dat"));
            auto s = f.tryOpenRead();
            if (s.valid()) {
                UInt64 size = s.size();
                if (size > _tests.count()*2)
                    throw Exception("Cache file too large!");
                s.read(reinterpret_cast<Byte*>(&_cache[0]), (int)size);
                _cacheHighWaterMark = static_cast<int>(size)/2;
            }
        }

        Emulator emulator;

        int nextTest = 0;
        int availableLength = 0xff00 - testProgram.count();
        do {
            int totalLength = 0;
            int newNextTest = _tests.count();
            for (int i = nextTest; i < newNextTest; ++i) {
                int nl = totalLength + _tests[i].length();
                int cycles = emulator.expected(_tests[i]);
                _tests[i].setCycles(cycles);
                bool useTest = true;
                if (i < _cacheHighWaterMark)
                    useTest = cycles != _cache[i];
                if (useTest) {
                    if (nl > availableLength) {
                        newNextTest = i;
                        break;
                    }
                    totalLength = nl;
                    if (i < _cacheHighWaterMark) {
                        // This test will fail. Just run the one to get a
                        // sniffer log.
                        nextTest = i;
                        newNextTest = i + 1;
                        break;
                    }
                }
                else
                    nextTest = i + 1;
            }
            Array<Byte> output(totalLength + 2);
            Byte* p = &output[0];
            *p = totalLength;
            p[1] = totalLength >> 8;
            p += 2;
            for (int i = nextTest; i < newNextTest; ++i) {
                int cycles = (_tests[i].cycles() + 1) * 2;
                p[0] = cycles;
                p[1] = cycles >> 8;
                p += 2;
                _tests[i].output(p);
                p += _tests[i].length() - 2;
                if (i % 10 == 0)
                    printf(".");
            }

            {
                auto h = File("runtest.bin").openWrite();
                h.write(testProgram);
                h.write(output);
            }
            NullTerminatedWideString data(String("cmd /c run.bat"));

            {
                PROCESS_INFORMATION pi;
                ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

                STARTUPINFO si;
                ZeroMemory(&si, sizeof(STARTUPINFO));
                si.cb = sizeof(STARTUPINFO);

                IF_FALSE_THROW(CreateProcess(NULL, data, NULL, NULL, FALSE,
                    0, NULL, NULL, &si, &pi) != 0);
                CloseHandle(pi.hThread);
                WindowsHandle hProcess = pi.hProcess;
                IF_FALSE_THROW(WaitForSingleObject(hProcess, 3*60*1000) ==
                    WAIT_OBJECT_0);
            }

            String result = File("runtests.output").contents();
            CharacterSource s(result);
            do {
                if (parse(&s, "FAIL")) {
                    Rational result;
                    Space::parse(&s);
                    if (!Space::parseNumber(&s, &result))
                        throw Exception("Cannot parse number of failing test");
                    int n = result.floor() + nextTest;

                    console.write(decimal(n) + "\n");
                    _tests[n].write();
                    dumpCache(n);

                    String expected = emulator.log(_tests[n]);
                    String expected1;

                    String observed;
                    CharacterSource e(expected);
                    int skipLines = 4;
                    do {
                        int oc = s.get();
                        if (oc == -1)
                            break;
                        if (oc == '\n')
                            --skipLines;
                    } while (skipLines > 0);

                    int c = _tests[n].cycles() - 5;
                    int line = 0;
                    int column = 1;

                    do {
                        int ec = e.get();
                        if (ec == -1)
                            break;
                        ++column;
                        if ((column >= 7 && column < 20) || column >= 23) {
                            //if (line < c)
                                expected1 += codePoint(ec);
                        }
                        if (ec == '\n') {
                            ++line;
                            column = 1;
                        }
                    } while (true);
                    line = 0;
                    column = 1;
                    do {
                        int oc = s.get();
                        if (oc == -1)
                            break;
                        ++column;
                        if ((column >= 7 && column < 20) || column >= 23) {
                            //if (line < c)
                                observed += codePoint(oc);
                        }
                        if (oc == '\n') {
                            ++line;
                            if (column == 1)
                                break;
                            column = 1;
                        }
                    } while (true);

                    File("expected.txt").openWrite().write(expected1);
                    File("observed.txt").openWrite().write(observed);

                    PROCESS_INFORMATION pi;
                    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

                    STARTUPINFO si;
                    ZeroMemory(&si, sizeof(STARTUPINFO));
                    si.cb = sizeof(STARTUPINFO);

                    NullTerminatedWideString data(
                        String("q observed.txt expected.txt"));

                    IF_FALSE_THROW(CreateProcess(NULL, data, NULL, NULL, FALSE,
                        0, NULL, NULL, &si, &pi) != 0);

                    exit(1);
                }
                if (parse(&s, "PASS"))
                    break;
                int c = s.get();
                if (c == -1)
                    throw Exception("Test was inconclusive");
            } while (true);

            nextTest = newNextTest;
            if (nextTest == _tests.count())
                break;

        } while (true);
        dumpCache(_tests.count());
    }
private:
    void addTest(Instruction i)
    {
        for (int nops = 0; nops < 20; ++nops) {
            if (i.opcode() == 0x9b)  // No way to test WAIT at the moment...
                continue;
            Test t(0, nops);
            t.addInstruction(i);
            _tests.append(t);
        }
    }
    bool parse(CharacterSource* s, String m)
    {
        CharacterSource ss = *s;
        CharacterSource ms(m);
        do {
            int mc = ms.get();
            if (mc == -1) {
                *s = ss;
                return true;
            }
            int sc = ss.get();
            if (sc != mc)
                return false;
        } while (true);
    }
    void dumpCache(int n)
    {
        if (_cacheHighWaterMark > n)
            return;
        for (int i = _cacheHighWaterMark; i < n; ++i)
            _cache[i] = _tests[i].cycles();
        _cacheHighWaterMark = n;
        File(String("cache.dat")).
            save(reinterpret_cast<const Byte*>(&_cache[0]), n*2);
    }

    AppendableArray<Test> _tests;

    Array<Word> _cache;
    int _cacheHighWaterMark = 0;
};