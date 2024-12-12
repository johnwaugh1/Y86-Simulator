#include "PipeRegArray.h"
#include "ExecuteStage.h"
#include "RegisterFile.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Instruction.h"
#include "ConditionCodes.h"
#include "Tools.h"
#include "Stage.h"
#include "Status.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool ExecuteStage::doClockLow(PipeRegArray * pipeRegs)
{

	PipeReg * ereg = pipeRegs -> getExecuteReg();
	PipeReg * mreg = pipeRegs -> getMemoryReg();
	PipeReg * wreg = pipeRegs -> getWritebackReg();

	uint64_t stat = ereg -> get(E_STAT);
	uint64_t icode = ereg -> get(E_ICODE);
	uint64_t ifun = ereg -> get(E_IFUN);
	uint64_t valA = ereg -> get(E_VALA);
	uint64_t valB = ereg -> get(E_VALB);
	Stage::e_Cnd = cond(icode, ifun);

	Stage::e_dstE = dstE(icode, Stage::e_Cnd, ereg -> get(E_DSTE));
	uint64_t dstM = ereg -> get(E_DSTM);

	int64_t alu_A = aluA(icode, valA, ereg -> get(E_VALC));
	int64_t alu_B = aluB(icode, valB);
	uint64_t aluFun = alufun(icode, ifun);
	bool set_CC = set_cc(icode, wreg);	
	M_bubble = calculateControlSignals(wreg);
	Stage::e_valE = alu(aluFun, alu_A, alu_B, set_CC);
	
	setMInput(mreg, stat, icode, Stage::e_Cnd, e_valE, valA, e_dstE, dstM);

	return false;
}
/*
 * setMInput
 *
 * Sets the values in the Memory pipeline register.
 *
 * @param: mreg - pointer to the Memory pipeline register
 * @param: stat, icode, cnd, valE, valA, dstE, dstM - values to be set in the register
 */
