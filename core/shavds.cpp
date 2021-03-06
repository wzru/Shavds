#include "llvm-c/Types.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/EpochTracker.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/DWARFLinker/DWARFLinkerCompileUnit.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/AlignOf.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/ReverseIteration.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/type_traits.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <new>
#include <queue>
#include <type_traits>
#include <typeinfo>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace llvm;

using u32  = uint32_t;
using VOPI = std::vector<llvm::Function::op_iterator>;

const std::string RED    = "\e[1;41m";   //红
const std::string GREEN  = "\e[1;42m";   //绿
const std::string YELLOW = "\e[43;37m";  //黄
const std::string BLUE   = "\e[1;44m";   //蓝
const std::string PINK   = "\e[1;45m";   //粉红
const std::string RESET  = "\e[0m";      //清除颜色

const std::string SHAVDS = "__SHAVDS_by_WZR__";

inline std::string systemOut(const std::string cmd)
{
    char buf[1024] = {0};
    int  fd[2];
    int  backfd;
    pipe(fd);
    backfd = dup(STDOUT_FILENO);  //备份标准输出，用于恢复
    dup2(fd[1], STDOUT_FILENO);   //将标准输出重定向到fd[1]
    system(cmd.c_str());
    read(fd[0], buf, 1024);
    dup2(backfd, STDOUT_FILENO);  //恢复标准输出
    if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
    return std::string(buf);
}

inline bool cmpOp(const llvm::Function::op_iterator& a, const llvm::Function::op_iterator& b)
{
    return a->get()->getType() == b->get()->getType() && a->get()->getValueID() == b->get()->getValueID() &&
           a->get()->getNumUses() == b->get()->getNumUses() && a->get()->getValueName() == b->get()->getValueName() &&
           a->get()->getName() == b->get()->getName() &&
           a->get()->getRawSubclassOptionalData() == b->get()->getRawSubclassOptionalData();
}

inline u32 LCS(const VOPI& u, const VOPI& v)
{
    u32 n = u.size(), m = v.size();
    // std::cerr << "LCS n=" << n << " m=" << m;
    u32** f = new u32*[n + 1];
    for (int i = 0; i <= n; ++i) { f[i] = new u32[m + 1](); }
    for (auto i = 0; i < n; ++i) {
        for (auto j = 0; j < m; ++j) {
            if (cmpOp(u[i], v[j]))
                f[i + 1][j + 1] = f[i][j] + 1;
            else
                f[i + 1][j + 1] = std::max(f[i][j + 1], f[i + 1][j]);
        }
    }
    u32 ans = f[n][m];
    for (int i = 0; i <= n; ++i) { delete[] f[i]; }
    delete[] f;
    // std::cerr << " ans=" << ans << "\n";
    return ans;
}

inline bool startWith(const std::string& s1, const std::string& s2)
{
    return s1.compare(0, s2.size(), s2) == 0;
}

inline bool isIgnored(const std::string& name)
{
    return startWith(name, "_GLOBAL")
           // || name == "main"
           || (name.find("::") != std::string::npos);
}

struct CmpRes
{
    std::string func1, func2;
    u32         cnti1, cnti2;
    u32         line1, line2;
    std::string file1, file2;
    double      sim;
    CmpRes() = default;
    CmpRes(const std::string& func1,
           const std::string& func2,
           u32                c1,
           u32                c2,
           u32                l1,
           u32                l2,
           const std::string& file1,
           const std::string& file2,
           double             s)
        : func1(func1), func2(func2), cnti1(c1), cnti2(c2), line1(l1), line2(l2), file1(file1), file2(file2), sim(s)
    {
    }
};

struct DetRes
{
    u32         line, col;
    std::string type;
    DetRes() = default;
    DetRes(u32 l, u32 c, const std::string& s) : line(l), col(c), type(s) {}
};

