#include "llvm-c/Types.h"
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
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/FunctionComparator.h"

#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace llvm;

const std::string SHAVDS = "__SHAVDS__";

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
        errs() << "Obfuscator: ";
        std::string fileName = (F.getParent())->getSourceFileName();
        F.setName(fileName + SHAVDS + F.getName());
        errs().write_escaped(F.getName()) << '\n';
        return false;
    }
};

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
    return std::string(buf);
}

struct Comparator : public ModulePass
{
    using u32 = uint32_t;
    static char ID;
    struct File
    {
        std::string                         name;
        std::vector<llvm::Module::iterator> mi;
        std::vector<u32>                    x, a, l;
        std::vector<std::string>            n;
        void push(const llvm::Module::iterator& i, const std::string& funcName)
        {
            mi.push_back(i);
            x.push_back(0);
            a.push_back(0);
            l.push_back(0);
            n.push_back(funcName);
        }
    };
    bool cmpBasicBlocks(const llvm::Function::iterator& a,
                        const llvm::Function::iterator& b)
    {
        auto j = b->begin();
        for (auto i = a->begin(); i != a->end(); ++i) {
            if (j == b->end()) return false;
            auto jj = j->op_begin();
            for (auto ii = i->op_begin(); ii != i->op_end(); ++ii, ++jj) {
                if (jj == j->op_end()) return false;
                if (ii->getOperandNo() != jj->getOperandNo()) return false;
            }
            if (jj != j->op_end()) return false;
        }
        if (j != b->end()) return false;
        return true;
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
        //    && a->get()->get
        // a->get()->getContext() == b->get()->getContext();
    }
    double LCS(llvm::Module::iterator& a, llvm::Module::iterator& b)
    {
        int   n = a->getInstructionCount(), m = b->getInstructionCount();
        int** f = new int*[n + 1];
        for (int i = 0; i <= n; ++i) { f[i] = new int[m + 1](); }
        int x = 1;
        // for (auto i = a->op_begin(); i != a->op_end(); ++i, ++x) {
        //     int y = 1;
        //     for (auto j = b->op_begin(); j != b->op_end(); ++j, ++y) {
        //         std::cerr << "opi=" << i->getOperandNo()
        //                   << " opj=" << j->getOperandNo() << '\n';
        //         if (i->getOperandNo() == j->getOperandNo())
        //             f[x][y] = f[x - 1][y - 1] + 1;
        //         else
        //             f[x][y] = std::max(f[x][y - 1], f[x - 1][y]);
        //     }
        // }
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
                // std::cerr << "opi=" << u[i]->get()->getType()
                //           << " opj=" << v[j]->get()->getType() << '\n';
                if (cmpOp(u[i], v[j]))
                    f[i + 1][j + 1] = f[i][j] + 1;
                else
                    f[i + 1][j + 1] = std::max(f[i][j + 1], f[i + 1][j]);
            }
        }
        // for (auto i = a->begin(); i != a->end(); ++i, ++x) {
        //     int y = 1;
        //     for (auto j = b->begin(); j != b->end(); ++j, ++y)
        //         if (cmpBasicBlocks(i, j))
        //             f[x][y] = f[x - 1][y - 1] + 1;
        //         else
        //             f[x][y] = std::max(f[x][y - 1], f[x - 1][y]);
        // }
        std::cerr << "-------" << f[n][m] << ',' << n << ',' << m << '\n';
        return (double)f[n][m] / std::min(n, m);
    }
    File p, q;
    Comparator() : ModulePass(ID) {}
    bool runOnModule(Module& M) override
    {
        errs() << "Module Name: ";
        errs().write_escaped(M.getName()) << '\n';
        for (auto i = M.functions().begin(); i != M.functions().end(); ++i) {
            std::string fullName = i->getName();
            size_t      pos      = fullName.find(SHAVDS);
            if (pos != std::string::npos) {
                std::string fileName = fullName.substr(0, pos);
                std::string funcName = fullName.substr(
                    pos + SHAVDS.size(), fullName.size() - pos - SHAVDS.size());
                std::string origName =
                    systemOut(std::string("c++filt ") + funcName);
                errs() << "Original Name=" << origName << '\n';
                if (p.name == "" || p.name == fileName) {
                    p.name = fileName;
                    p.push(i, origName);
                }
                else {
                    q.name = fileName;
                    q.push(i, origName);
                }
                // errs() << fileName << " Operand1: ";
                // auto F = (*i).getFunction();
                // for()
                // for (auto j = i->op_begin(); j != i->op_end(); ++j) {
                //     errs() << j->get() << '\n';
                //     auto v = j->get();
                //     v->printAsOperand(errs());
                // }
                errs() << fileName << " Operand: ";
                errs() << "name=" << i->getName() << '\n';
                errs() << "inst_cnt=" << i->getInstructionCount() << '\n';
                for (auto j = i->begin(); j != i->end(); ++j) {
                    j->print(errs());
                }
            }
        }
        for (int i = 0; i < p.mi.size(); ++i)
            for (int j = 0; j < p.mi.size(); ++j) {
                double result = LCS(p.mi[i], p.mi[j]);
                std::cerr << p.n[i] << " and " << q.n[j] << " are "
                          << result * 100 << "% similar\n";
            }
        return false;
    }
};

}  // namespace

char Hello::ID      = 0;
char Obfuscator::ID = 1;
char Comparator::ID = 2;

// Register for opt
static RegisterPass<Hello>      hello("hello", "Hello World Pass");
static RegisterPass<Obfuscator> obfuscate("obfuscate", "Prefix Obfuscate Pass");
static RegisterPass<Comparator> compare("compare", "Compare Merge IR Pass");

// Register for clang
static RegisterStandardPasses Y(PassManagerBuilder::EP_EarlyAsPossible,
                                [](const PassManagerBuilder& Builder,
                                   legacy::PassManagerBase&  PM) {
                                    PM.add(new Hello());
                                    PM.add(new Obfuscator());
                                    PM.add(new Comparator());
                                });