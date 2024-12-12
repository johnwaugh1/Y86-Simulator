#include <cstdint>
#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Memory.h"
#include "FetchStage.h"
#include "Instruction.h"
#include "RegisterFile.h"
#include "Status.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Tools.h"
/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
 * (F, D, E, M, W instances)
 */
bool FetchStage::doClockLow(PipeRegArray * pipeRegs)
{
	PipeReg * freg = pipeRegs->getFetchReg();
	PipeReg * dreg = pipeRegs->getDecodeReg();
	PipeReg * ereg = pipeRegs->getExecuteReg();
	PipeReg * wreg = pipeRegs->getWritebackReg();
	PipeReg * mreg = pipeRegs->getMemoryReg();

	bool mem_error = false;

	uint64_t icode = Instruction::INOP, ifun = Instruction::FNONE;
	uint64_t rA = RegisterFile::RNONE, rB = RegisterFile::RNONE;
	uint64_t valC = 0, valP = 0, stat = Status::SAOK, predPC = 0;
		
	uint64_t f_pc = selectPC(freg, mreg, wreg);
	Memory * memory = Memory::getInstance();
	uint8_t instructionByte = 0;
	instructionByte = memory->getByte(f_pc, mem_error);
	icode = instructionByte >> 4;
	ifun = instructionByte & 0xF;

	if(icode == Instruction::IHALT){
		stat = Status::SHLT;
	}

	uint64_t f_icode, f_ifun;
	if(mem_error){
		f_icode = Instruction::INOP, f_ifun = Instruction::FNONE;
	} else {
		f_icode = icode, f_ifun = ifun;
	}

	bool instrValid = this->instr_valid(f_icode);
	stat = f_stat(f_icode, instrValid, mem_error);

	bool needvalC = this->needValC(icode);
	bool needregId = this->needRegIDs(icode); 
	getRegIds(needregId, f_pc, rA, rB);
	valC = getValC(needvalC, needregId, f_pc);

	valP = pcIncrement(f_pc, needregId, needvalC);
	predPC = predictPC(icode, valC, valP);
		freg->set(F_PREDPC, predPC);
	
	calculateControlSignals(dreg, ereg, mreg);

	setDInput(dreg, stat, f_icode, f_ifun, rA, rB, valC, valP);
	return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeRegArray * pipeRegs)
{
	PipeReg * freg = pipeRegs->getFetchReg();  
	PipeReg * dreg = pipeRegs->getDecodeReg();
	
	if(D_Bubble){
		((D *)dreg)->bubble();
	}
	else if (!D_Stall)
	{
		dreg->normal();
	}
	if(!F_Stall){
		freg->normal();
	}
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
 */
void FetchStage::setDInput(PipeReg * dreg, uint64_t stat, uint64_t icode,
		uint64_t ifun, uint64_t rA, uint64_t rB,
		uint64_t valC, uint64_t valP)
{
	dreg->set(D_STAT, stat);
	dreg->set(D_ICODE, icode);
	dreg->set(D_IFUN, ifun);
	dreg->set(D_RA, rA);
	dreg->set(D_RB, rB);
	dreg->set(D_VALC, valC);
	dreg->set(D_VALP, valP);
}
/*
 * selectPC
 *
 * Determines the appropriate program counter (PC) value for fetching the next instruction.
 *
 * @param: freg, mreg, wreg - Fetch, Memory, and Writeback pipeline registers.
 * @return: The selected PC value based on control flow and instruction type.
 */
uint64_t FetchStage::selectPC(PipeReg * freg, PipeReg * mreg, PipeReg * wreg){
	uint64_t M_icode = mreg->get(M_ICODE);
	uint64_t M_cnd = mreg->get(M_CND);
	uint64_t M_valA = mreg->get(M_VALA);
	uint64_t W_icode = wreg->get(W_ICODE);
	uint64_t W_valM = wreg->get(W_VALM);
	uint64_t F_predPC = freg->get(F_PREDPC);

	if(M_icode == Instruction::IJXX && !M_cnd){
		return M_valA;
	} else if(W_icode == Instruction::IRET){
		return W_valM;
	} else {
		return F_predPC;
	}
}
/*
 * needRegIDs
 *
 * Determines whether the current instruction requires register IDs.
 *
 * @param: f_icode - the icode value of the current instruction.
 * 
 * This function checks if the current instruction involves registers (like ADD, MOV, etc.)
 * and requires register IDs to be fetched.
 * 
 * @return: bool - true if register IDs are needed, false otherwise.
 */
bool FetchStage::needRegIDs(uint64_t f_icode){
	return f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IOPQ ||
		f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ ||
		f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ ||
		f_icode == Instruction::IMRMOVQ;
}
/*
 * needValC
 *
 * Determines whether the current instruction requires an immediate value (valC).
 *
 * @param: f_icode - the icode value of the current instruction.
 * 
 * This function checks if the current instruction requires a constant value (such as an immediate value).
 * 
 * @return: bool - true if valC is needed, false otherwise.
 */
bool FetchStage::needValC(uint64_t f_icode) {
	return f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ ||
		f_icode == Instruction::IMRMOVQ || f_icode == Instruction::IJXX ||
		f_icode == Instruction::ICALL;
}
/*
 * pcIncrement
 *
 * Calculates the address of the next instruction based on the current PC and the type of instruction.
 *
 * @param: f_pc - the current PC value
 * @param: needregId - flag indicating whether the instruction requires register IDs
 * @param: needvalC - flag indicating whether the instruction requires an immediate value
 * 
 * This function increments the PC based on the instruction type, considering whether 
 * the instruction uses register IDs and an immediate value.
 * 
 * @return: uint64_t - the next PC value.
 */
uint64_t FetchStage::pcIncrement(uint64_t f_pc, bool needregId, bool needvalC){
	uint64_t offset = 1;
	if(needregId) offset += 1;
	if(needvalC) offset += 8;
	return f_pc + offset;
}
/*
 * predictPC
 *
 * Calculates the predicted next Program Counter (PC) based on the current instruction.
 *
 * @param: f_icode - the icode value of the current instruction
 * @param: f_valC - the constant value (valC) associated with the current instruction
 * @param: f_valP - the next PC value (PC + 1)
 * 
 * This function predicts the next instruction’s PC based on whether the current instruction
 * is a jump or call. If the instruction is a jump (IJXX) or a call (ICALL), the next PC 
 * is predicted to be the constant value (valC). Otherwise, the next sequential instruction is selected.
 * 
 * @return: uint64_t - the predicted next PC value.
 */
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP) {
	if (f_icode == Instruction::IJXX || f_icode == Instruction::ICALL) {
		return f_valC;
	} else {
		return f_valP;
	}
}
/*
 * getRegIds
 *
 * Fetches the register IDs for the current instruction, if required.
 *
 * @param: needregId - flag indicating whether register IDs are needed for the instruction
 * @param: f_pc - the current PC value
 * @param: rA - reference to store the first register ID
 * @param: rB - reference to store the second register ID
 * 
 * This function retrieves the register IDs from memory if the instruction requires them. 
 * It fetches the register byte and extracts the register IDs for the source and destination registers.
 */
