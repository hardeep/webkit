/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "DFGVirtualRegisterAllocationPhase.h"

#if ENABLE(DFG_JIT)

#include "DFGGraph.h"
#include "DFGScoreBoard.h"

namespace JSC { namespace DFG {

class VirtualRegisterAllocationPhase : public Phase {
public:
    VirtualRegisterAllocationPhase(Graph& graph)
        : Phase(graph, "virtual register allocation")
    {
    }
    
    void run()
    {
#if DFG_ENABLE(DEBUG_VERBOSE)
        dataLog("Preserved vars: ");
        m_graph.m_preservedVars.dump(WTF::dataFile());
        dataLog("\n");
#endif
        ScoreBoard scoreBoard(m_graph, m_graph.m_preservedVars);
        for (size_t i = 0; i < m_graph.size(); ++i) {
            Node& node = m_graph[i];
        
            if (!node.shouldGenerate() || node.op == Phi || node.op == Flush)
                continue;
            
            // GetLocal nodes are effectively phi nodes in the graph, referencing
            // results from prior blocks.
            if (node.op != GetLocal) {
                // First, call use on all of the current node's children, then
                // allocate a VirtualRegister for this node. We do so in this
                // order so that if a child is on its last use, and a
                // VirtualRegister is freed, then it may be reused for node.
                if (node.op & NodeHasVarArgs) {
                    for (unsigned childIdx = node.firstChild(); childIdx < node.firstChild() + node.numChildren(); childIdx++)
                        scoreBoard.use(m_graph.m_varArgChildren[childIdx]);
                } else {
                    scoreBoard.use(node.child1());
                    scoreBoard.use(node.child2());
                    scoreBoard.use(node.child3());
                }
            }

            if (!node.hasResult())
                continue;

            node.setVirtualRegister(scoreBoard.allocate());
            // 'mustGenerate' nodes have their useCount artificially elevated,
            // call use now to account for this.
            if (node.mustGenerate())
                scoreBoard.use(i);
        }

        // 'm_numCalleeRegisters' is the number of locals and temporaries allocated
        // for the function (and checked for on entry). Since we perform a new and
        // different allocation of temporaries, more registers may now be required.
        unsigned calleeRegisters = scoreBoard.highWatermark() + m_graph.m_parameterSlots;
        if ((unsigned)codeBlock()->m_numCalleeRegisters < calleeRegisters)
            codeBlock()->m_numCalleeRegisters = calleeRegisters;
#if DFG_ENABLE(DEBUG_VERBOSE)
        dataLog("Num callee registers: %u\n", calleeRegisters);
#endif
    }
};

void performVirtualRegisterAllocation(Graph& graph)
{
    runPhase<VirtualRegisterAllocationPhase>(graph);
}

} } // namespace JSC::DFG

#endif // ENABLE(DFG_JIT)