void ExecuteStage::setMInput(PipeReg * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM)
{
	mreg -> set(M_STAT, stat);
	mreg -> set(M_ICODE, icode);
	mreg -> set(M_CND, cnd);
	mreg -> set(M_VALE, valE);
	mreg -> set(M_VALA, valA);
	mreg -> set(M_DSTE, dstE);
	mreg -> set(M_DSTM, dstM);
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void ExecuteStage::doClockHigh(PipeRegArray * pipeRegs)
{
	PipeReg * mreg = pipeRegs->getMemoryReg();
	if (M_bubble == false) {
		mreg -> normal();
	} else {
		((M *)mreg) -> bubble();
	}
}
/*
 * aluA
 *
 * Determines the ALU A input based on the instruction code.
 *
 * @param: E_icode - instruction code
 * @param: E_valA, E_valC - potential inputs to the ALU
 * @return: ALU A input
 */
int64_t ExecuteStage::aluA(uint64_t E_icode, int64_t E_valA, int64_t E_valC)
{
	if(E_icode == Instruction::IRRMOVQ || E_icode == Instruction::IOPQ){
	       return E_valA;
	}
	if(E_icode == Instruction::IIRMOVQ || E_icode == Instruction::IRMMOVQ|| E_icode == Instruction::IMRMOVQ){
	       return E_valC;
	}
	if(E_icode == Instruction::ICALL || E_icode == Instruction::IPUSHQ){
		return -8;
	}
	if(E_icode == Instruction::IRET || E_icode == Instruction::IPOPQ){
		return 8;
	}
	else{
		return 0;
	}
}
/*
 * aluB
 *
 * Determines the ALU B input based on the instruction code.
 *
 * @param: E_icode - instruction code
 * @param: E_valB - potential input to the ALU
 * @return: ALU B input
 */
int64_t ExecuteStage::aluB(uint64_t E_icode, int64_t E_valB)
{
	if(E_icode == Instruction::IRMMOVQ || E_icode == Instruction::IMRMOVQ|| E_icode == Instruction::IOPQ|| E_icode == Instruction::ICALL || E_icode == Instruction::IPUSHQ || E_icode == Instruction::IRET || E_icode == Instruction::IPOPQ){
	       return E_valB;
	}
	if(E_icode == Instruction::IRRMOVQ|| E_icode == Instruction::IIRMOVQ)
	{
	       return 0;
	}
	else{
		return 0;
	}
}
/*
 * alufun
 *
 * Determines the ALU function code based on the instruction code.
 *
 * @param: E_icode - instruction code
 * @param: E_ifun - instruction function
 * @return: ALU function code
 */
uint64_t ExecuteStage::alufun(uint64_t E_icode, uint64_t E_ifun)
{
	if(E_icode == Instruction::IOPQ)
	{
		return E_ifun;
	}
	else
	{
		return Instruction::ADDQ;
	}
}
/*
 * set_cc
 *
 * Determines whether condition codes should be set.
 *
 * @param: E_icode - instruction code
 * @param: wreg - pointer to the Writeback pipeline register
 * @return: true if condition codes should be set; false otherwise
 */
bool ExecuteStage::set_cc(uint64_t E_icode, PipeReg * wreg)
{
	uint64_t W_stat = wreg -> get(W_STAT); 	

	return E_icode == Instruction::IOPQ && (Stage::m_stat != Status::SADR && Stage::m_stat != Status::SINS && Stage::m_stat != Status::SHLT) && (W_stat != Status::SADR && W_stat != Status::SINS && W_stat != Status::SHLT);
}
/*
 * calculateControlSignals
 *
 * Determines whether the Memory pipeline register should bubble.
 *
 * @param: wreg - pointer to the Writeback pipeline register
 * @return: true if the Memory register should bubble; false otherwise
 */
bool ExecuteStage::calculateControlSignals(PipeReg * wreg) 
{
	uint64_t W_stat = wreg -> get(W_STAT);

	return ((Stage::m_stat == Status::SADR || Stage::m_stat == Status::SINS || Stage::m_stat == Status::SHLT) || (W_stat == Status::SADR || W_stat == Status::SINS || W_stat == Status::SHLT));
}
/*
 * dstE
 *
 * Determines the destination register for the result.
 *
 * @param: E_icode - instruction code
 * @param: e_Cnd - condition code
 * @param: E_dstE - default destination
 * @return: destination register
 */
uint64_t ExecuteStage::dstE(uint64_t E_icode, uint64_t e_Cnd, uint64_t E_dstE){
	if(E_icode == Instruction::IRRMOVQ && !e_Cnd)
	{
  		return RegisterFile::RNONE;
	}
	else
	{
		return E_dstE;
	}
}
/*
 * cond
 *
 * Evaluates the condition codes for conditional instructions.
 *
 * @param: E_icode - instruction code
 * @param: E_ifun - instruction function
 * @return: condition code result
 */
uint64_t ExecuteStage::cond(uint64_t E_icode, uint64_t E_ifun)
{
	if (E_icode != Instruction::IJXX && E_icode != Instruction::ICMOVXX) 
	{
		return 0;
	}
	bool error = false;
	ConditionCodes * cc = ConditionCodes::getInstance();
	uint64_t sf = cc -> getConditionCode(ConditionCodes::SF, error); 
	uint64_t of = cc -> getConditionCode(ConditionCodes::OF, error);
	uint64_t zf = cc -> getConditionCode(ConditionCodes::ZF, error);

	switch (E_ifun) {
		case Instruction::UNCOND:
			return 1;
		case Instruction::LESSEQ:
			return (sf ^ of) | zf;
		case Instruction::LESS:
			return (sf ^ of);
		case Instruction::EQUAL:
			return zf;
		case Instruction::NOTEQUAL:
			return !zf;
		case Instruction::GREATER:
			return !(sf ^ of) & !zf;
		case Instruction::GREATEREQ:
			return !(sf ^ of);
		default:
			return 0;
	}
}
/*
 * alu
 *
 * Executes the ALU operation and optionally updates condition codes.
 *
 * @param: aluFun - ALU function code
 * @param: aluA, aluB - ALU inputs
 * @param: set_cc - flag to update condition codes
 * @return: ALU result
 */
uint64_t ExecuteStage::alu(uint64_t aluFun, int64_t aluA, int64_t aluB, bool set_CC)
{
	int64_t result = 0;
	if(aluFun == Instruction::ADDQ){
		result = aluA + aluB;
		if (set_CC) ExecuteStage::cc(aluFun, aluA, aluB, result);
		return result;
	}
	else if(aluFun == Instruction::SUBQ){
		result = aluB - aluA;
		if (set_CC) ExecuteStage::cc(aluFun, aluA, aluB, result);
		return result;
	}
	else if(aluFun == Instruction::ANDQ){
		result = aluA & aluB;
		if (set_CC) ExecuteStage::cc(aluFun, aluA, aluB, result);
		return result;
	}
	else if(aluFun == Instruction::XORQ){
		result = aluA ^ aluB;
		if (set_CC) ExecuteStage::cc(aluFun, aluA, aluB, result);
		return result;
	}
	else {
		return 0;
	}
}
/*
 * cc
 *
 * Updates the condition codes (ZF, SF, OF) based on the ALU operation and result.
 *
 * @param: aluFun - ALU function code (e.g., ADDQ, SUBQ, ANDQ, XORQ)
 * @param: aluA - First operand for the ALU operation
 * @param: aluB - Second operand for the ALU operation
 * @param: result - Result of the ALU operation
 */
void ExecuteStage::cc(uint64_t aluFun, int64_t aluA, int64_t aluB, int64_t result)
{
	ConditionCodes * cc = ConditionCodes::getInstance();
	bool error = false;

	switch(aluFun){
		case Instruction::ADDQ:
		case Instruction::SUBQ:

			cc->setConditionCode(result == 0, ConditionCodes::ZF, error);
            		cc->setConditionCode(result < 0, ConditionCodes::SF, error);

			cc->setConditionCode(
                		aluFun == Instruction::ADDQ ? Tools::addOverflow(aluA, aluB) : Tools::subOverflow(aluA, aluB), ConditionCodes::OF, error);
			break;	
		
		case Instruction::ANDQ:
        	case Instruction::XORQ:
            
            	cc->setConditionCode(result == 0, ConditionCodes::ZF, error);
            	cc->setConditionCode(result < 0, ConditionCodes::SF, error);
            	cc->setConditionCode(false, ConditionCodes::OF, error);
            	break;
	}
}
