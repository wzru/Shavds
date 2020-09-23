#include "llvm-c/Types.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/EpochTracker.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/DWARFLinker/DWARFLinkerCompileUnit.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
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
    FunComparator() : ModulePass(ID) {}
    double RepRate(llvm::Module::iterator& a, llvm::Module::iterator& b)
    {
        VOPI u, v;
        for (auto i = a->begin(); i != a->end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                for (auto k = j->op_begin(); k != j->op_end(); ++k) u.push_back(k);
        for (auto i = b->begin(); i != b->end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                for (auto k = j->op_begin(); k != j->op_end(); ++k) v.push_back(k);
        return (double)LCS(u, v) / std::min(u.size(), v.size());
    }
    // double RepRate(llvm::Module::iterator& a, llvm::Module::iterator& b)
    // {
    //     return (double)FuncLcs(a, b) / std::min(a->getInstructionCount(), b->getInstructionCount());
    // }
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
        // std::cerr << "ready to compare...\n";
        // std::map<std::string, std::vector<FuncInfo>> mfi;
        for (auto i = mfi.begin(); i != mfi.end(); ++i) {
            auto tmp = i;
            for (auto j = ++tmp; j != mfi.end(); ++j) {
                // std::cerr << "Now compare " << i->first << " and " << j->first << "\n";
                for (auto i1 : i->second)
                    for (auto j1 : j->second) {
                        double res = RepRate(i1.mi, j1.mi);
                        std::cerr << GREEN << i1.name << RESET << " (" << i1.mi->getInstructionCount()
                                  << " instructions) in " << BLUE << i->first << ":" << i1.line << RESET << "\n"
                                  << GREEN << j1.name << RESET << " (" << j1.mi->getInstructionCount()
                                  << " instructions) in " << BLUE << j->first << ":" << j1.line << RESET << "\n\tare "
                                  << RED << res * 100 << "% similar!" << RESET << "\n";
                    }
            }
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
        // errs() << "Topological sort of " << F.getName() << ":\n";
        // Initialize the color map by marking all the vertices white.
        for (auto I = F.begin(), IE = F.end(); I != IE; ++I) { ColorMap[&*I] = WHITE; }
        // The BB graph has a single entry vertex from which the other BBs
        // should be discoverable - the function entry block.
        bool success = DoToposort(&F.getEntryBlock(), it);
        // if (success) {
        //     // Now we have all the BBs inside SortedBBs in reverse topological
        //     // order.
        //     errs() << "toposort size=" << SortedBBs.size() << "\n";
        //     for (BBVector::const_reverse_iterator RI = SortedBBs.rbegin(), RE = SortedBBs.rend(); RI != RE; ++RI) {
        //         errs() << "name=" << (*RI)->getName() << "\n";
        //     }
        // }
        // else {
        //     errs() << "  Sorting failed\n";
        // }
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
                u32 sum1 = 0, sum2 = 0, cnt1 = 0, cnt2 = 0;
                u32 siz1 = i->second.size(), siz2 = j->second.size();
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
                        fprintf(stderr, "n:%s m:%s\n", i->first.c_str(), j->first.c_str());
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
                        fprintf(stderr, "%6.2lf\%\n", (double)(cnt1) / (sum1 * siz2) * 100);
                        // std::cerr << (double)(cnt1) / (sum1 * siz2) * 100 << "%\n";
                    }
            }
        }
        return false;
    }
};

}  // namespace

char Hello::ID         = 0;
char Obfuscator::ID    = 1;
char FunComparator::ID = 2;
char CfgComparator::ID = 3;

// Register for opt
static RegisterPass<Hello>         hello("hello", "Hello World Pass");
static RegisterPass<Obfuscator>    obfuscate("obfuscate", "Prefix Obfuscate Pass");
static RegisterPass<FunComparator> cmpfun("cmpfun", "Compare Merged IR Functions Pass");
static RegisterPass<CfgComparator> cmpcfg("cmpcfg", "Compare Merged IR CFG Pass");

// Register for clang
static RegisterStandardPasses Y(PassManagerBuilder::EP_EarlyAsPossible,
                                [](const PassManagerBuilder& Builder, legacy::PassManagerBase& PM) {
                                    PM.add(new Hello());
                                    PM.add(new Obfuscator());
                                    PM.add(new FunComparator());
                                    PM.add(new CfgComparator());
                                });