void FetchStage::getRegIds(bool needregId, uint64_t f_pc, uint64_t& rA, uint64_t& rB) {

	if (needregId) {
		bool flag;
		uint8_t regByte = mem->getByte(f_pc + 1, flag);
		rA = Tools::getBits(regByte, 4, 7);
		rB = Tools::getBits(regByte, 0, 3);
	}
}
/*
 * getValC
 *
 * Fetches the constant value (valC) for the current instruction, if required.
 *
 * @param: needvalC - flag indicating whether the instruction requires a constant value
 * @param: needregId - flag indicating whether register IDs are required
 * @param: f_pc - the current PC value
 * 
 * This function retrieves the constant value (valC) from memory if the instruction requires it. 
 * The address from where the value is fetched depends on whether register IDs are used.
 * 
 * @return: uint64_t - the fetched constant value (valC).
 */
uint64_t FetchStage::getValC(bool needvalC, bool needregId, uint64_t f_pc){
	if (needvalC) {
		bool flag;
		uint8_t memBytes[8];
		uint64_t address = f_pc + (needregId ? 2 : 1);

		for (int i = 0; i < 8; i++) {
			memBytes[i] = mem->getByte(address, flag);
			address++;	
		}
		return Tools::buildLong(memBytes);
	}
	return 0;	
}
/*
 * instr_valid
 *
 * Checks if the current instruction is valid.
 *
 * @param: f_icode - the icode value of the current instruction
 * 
 * This function checks whether the given instruction opcode (icode) is valid.
 * Valid instructions are the ones defined in the Instruction enum, including HALT and NOP.
 * 
 * @return: bool - true if the instruction is valid, false otherwise.
 */
