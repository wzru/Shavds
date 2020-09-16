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
#include <type_traits>
#include <typeinfo>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace llvm;

using u32 = uint32_t;

struct FuncInfo
{
    std::string            name;
    llvm::Module::iterator mi;
    u32                    line;
    FuncInfo() = default;
    FuncInfo(const std::string&            name,
             const llvm::Module::iterator& mi,
             u32                           line)
        : name(name), mi(mi), line(line)
    {
    }
};

const std::string SHAVDS = "__SHAVDS__";

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

bool cmpOp(const llvm::Function::op_iterator& a,
           const llvm::Function::op_iterator& b)
{
    return a->get()->getType() == b->get()->getType() &&
           a->get()->getValueID() == b->get()->getValueID() &&
           a->get()->getNumUses() == b->get()->getNumUses() &&
           a->get()->getValueName() == b->get()->getValueName() &&
           a->get()->getName() == b->get()->getName() &&
           a->get()->getRawSubclassOptionalData() ==
               b->get()->getRawSubclassOptionalData();
}

bool startWith(const std::string& s1, const std::string& s2)
{
    return s1.compare(0, s2.size(), s2) == 0;
}

bool isIgnored(const std::string& name)
{
    return startWith(name, "_GLOBAL") || name == "main";
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
    static char                                  ID;
    std::map<std::string, std::vector<FuncInfo>> mfi;
    FunComparator() : ModulePass(ID) {}
    u32 LCS(llvm::Module::iterator& a, llvm::Module::iterator& b)
    {
        int   n = a->getInstructionCount(), m = b->getInstructionCount();
        u32** f = new u32*[n + 1];
        for (int i = 0; i <= n; ++i) { f[i] = new u32[m + 1](); }
        int                                      x = 1;
        std::vector<llvm::Function::op_iterator> u, v;
        for (auto i = a->begin(); i != a->end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                for (auto k = j->op_begin(); k != j->op_end(); ++k)
                    u.push_back(k);
        for (auto i = b->begin(); i != b->end(); ++i)
            for (auto j = i->begin(); j != i->end(); ++j)
                for (auto k = j->op_begin(); k != j->op_end(); ++k)
                    v.push_back(k);
        for (auto i = 0; i < n; ++i) {
            for (auto j = 0; j < m; ++j) {
                if (cmpOp(u[i], v[j]))
                    f[i + 1][j + 1] = f[i][j] + 1;
                else
                    f[i + 1][j + 1] = std::max(f[i][j + 1], f[i + 1][j]);
            }
        }
        // for (int i = 0; i <= n; ++i) { delete[] f[i]; }
        // delete[] f;
        return f[n][m];
    }
    double RepRate(llvm::Module::iterator& a, llvm::Module::iterator& b)
    {
        return (double)LCS(a, b) /
               std::min(a->getInstructionCount(), b->getInstructionCount());
    }
    bool runOnModule(Module& M) override
    {
        for (auto i = M.functions().begin(); i != M.functions().end();
             ++i) {  //遍历模块中的函数
            std::string fullName = i->getName();
            size_t      pos      = fullName.find(SHAVDS);
            if (pos != std::string::npos) {  //如果是目标函数
                std::string fileName = fullName.substr(0, pos);
                std::string funcName = fullName.substr(
                    pos + SHAVDS.size(), fullName.size() - pos - SHAVDS.size());
                std::string origName =
                    systemOut(std::string("c++filt ") + funcName);
                std::cerr << "funcName=" << funcName << "\n";
                if (isIgnored(origName)) continue;
                mfi[fileName].push_back(
                    FuncInfo(origName, i, i->getSubprogram()->getLine()));
            }
        }
        std::cerr << "ready to compare...\n";
        for (auto i = mfi.begin(); i != mfi.end(); ++i) {
            auto tmp = i;
            for (auto j = ++tmp; j != mfi.end(); ++j) {
                std::cerr << "Now compare " << i->first << " and " << j->first
                          << "\n";
                for (auto ii : i->second)
                    for (auto jj : j->second) {
                        double res = RepRate(ii.mi, jj.mi);
                        std::cerr
                            << ii.name << "(" << ii.mi->getInstructionCount()
                            << " insts) in " << i->first << ":" << ii.line
                            << "\n"
                            << jj.name << "(" << jj.mi->getInstructionCount()
                            << " insts) in " << j->first << ":" << jj.line
                            << "\n\tare " << res * 100 << "% similar!\n";
                    }
            }
        }
        // for (int i = 0; i < p.mi.size(); ++i)
        //     for (int j = 0; j < p.mi.size(); ++j) {
        //         double result = LCS(p.mi[i], p.mi[j]);
        //         std::cerr << p.n[i] << " in " << p.name << ":" << p.l[i] <<
        //         "\n"
        //                   << q.n[j] << " in " << q.name << ":" << q.l[j]
        //                   << "\n\tare " << result * 100 << "% similar!\n";
        //     }
        return false;
    }
};

struct CfgComparator : public ModulePass
{
  public:
    static char ID;
    std::string AnalysisKind = "-topo";
    CfgComparator() : ModulePass(ID) {}
    void runToposort(const Function& F)
    {
        errs() << "Topological sort of " << F.getName() << ":\n";
        // Initialize the color map by marking all the vertices white.
        for (Function::const_iterator I = F.begin(), IE = F.end(); I != IE;
             ++I) {
            ColorMap[&*I] = WHITE;
        }
        // The BB graph has a single entry vertex from which the other BBs
        // should be discoverable - the function entry block.
        bool success = recursiveDFSToposort(&F.getEntryBlock());
        if (success) {
            // Now we have all the BBs inside SortedBBs in reverse topological
            // order.
            errs() << "toposort size=" << SortedBBs.size() << "\n";
            for (BBVector::const_reverse_iterator RI = SortedBBs.rbegin(),
                                                  RE = SortedBBs.rend();
                 RI != RE; ++RI) {
                errs() << "name=" << (*RI)->getName() << "\n";
            }
        }
        else {
            errs() << "  Sorting failed\n";
        }
    }

  private:
    enum Color { WHITE, GREY, BLACK };
    // Color marks per vertex (BB).
    typedef DenseMap<const BasicBlock*, Color> BBColorMap;
    // Collects vertices (BBs) in "finish" order. The first finished vertex is
    // first, and so on.
    typedef SmallVector<const BasicBlock*, 32> BBVector;
    BBColorMap                                 ColorMap;
    BBVector                                   SortedBBs;

    // Helper function to recursively run topological sort from a given BB.
    // Returns true if the sort succeeded and false otherwise; topological sort
    // may fail if, for example, the graph is not a DAG (detected a cycle).
    bool recursiveDFSToposort(const BasicBlock* BB)
    {
        ColorMap[BB] = GREY;
        // For demonstration, using the lowest-level APIs here. A BB's
        // successors are determined by looking at its terminator instruction.
        const auto* TInst = BB->getTerminator();
        errs() << BB->getParent()->getName();
        errs() << " num=" << TInst->getNumSuccessors() << '\n';
        for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc;
             ++I) {
            BasicBlock* Succ      = TInst->getSuccessor(I);
            Color       SuccColor = ColorMap[Succ];
            if (SuccColor == WHITE) {
                if (!recursiveDFSToposort(Succ)) return false;
            }
            else if (SuccColor == GREY) {
                // This detects a cycle because grey vertices are all ancestors
                // of the currently explored vertex (in other words, they're "on
                // the stack").
                errs() << "  Detected cycle: edge from " << BB->getName()
                       << " to " << Succ->getName() << "\n";
                return false;
            }
        }
        // This BB is finished (fully explored), so we can add it to the vector.
        ColorMap[BB] = BLACK;
        SortedBBs.push_back(BB);
        return true;
    }
    bool runOnModule(Module& M)
    {
        errs() << "toposort begins...\n";
        // if (AnalysisKind == "-topo") {

        return false;
    }
    // The address of this member is used to uniquely identify the class. This
    // is used by LLVM's own RTTI mechanism.
};

}  // namespace

char Hello::ID         = 0;
char Obfuscator::ID    = 1;
char FunComparator::ID = 2;
char CfgComparator::ID = 3;

// Register for opt
static RegisterPass<Hello>      hello("hello", "Hello World Pass");
static RegisterPass<Obfuscator> obfuscate("obfuscate", "Prefix Obfuscate Pass");
static RegisterPass<FunComparator> cmpfun("cmpfun",
                                          "Compare Merged IR Functions Pass");
static RegisterPass<CfgComparator> cmpcfg("cmpcfg",
                                          "Compare Merged IR CFG Pass");

// Register for clang
static RegisterStandardPasses Y(PassManagerBuilder::EP_EarlyAsPossible,
                                [](const PassManagerBuilder& Builder,
                                   legacy::PassManagerBase&  PM) {
                                    PM.add(new Hello());
                                    PM.add(new Obfuscator());
                                    PM.add(new FunComparator());
                                    PM.add(new CfgComparator());
                                });