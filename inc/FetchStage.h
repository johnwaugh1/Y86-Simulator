#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Stage.h"

#ifndef FETCHSTAGE_H
#define FETCHSTAGE_H
class FetchStage: public Stage
{
	private:
		bool D_Bubble;
		bool D_Stall;
		bool F_Stall;
		void setDInput(PipeReg * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, uint64_t rA, uint64_t rB, uint64_t valC, uint64_t valP);
		uint64_t selectPC(PipeReg * freg, PipeReg * mreg, PipeReg * wreg);
		bool needRegIDs(uint64_t f_icode);
		bool needValC(uint64_t f_icode);
		uint64_t pcIncrement(uint64_t f_pc, bool needRegIds, bool needValC);
		uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
		void getRegIds(bool needregId, uint64_t f_pc, uint64_t& rA, uint64_t& rB);
		uint64_t getValC(bool needvalC, bool needregId, uint64_t f_pc);
	       	bool instr_valid(uint64_t f_icode);
       		uint64_t f_stat(uint64_t f_icode, bool instrValid, bool mem_error);	       
		bool F_stall(PipeReg * dreg, PipeReg * ereg, PipeReg * mreg);
		bool D_stall(PipeReg * ereg);
		bool D_bubble(PipeReg * dreg, PipeReg * ereg, PipeReg * mreg);
		void calculateControlSignals(PipeReg * dreg, PipeReg * ereg, PipeReg * mreg);
	public:
		//These are the only methods called outside of the class
		bool doClockLow(PipeRegArray * pipeRegs);
		void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
