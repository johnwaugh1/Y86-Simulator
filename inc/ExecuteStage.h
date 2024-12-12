#include "PipeRegArray.h"
#include "Stage.h"

#ifndef EXECUTESTAGE_H
#define EXECUTESTAGE_H
class ExecuteStage: public Stage
{
   private:
     	bool M_bubble;
	void setMInput(PipeReg * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM);
  	int64_t aluA(uint64_t E_icode, int64_t E_valA, int64_t E_valC);
	int64_t aluB(uint64_t E_icode, int64_t E_valB);
	uint64_t alufun(uint64_t E_icode, uint64_t E_ifun);
	bool set_cc(uint64_t E_icode, PipeReg * wreg);
	uint64_t dstE(uint64_t E_icode, uint64_t e_Cnd, uint64_t E_dstE);
	bool calculateControlSignals(PipeReg * wreg); 
	uint64_t cond(uint64_t E_icode, uint64_t E_ifun);
	uint64_t alu(uint64_t aluFun, int64_t aluA, int64_t aluB, bool set_CC);
	void cc(uint64_t aluFun, int64_t aluA, int64_t aluB, int64_t result);
   public:
      //These are the only methods called outside of the class
	bool doClockLow(PipeRegArray * pipeRegs);
     	void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