namespace {
struct Hello : public FunctionPass
{
    static char ID;
    Hello() : FunctionPass(ID) {}
    bool runOnFunction(Function& F) override
    {
        errs() << "Hello: ";
        errs().write_escaped(F.getName()) << '\n';
        return false;
    }
};

struct Obfuscator : public FunctionPass
{
    static char ID;
    Obfuscator() : FunctionPass(ID) {}
    bool runOnFunction(Function& F) override
    {
        // errs() << "Obfuscator: ";
        std::string fileName = (F.getParent())->getSourceFileName();
        F.setName(systemOut("basename " + fileName) + SHAVDS + F.getName());
        // errs().write_escaped(systemOut("basename " + fileName) ) << '\n';
        return false;
    }
};

//比较若干文件中的所有函数IR相似性
struct FunComparator : public ModulePass
{
    struct FuncInfo
    {
        std::string            name;
        llvm::Module::iterator mi;
        u32                    line;
        FuncInfo() = default;
        FuncInfo(const std::string& name, const llvm::Module::iterator& mi, u32 line) : name(name), mi(mi), line(line)
        {
        }
    };
    static char                                  ID;
    std::map<std::string, std::vector<FuncInfo>> mfi;
    std::map<std::string, u32>                   mfic;  //每个文件的inst数量
    FunComparator() : ModulePass(ID) {}
    u32 Count(const llvm::Module::iterator& a)
    {
        u32 res = 0;
        for (auto i = a->begin(); i != a->end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                for (auto k = j->op_begin(); k != j->op_end(); ++k) ++res;
        return res;
    }
    double RepRate(llvm::Module::iterator& a, llvm::Module::iterator& b, u32& cnt)
    {
        VOPI u, v;
        for (auto i = a->begin(); i != a->end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                for (auto k = j->op_begin(); k != j->op_end(); ++k) u.push_back(k);
        for (auto i = b->begin(); i != b->end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                for (auto k = j->op_begin(); k != j->op_end(); ++k) v.push_back(k);
        cnt += u.size() * v.size();
        return (double)LCS(u, v) / std::min(u.size(), v.size());
    }
    bool runOnModule(Module& M) override
    {
        for (auto i = M.functions().begin(); i != M.functions().end(); ++i) {  //遍历模块中的函数
            std::string fullName = i->getName();
            size_t      pos      = fullName.find(SHAVDS);
            if (pos != std::string::npos) {  //如果是目标函数
                std::string fileName = fullName.substr(0, pos);
                std::string funcName = fullName.substr(pos + SHAVDS.size(), fullName.size() - pos - SHAVDS.size());
                std::string origName = systemOut(std::string("c++filt ") + funcName);
                // std::cerr << "funcName=" << funcName << "\n";
                if (isIgnored(origName)) continue;
                mfi[fileName].push_back(FuncInfo(origName, i, i->getSubprogram()->getLine()));
                // std::cerr << "name=" << mfi[fileName][0].name << "\n";
            }
        }
        for (auto& i : mfi) {
            for (auto j : i.second) { mfic[i.first] += Count(j.mi); }
        }
        u32 sum = 0, sum2 = 0;
        for (auto& i : mfic) {
            // printf("i.sec=%d\n", i.second);
            sum += i.second;
            sum2 += i.second * i.second;
        }
        u32                 s = (sum * sum - sum2) / 2, cnt = 0;
        std::vector<CmpRes> vcr;
        for (auto i = mfi.begin(); i != mfi.end(); ++i) {
            auto tmp = i;
            for (auto j = ++tmp; j != mfi.end(); ++j) {
                for (auto& i1 : i->second)
                    for (auto& j1 : j->second) {
                        double res = RepRate(i1.mi, j1.mi, cnt);
                        fprintf(stderr, "progress %.8lf\r", (double)cnt / s);
                        vcr.push_back(CmpRes(i1.name, j1.name, i1.mi->getInstructionCount(),
                                             j1.mi->getInstructionCount(), i1.line, j1.line, i->first, j->first, res));
                    }
            }
        }
        fprintf(stderr, "\n");
        for (int i = 0; i < vcr.size(); ++i) {
            fprintf(stderr, "'%s' '%s' %u %u %u %u '%s' '%s' %.8lf\n", vcr[i].func1.c_str(), vcr[i].func2.c_str(),
                    vcr[i].cnti1, vcr[i].cnti2, vcr[i].line1, vcr[i].line2, vcr[i].file1.c_str(), vcr[i].file2.c_str(),
                    vcr[i].sim);
        }
        return false;
    }
};

struct CfgComparator : public ModulePass
{
    struct FuncCfg
    {
        std::string              name;
        llvm::Module::iterator   mi;
        u32                      line;
        std::vector<BasicBlock*> bb;
        FuncCfg() = default;
        FuncCfg(const std::string& name, llvm::Module::iterator mi, u32 line) : name(name), mi(mi), line(line) {}
    };
    static char                                 ID;
    std::map<std::string, std::vector<FuncCfg>> mfi;
    typedef decltype(mfi["123"].rbegin())       RIT;
    enum Color { WHITE, GREY, BLACK };
    // Color marks per vertex (BB).
    typedef DenseMap<BasicBlock*, Color> BBColorMap;
    // Collects vertices (BBs) in "finish" order. The first finished vertex is
    // first, and so on.
    typedef SmallVector<BasicBlock*, 32> BBVector;
    BBColorMap                           ColorMap;
    BBVector                             SortedBBs;

    CfgComparator() : ModulePass(ID) {}
    void Toposort(Function& F, RIT it)
    {
        for (auto I = F.begin(), IE = F.end(); I != IE; ++I) { ColorMap[&*I] = WHITE; }
        bool success = DoToposort(&F.getEntryBlock(), it);
    }
    // Helper function to recursively run topological sort from a given BB.
    // Returns true if the sort succeeded and false otherwise; topological sort
    // may fail if, for example, the graph is not a DAG (detected a cycle).
    bool DoToposort(BasicBlock* BB, RIT it)
    {
        // std::cerr << "Here's " << BB << "\n";
        ColorMap[BB] = GREY;
        // For demonstration, using the lowest-level APIs here. A BB's
        // successors are determined by looking at its terminator instruction.
        auto* TInst = BB->getTerminator();
        // errs() << BB->getParent()->getName();
        // errs() << " num=" << TInst->getNumSuccessors() << '\n';
        for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I) {
            BasicBlock* Succ      = TInst->getSuccessor(I);
            Color       SuccColor = ColorMap[Succ];
            if (SuccColor == WHITE) {
                // if (!DoToposort(Succ, it)) return false;
                DoToposort(Succ, it);
            }
            else if (SuccColor == GREY) {
                // This detects a cycle because grey vertices are all ancestors
                // of the currently explored vertex (in other words, they're "on
                // the stack").
                // errs() << "  Detected cycle: edge from " << BB->getName() << " to " << Succ->getName() << "\n";
                // return false;
            }
        }
        // This BB is finished (fully explored), so we can add it to the vector.
        ColorMap[BB] = BLACK;
        SortedBBs.push_back(BB);
        it->bb.push_back(BB);
        return true;
    }

    bool runOnModule(Module& M)
    {
        // fprintf(stderr, "Module=%s", M.getName());
        for (auto i = M.functions().begin(); i != M.functions().end(); ++i) {  //遍历模块中的函数
            std::string fullName = i->getName();
            size_t      pos      = fullName.find(SHAVDS);
            if (pos != std::string::npos) {  //如果是目标函数
                std::string fileName = fullName.substr(0, pos);
                std::string funcName = fullName.substr(pos + SHAVDS.size(), fullName.size() - pos - SHAVDS.size());
                std::string origName = systemOut(std::string("c++filt ") + funcName);
                if (isIgnored(origName)) continue;
                mfi[fileName].push_back(FuncCfg(origName, i, i->getSubprogram()->getLine()));
                Toposort(*i, mfi[fileName].rbegin());
            }
        }
        // for (auto i : mfi) {
        //     for (auto j : i.second) { fprintf(stderr, "%s %s\n", i.first.c_str(), j.name.c_str()); }
        // }
        for (auto i = mfi.begin(); i != mfi.end(); ++i) {
            auto tmp = i;
            for (auto j = ++tmp; j != mfi.end(); ++j) {  // i, j枚举module
                u32                 sum1 = 0, sum2 = 0, cnt1 = 0, cnt2 = 0;
                u32                 siz1 = i->second.size(), siz2 = j->second.size();
                std::vector<CmpRes> vcr;
                // 统计op数
                for (auto i1 : i->second)
                    for (auto i2 = i1.bb.begin(); i2 != i1.bb.end(); ++i2)
                        for (auto i3 = (*i2)->begin(); i3 != (*i2)->end(); ++i3) sum1 += i3->getNumOperands();
                for (auto j1 : j->second)
                    for (auto j2 = j1.bb.begin(); j2 != j1.bb.end(); ++j2)
                        for (auto j3 = (*j2)->begin(); j3 != (*j2)->end(); ++j3) sum2 += j3->getNumOperands();
                for (auto i1 : i->second)        // i1枚举所有函数
                    for (auto j1 : j->second) {  // i2枚举所有函数
                        // fprintf(stderr, "n:%s m:%s\n", i1.name.c_str(), j1.name.c_str());
                        // fprintf(stderr, "n:%s m:%s\n", i->first.c_str(), j->first.c_str());
                        VOPI u, v;
                        // 做LCS
                        for (auto i2 = i1.bb.begin(); i2 != i1.bb.end(); ++i2)
                            for (auto i3 = (*i2)->begin(); i3 != (*i2)->end(); ++i3) {
                                cnt1 += i3->getNumOperands();
                                for (auto i4 = i3->op_begin(); i4 != i3->op_end(); ++i4) { u.push_back(i4); }
                            }
                        for (auto j2 = j1.bb.begin(); j2 != j1.bb.end(); ++j2)
                            for (auto j3 = (*j2)->begin(); j3 != (*j2)->end(); ++j3) {
                                cnt2 += j3->getNumOperands();
                                for (auto j4 = j3->op_begin(); j4 != j3->op_end(); ++j4) { v.push_back(j4); }
                            }
                        double res = (double)LCS(u, v) / std::min(u.size(), v.size());
                        // std::cerr << GREEN << i1.name << RESET << " (" << i1.mi->getInstructionCount()
                        //           << " instructions) in " << BLUE << i->first << ":" << i1.line << RESET << "\n"
                        //           << GREEN << j1.name << RESET << " (" << j1.mi->getInstructionCount()
                        //           << " instructions) in " << BLUE << j->first << ":" << j1.line << RESET << "\n\tare
                        //           "
                        //           << RED << res * 100 << "% similar!" << RESET << "\n";
                        // std::cerr.width(5);
                        // fprintf(stderr, "%6.2lf\%\r", (double)(cnt1) / (sum1 * siz2) * 100);
                        fprintf(stderr, "progress %.8lf\r", (double)(cnt1) / (sum1 * siz2));
                        // std::cerr << (double)(cnt1) / (sum1 * siz2) * 100 << "%\n";
                        vcr.push_back(CmpRes(i1.name, j1.name, i1.mi->getInstructionCount(),
                                             j1.mi->getInstructionCount(), i1.line, j1.line, i->first, j->first, res));
                    }
                fprintf(stderr, "\n");
                for (int i = 0; i < vcr.size(); ++i) {
                    fprintf(stderr, "'%s' '%s' %u %u %u %u '%s' '%s' %.8lf\n", vcr[i].func1.c_str(),
                            vcr[i].func2.c_str(), vcr[i].cnti1, vcr[i].cnti2, vcr[i].line1, vcr[i].line2,
                            vcr[i].file1.c_str(), vcr[i].file2.c_str(), vcr[i].sim);
                }
            }
        }
        return false;
    }
};

struct Detector : public ModulePass
{
    static char                    ID;
    LLVMContext*                   C;
    Value*                         logFunc;
    Type*                          VoidTy;
    Type*                          Int32Ty;
    const std::vector<std::string> BufVulFuncNames{"strcpy", "strcpy",  "strncpy", "memcpy", "strcat", "strncat",
                                                   "gets",   "getchar", "fgetc",   "getc",   "read"};
    const std::vector<std::string> FmtVulFuncNames{
        "sprintf", "vsprintf",        "sscanf",         "fscanf",           "vfscanf",        "vscanf",
        "vsscanf", "__isoc99_sscanf", "__isoc99_scanf", "__isoc99_sprintf", "__isoc99_printf"};
    Detector() : ModulePass(ID) {}
    void init(Module& M)
    {
        C       = &(M.getContext());
        VoidTy  = Type::getVoidTy(*C);
        Int32Ty = Type::getInt32Ty(*C);
        // logFunc = M.getOrInsertFunction("logop", VoidTy, Int32Ty, NULL);
    }
    bool isBufVul(const std::string& name)  //缓冲区溢出漏洞
    {
        for (auto i : BufVulFuncNames) {
            if (i == name) return true;
        }
        return false;
    }
    bool isIntWidthVul(const std::string& name)  //整数宽度溢出漏洞
    {
        return name.find("__ubsan_handle_implicit_conversion") != std::string::npos;
    }
    bool isIntOpVul(const std::string& name)  //整数运算溢出漏洞
    {
        return name.find("with.overflow") != std::string::npos;
    }
    bool isIntSgnVul(const std::string& name)  //整数符号溢出漏洞
    {
        return name.find("__ubsan_handle_implicit_conversion") != std::string::npos;
    }
    bool isPtrNullVul(const std::string& name)  //空指针漏洞
    {
        return name.find("__ubsan_handle_type_mismatch") != std::string::npos;
    }
    bool isFmtStrVul(const std::string& name)  //空指针漏洞
    {
        for (auto i : FmtVulFuncNames) {
            if (i == name) return true;
        }
        return false;
    }
    void findOperand(Value* itVal)
    {
        std::stack<Value*> st;
        st.push(itVal);
        while (!st.empty()) {
            auto ele = st.top();
            st.pop();
            if (isa<Instruction>(ele)) {
                Instruction* tip = (Instruction*)ele;
                if (isa<AllocaInst>(tip)) {
                    errs() << "others\n";
                    // opdSet.insert(ele);
                }
                else if (isa<LoadInst>(tip)) {
                    Value* ti = tip->getOperand(0);
                    if (!isa<ConstantData>(ti)) st.push(ti);
                }
                else if (isa<CallInst>(tip)) {
                    Function* calledFp = cast<CallInst>(tip)->getCalledFunction();
                    errs() << calledFp->getName() << "\n";
                    if (calledFp->getName() == "malloc" || calledFp->getName() == "_Znwm") {
                        errs() << "Dynamic memory allocation!\n";
                        errs() << tip->getNumOperands() << "\n";
                        errs() << tip->getOperand(0) << "\n";
                    }
                    else {
                        // fetch the last bb of the function
                        auto bb = calledFp->end();
                        if (bb != calledFp->begin()) {
                            bb--;
                            BasicBlock* bp = &(*bb);
                            // fetch the terminator
                            Instruction* term = bp->getTerminator();
                            if (isa<ReturnInst>(term)) {
                                // find Operand
                                findOperand(term->getOperand(0));
                                errs() << "done\n";
                            }
                        }
                    }
                }
                else {
                    for (int i = 0, numOp = tip->getNumOperands(); i < numOp; i++) {
                        Value* ti = tip->getOperand(i);
                        if (!isa<ConstantData>(ti)) { st.push(ti); }
                    }
                }
            }
            else if (isa<GlobalVariable>(ele)) {
                errs() << "others\n";
            }
        }

    }  // findOperand
    void visitStoreInst(StoreInst& ip)
    {
        Value* lhs = ip.getOperand(1);
        Value* rhs = ip.getOperand(0);
        if (lhs->getType()->getContainedType(0)->isPointerTy()) {
            // figure out rhs
            errs() << "pointer assignment!" << lhs->getName() << "\n";
            findOperand(rhs);
        }
    }
    Value* getLineNum(Instruction* I)
    {
        const DebugLoc* debugLoc = &I->getDebugLoc();
        if (debugLoc) return ConstantInt::get(Int32Ty, debugLoc->getLine());
        return ConstantInt::get(Int32Ty, -1);
    }
    bool runOnModule(Module& M)
    {
        bool                res = false;
        std::vector<DetRes> vdr;
        // init(M);
        for (Function& F : M) {
            for (BasicBlock& B : F) {
                for (Instruction& I : B) {
                    // if (CallInst* call_inst = dyn_cast<CallInst>(&I)) {
                    //     Function* fn      = call_inst->getCalledFunction();
                    //     StringRef fn_name = fn->getName();
                    //     errs() << fn_name << ":"
                    //            << "\n";
                    //     for (auto args = fn->arg_begin(); args != fn->arg_end(); ++args) {
                    //         ConstantInt* arg = dyn_cast<ConstantInt>(&(*args));
                    //         if (arg != NULL) errs() << arg->getValue() << "\n";
                    //     }
                    // }
                    for (auto op = I.op_begin(); op != I.op_end(); ++op) {
                        // errs() << op->get()->getName() << "\n";
                        if (isIntWidthVul(op->get()->getName())) {
                            vdr.push_back(
                                DetRes(I.getDebugLoc().getLine(), I.getDebugLoc().getCol(), "integer-width-overflow"));
                        }
                        if (isIntOpVul(op->get()->getName())) {
                            vdr.push_back(DetRes(I.getDebugLoc().getLine(), I.getDebugLoc().getCol(),
                                                 "integer-operate-overflow"));
                        }
                        if (isIntSgnVul(op->get()->getName())) {
                            vdr.push_back(
                                DetRes(I.getDebugLoc().getLine(), I.getDebugLoc().getCol(), "integer-sign-overflow"));
                        }
                        if (isPtrNullVul(op->get()->getName())) {
                            vdr.push_back(DetRes(I.getDebugLoc().getLine(), I.getDebugLoc().getCol(), "pointer-null"));
                        }
                        if (isFmtStrVul(op->get()->getName())) {
                            vdr.push_back(DetRes(I.getDebugLoc().getLine(), I.getDebugLoc().getCol(), "format-string"));
                        }
                        if (isBufVul(op->get()->getName())) {
                            vdr.push_back(
                                DetRes(I.getDebugLoc().getLine(), I.getDebugLoc().getCol(), "buffer-overflow"));
                        }
                        // if (isBufVul(op->get()->getName())) {
                        //     errs() << "buffer-overflow        " << I.getDebugLoc().getLine() << " "
                        //            << I.getDebugLoc().getCol() << "\n";
                        // }
                        // if (isIntWidthVul(op->get()->getName())) {
                        //     errs() << "integer-width-overflow " << I.getDebugLoc().getLine() << " "
                        //            << I.getDebugLoc().getCol() << "\n";
                        // }
                        // if (isIntOpVul(op->get()->getName())) {
                        //     errs() << "integer-operate-overflow " << I.getDebugLoc().getLine() << " "
                        //            << I.getDebugLoc().getCol() << "\n";
                        // }
                        // if (isIntSgnVul(op->get()->getName())) {
                        //     errs() << "integer-sign-overflow " << I.getDebugLoc().getLine() << " "
                        //            << I.getDebugLoc().getCol() << "\n";
                        // }
                        // if (isPtrNullVul(op->get()->getName())) {
                        //     errs() << "pointer-null          " << I.getDebugLoc().getLine() << " "
                        //            << I.getDebugLoc().getCol() << "\n";
                        // }
                        // if (isFmtStrVul(op->get()->getName())) {
                        //     errs() << "format-string " << I.getDebugLoc().getLine() << " " <<
                        //     I.getDebugLoc().getCol()
                        //            << "\n";
                        // }
                    }
                }
            }
        }
        char buf[65536];
        memset(buf, 0, sizeof(buf));
        for (int i = 0; i < vdr.size(); ++i) {
            sprintf(buf, "%s %d %d\n", vdr[i].type.c_str(), vdr[i].line, vdr[i].col);
        }
        fputs(buf, stderr);
        return 0;
    }
};

}  // namespace

char Hello::ID         = 0;
char Obfuscator::ID    = 1;
char FunComparator::ID = 2;
char CfgComparator::ID = 3;
char Detector::ID      = 4;

// Register for opt
static RegisterPass<Hello>         hello("hello", "Hello World Pass");
static RegisterPass<Obfuscator>    obfuscate("obfuscate", "Prefix Obfuscate Pass");
static RegisterPass<FunComparator> cmpfun("cmpfun", "Compare Merged IR Functions Pass");
static RegisterPass<CfgComparator> cmpcfg("cmpcfg", "Compare Merged IR CFG Pass");
static RegisterPass<Detector>      detect("detect", "Detect Vulnerability Pass");

// Register for clang
static RegisterStandardPasses Y(PassManagerBuilder::EP_EarlyAsPossible,
                                [](const PassManagerBuilder& Builder, legacy::PassManagerBase& PM) {
                                    PM.add(new Hello());
                                    PM.add(new Obfuscator());
                                    PM.add(new FunComparator());
                                    PM.add(new CfgComparator());
                                    PM.add(new Detector());
                                });