bool FetchStage::instr_valid(uint64_t f_icode){
	return f_icode == Instruction::INOP ||f_icode == Instruction::IHALT ||f_icode == Instruction::IRRMOVQ ||f_icode == Instruction::IIRMOVQ ||f_icode == Instruction::IRMMOVQ ||f_icode == Instruction::IOPQ ||f_icode == Instruction::IJXX ||f_icode == Instruction::ICALL ||f_icode == Instruction::IRET ||f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ || f_icode == Instruction::IMRMOVQ;
}
/*
 * f_stat
 *
 * Determines the status of the instruction based on its validity and memory error.
 *
 * @param: f_icode - the icode value of the current instruction
 * @param: instrValid - flag indicating whether the instruction is valid
 * @param: mem_error - flag indicating whether a memory error occurred
 * 
 * This function returns the status code for the instruction based on its validity and 
 * whether a memory error occurred. It handles cases like ADR (address error), INS (invalid instruction),
 * and HLT (halt).
 * 
 * @return: uint64_t - the status code indicating the state of the instruction.
 */
uint64_t FetchStage::f_stat(uint64_t f_icode, bool instrValid, bool mem_error){
	if(mem_error){
		return Status::SADR;
	} else if (!instrValid){
		return Status::SINS;
	} else if (f_icode == Instruction::IHALT){
		return Status::SHLT;
	} else {
		return Status::SAOK;
	}
}
/*
 * F_stall
 *
 * Checks if the Fetch stage needs to stall based on the conditions of the pipeline.
 *
 * @param: dreg - pointer to the Decode register instance
 * @param: ereg - pointer to the Execute register instance
 * @param: mreg - pointer to the Memory register instance
 * 
 * This function checks for dependencies that require stalling in the Fetch stage. 
 * If certain instructions like IMRMOVQ or IPOPQ are in the pipeline and need data from the 
 * current instruction, the Fetch stage will stall.
 * 
 * @return: bool - true if a stall is needed, false otherwise.
 */
bool FetchStage::F_stall(PipeReg * dreg, PipeReg * ereg, PipeReg * mreg){
	uint64_t E_icode = ereg->get(E_ICODE);
	uint64_t E_dstM = ereg->get(E_DSTM);
	uint64_t D_icode = dreg->get(D_ICODE);
	uint64_t M_icode = mreg->get(M_ICODE);
	return (E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == Stage::d_srcA || E_dstM == Stage::d_srcB) || (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode);
}
/*
 * D_stall
 *
 * Checks if the Decode stage needs to stall based on the conditions of the pipeline.
 *
 * @param: ereg - pointer to the Execute register instance
 * 
 * This function checks for dependencies that require stalling in the Decode stage. 
 * If certain instructions like IMRMOVQ or IPOPQ need data from the Decode stage, it will stall.
 * 
 * @return: bool - true if a stall is needed, false otherwise.
 */
bool FetchStage::D_stall(PipeReg * ereg){
	uint64_t E_icode = ereg->get(E_ICODE);
	uint64_t E_dstM = ereg->get(E_DSTM);
	return (E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == Stage::d_srcA || E_dstM == Stage::d_srcB);
}
/*
 * calculateControlSignals
 *
 * Calculates the control signals for stalling and bubbling based on the pipeline state.
 *
 * @param: dreg - pointer to the Decode register instance
 * @param: ereg - pointer to the Execute register instance
 * @param: mreg - pointer to the Memory register instance
 * 
 * This function updates the stall and bubble control signals for the pipeline, based on 
 * the current state of the Decode, Execute, and Memory stages.
 */
void FetchStage::calculateControlSignals(PipeReg * dreg, PipeReg * ereg, PipeReg * mreg){
	F_Stall = F_stall(dreg, ereg, mreg);
	D_Stall = D_stall(ereg);
	D_Bubble = D_bubble(dreg, ereg, mreg);
}
/*
 * D_bubble
 *
 * Checks if the Decode stage needs a bubble (a stall to prevent incorrect execution).
 *
 * @param: dreg - pointer to the Decode register instance
 * @param: ereg - pointer to the Execute register instance
 * @param: mreg - pointer to the Memory register instance
 * 
 * This function checks if the Decode stage needs a bubble to handle data hazards or 
 * control hazards based on the values in the pipeline registers.
 * 
 * @return: bool - true if a bubble is needed, false otherwise.
 */
bool FetchStage::D_bubble(PipeReg * dreg, PipeReg * ereg, PipeReg * mreg){
	uint64_t D_icode = dreg->get(D_ICODE);
	uint64_t E_icode = ereg->get(E_ICODE);
	uint64_t E_dstM = ereg->get(E_DSTM);
	uint64_t M_icode = mreg->get(M_ICODE);
	return (E_icode == Instruction::IJXX && !Stage::e_Cnd) || !((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == Stage::d_srcA || E_dstM == Stage::d_srcB)) && (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode);
}
