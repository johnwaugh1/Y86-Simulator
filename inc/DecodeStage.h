#include "PipeRegArray.h"
#include "Stage.h"

#ifndef DECODESTAGE_H
#define DECODESTAGE_H
class DecodeStage: public Stage
{
	private:
		bool E_bubble;
		void setEInput(PipeReg * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t valC, uint64_t valA, uint64_t valB, uint64_t dstE, uint64_t  dstM, uint64_t srcA, uint64_t srcB);
		uint64_t srcA (uint64_t D_icode, uint64_t rA);
		uint64_t srcB (uint64_t D_icode, uint64_t rB);
		uint64_t dstE (uint64_t D_icode, uint64_t rB);			
		uint64_t dstM (uint64_t D_icode, uint64_t rA);
		uint64_t SelFwdA (uint64_t D_icode, uint64_t D_valP, uint64_t srcA, uint64_t d_rvalA, PipeReg * mreg, PipeReg * wreg);
		uint64_t FwdB(uint64_t srcB, uint64_t d_rvalB, PipeReg * mreg, PipeReg * wreg);
		bool calculateControlSignals(PipeReg * ereg);
	public:
		//These are the only methods called outside of the class
		bool doClockLow(PipeRegArray * pipeRegs);
		void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
