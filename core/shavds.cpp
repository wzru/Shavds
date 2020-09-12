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
        // errs().write_escaped(F.getName()) << '\n';
        return false;
    }
};

struct FunComparator : public ModulePass
{
    struct File
    {
        std::string                         name;
        std::vector<llvm::Module::iterator> mi;
        std::vector<u32>                    x, a, l;
        std::vector<std::string>            n;
        void push(const llvm::Module::iterator& i,
                  const std::string&            funcName,
                  int                           line)
        {
            mi.push_back(i);
            x.push_back(0);
            a.push_back(0);
            l.push_back(line);
            n.push_back(funcName);
        }
    };
    static char ID;
    File        p, q;
    FunComparator() : ModulePass(ID) {}
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
    double LCS(llvm::Module::iterator& a, llvm::Module::iterator& b)
    {
        int   n = a->getInstructionCount(), m = b->getInstructionCount();
        int** f = new int*[n + 1];
        for (int i = 0; i <= n; ++i) { f[i] = new int[m + 1](); }
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
        return (double)f[n][m] / std::min(n, m);
    }
    bool runOnModule(Module& M) override
    {
        for (auto i = M.functions().begin(); i != M.functions().end(); ++i) {
            std::string fullName = i->getName();
            size_t      pos      = fullName.find(SHAVDS);
            if (pos != std::string::npos) {
                std::string fileName = fullName.substr(0, pos);
                std::string funcName = fullName.substr(
                    pos + SHAVDS.size(), fullName.size() - pos - SHAVDS.size());
                std::string origName =
                    systemOut(std::string("c++filt ") + funcName);
                if (isIgnored(origName)) continue;
                if (p.name == "" || p.name == fileName) {
                    p.name = fileName;
                    p.push(i, origName, i->getSubprogram()->getLine());
                }
                else {
                    q.name = fileName;
                    q.push(i, origName, i->getSubprogram()->getLine());
                }
            }
        }
        for (int i = 0; i < p.mi.size(); ++i)
            for (int j = 0; j < p.mi.size(); ++j) {
                double result = LCS(p.mi[i], p.mi[j]);
                std::cerr << p.n[i] << " in " << p.name << ":" << p.l[i] << "\n"
                          << q.n[j] << " in " << q.name << ":" << q.l[j]
                          << "\n\tare " << result * 100 << "% similar!\n";
            }
        return false;
    }
};

struct CfgComparator : public FunctionPass
{
  public:
    static char ID;
    std::string AnalysisKind = "-topo";
    CfgComparator() : FunctionPass(ID) {}
    void runToposort(const Function& F)
    {
        // errs() << "Topological sort of " << F.getName() << ":\n";
        // // Initialize the color map by marking all the vertices white.
        // for (Function::const_iterator I = F.begin(), IE = F.end(); I != IE;
        //      ++I) {
        //     ColorMap[&*I] = WHITE;
        // }
        // // The BB graph has a single entry vertex from which the other BBs
        // // should be discoverable - the function entry block.
        // // F.end()
        // bool success = recursiveDFSToposort(F);
        // if (success) {
        //     errs() << F.getName() << " has success!\n";
        //     // Now we have all the BBs inside SortedBBs in reverse
        //     topological
        //     // order.
        //     for (BBVector::const_reverse_iterator RI = SortedBBs.rbegin(),
        //                                           RE = SortedBBs.rend();
        //          RI != RE; ++RI) {
        //         errs() << "  " << (*RI)->getName() << "\n";
        //     }
        // }
        // else {
        //     errs() << "  Sorting failed\n";
        // }
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
    bool recursiveDFSToposort(Function& F)
    {
        // ColorMap[BB] = GREY;
        // auto ori     = BB;
        // F.begin()->
        // For demonstration, using the lowest-level APIs here. A BB's
        // successors are determined by looking at its terminator
        // instruction.
        //     for (BB = F.begin(); BB != F.end(); ++BB)
        // {
        //     const auto* TInst = BB->getTerminator();
        //     errs() << "num=" << TInst->getNumSuccessors() << '\n';
        //     for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I <
        //     NSucc;
        //          ++I) {
        //         BasicBlock* Succ      = TInst->getSuccessor(I);
        //         Color       SuccColor = ColorMap[Succ];
        //         if (SuccColor == WHITE) {
        //             if (!recursiveDFSToposort(Succ)) return false;
        //         }
        //         else if (SuccColor == GREY) {
        //             // This detects a cycle because grey vertices are all
        //             // ancestors of the currently explored vertex (in
        //             other
        //             // words, they're "on the stack").
        //             errs() << "  Detected cycle: edge from " <<
        //             BB->getName()
        //                    << " to " << Succ->getName() << "\n";
        //             return false;
        //         }
        //     }
        // }
        // This BB is finished (fully explored), so we can add it to the
        // vector.
        //     ColorMap[ori] = BLACK;
        // SortedBBs.push_back(ori);
        return true;
    }
    bool runOnFunction(Function& F)
    {
        errs() << "toposort begins...\n" << F.getName() << '\n';
        // runToposort(F);
        // for(auto i = F.use_begin(); i!=F.use_end(); ++i)
        // {
        //     i->get()->print(errs());
        // }
        for (auto i = F.begin(); i != F.end(); ++i)
        // for(auto j = i->begin(); j!=i->end(); ++j)
        {
            auto t = i->getTerminator();
            u32  n = t->getNumSuccessors();
            errs() << n << '\n';
        }
        return 0;
    }
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